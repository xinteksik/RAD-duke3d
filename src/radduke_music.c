/* GPL-3.0-or-later. RAD DUKE3D MIDI/SF2 backend for the existing MUSIC ABI.
 * TinySoundFont (MIT), TinyMidiLoader (zlib) retain their complete headers.
 * FIR decimator is derived from GPL-3.0-or-later RAD-Wolf code.
 * No aggregate license or permission for combined public distribution is implied. */
#include "radduke_music.h"
#include "radduke_platform.h"
#include "radduke_downsample.h"
#include "music.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TSF_STATIC
#define TSF_IMPLEMENTATION
#define TSF_NO_STDIO
#include "../third_party/tsf.h"
#define TML_STATIC
#define TML_IMPLEMENTATION
#define TML_NO_STDIO
#include "../third_party/tml.h"

int MUSIC_ErrorCode;
static tsf *radduke_synth;
static tml_message *radduke_song, *radduke_next;
static radduke_downsampler radduke_filter;
static radduke_music_info radduke_info;
static uint64_t radduke_frame, radduke_loop_frames;
static int radduke_loop, radduke_volume = 192;
static float radduke_pending[256];
static unsigned radduke_pending_count, radduke_pending_pos;
static uint8_t radduke_programs[16];

/* Automatic placement follows frozen RAD-Wolf. Explicit MIDI CC10/42 wins. */
static void radduke_music_apply_pan(unsigned channel)
{
    if (!radduke_synth || (radduke_info.explicit_pan_mask & (1u << channel))) return;
    static const float positions[8] = {-0.35f, 0.35f, -0.20f, 0.20f, -0.45f, 0.45f, -0.30f, 0.30f};
    unsigned program = radduke_programs[channel];
    float offset = (channel == 9 || (program >= 32 && program <= 39) || program == 58)
        ? 0 : positions[channel & 7] * radduke_info.stereo_width / 100.0f;
    tsf_channel_set_pan(radduke_synth, (int)channel, 0.5f + offset);
}
void radduke_music_configure_stereo(unsigned width)
{
    radduke_info.stereo_width = width > 100 ? 100 : width;
    for (unsigned c = 0; c < 16; ++c) radduke_music_apply_pan(c);
}

static void radduke_music_reset_synth(void)
{
    tsf_reset(radduke_synth);
    /* Finish old voices' short release before resetting channel controllers. */
    float discarded[512 * 2];
    tsf_render_float(radduke_synth, discarded, 512, 0);
    radduke_info.explicit_pan_mask = 0;
    memset(radduke_programs, 0, sizeof(radduke_programs));
    for (int c = 0; c < 16; ++c) {
        tsf_channel_midi_control(radduke_synth, c, 121, 0);
        tsf_channel_set_presetnumber(radduke_synth, c, 0, c == 9);
        tsf_channel_set_pitchwheel(radduke_synth, c, 8192);
        tsf_channel_set_pan(radduke_synth, c, 0.5f);
        radduke_music_apply_pan((unsigned)c);
    }
    radduke_frame = 0;
    radduke_next = radduke_song;
    radduke_pending_count = radduke_pending_pos = 0;
    radduke_downsample_reset(&radduke_filter);
}

const char *MUSIC_ErrorString(int error)
{
    (void)error;
    return "RADDUKE MIDI/SF2 backend: missing font or invalid MIDI";
}
int MUSIC_Init(int device, const char *params)
{
    (void)device; (void)params;
    if (radduke_synth) return MUSIC_Ok;
    const char *path = "soundfont.sf2";
#ifndef AARCH
    const char *override = getenv("RADDUKE_SOUNDFONT");
    if (override && *override) path = override;
#endif
    FILE *f = fopen(path, "rb");
    long size = -1;
    if (f && !fseek(f, 0, SEEK_END)) size = ftell(f);
    if (f && size > 0 && size <= 64 * 1024 * 1024 && !fseek(f, 0, SEEK_SET)) {
        void *data = malloc((size_t)size);
        if (data && fread(data, 1, (size_t)size, f) == (size_t)size)
            radduke_synth = tsf_load_memory(data, (int)size);
        free(data);
    }
    if (f) fclose(f);
    if (radduke_synth && !tsf_set_max_voices(radduke_synth, 64)) {
        tsf_close(radduke_synth); radduke_synth = NULL;
    }
    radduke_info.ready = radduke_synth != NULL;
    if (radduke_synth) {
        tsf_set_output(radduke_synth, TSF_STEREO_INTERLEAVED, 44100, -9.0f);
        radduke_music_reset_synth();
        radduke_log("music: SF2 ready; 64 voices; 44100 -> FIR -> 22050 Hz\n");
    } else radduke_log("music: soundfont.sf2 unavailable; continuing silently\n");
    /* Missing private assets must never prevent the game from booting. */
    return MUSIC_Ok;
}
int MUSIC_StopSong(void)
{
    radduke_info.playing = radduke_info.paused = 0;
    if (radduke_song) tml_free(radduke_song);
    radduke_song = radduke_next = NULL;
    radduke_pending_count = 0;
    return MUSIC_Ok;
}
int MUSIC_Shutdown(void)
{
    MUSIC_StopSong();
    if (radduke_synth) tsf_close(radduke_synth);
    radduke_synth = NULL;
    radduke_info.ready = 0;
    return MUSIC_Ok;
}
int MUSIC_PlaySong(void *data, unsigned length, int loop)
{
    MUSIC_StopSong();
    if (!radduke_synth) return MUSIC_Ok;
    if (!data || length < 14 || length > 1024 * 1024 || memcmp(data, "MThd", 4)) {
        MUSIC_ErrorCode = MUSIC_MidiError; return MUSIC_Warning;
    }
    radduke_song = tml_load_memory(data, (int)length);
    int channels, programs, notes;
    unsigned duration = 0;
    if (!radduke_song || !tml_get_info(radduke_song, &channels, &programs, &notes, NULL, &duration) || !duration) {
        MUSIC_StopSong();
        MUSIC_ErrorCode = MUSIC_MidiError; return MUSIC_Warning;
    }
    radduke_info.messages = 0;
    for (tml_message *m = radduke_song; m; m = m->next) ++radduke_info.messages;
    radduke_info.channels = (unsigned)channels;
    radduke_info.programs = (unsigned)programs;
    radduke_info.notes = (unsigned)notes;
    radduke_info.length_ms = duration;
    radduke_info.output_frames = radduke_info.loops = 0;
    /* Even high-rate boundary preserves the exact 2:1 decimator phase. */
    radduke_loop_frames = (((uint64_t)duration * 44100 / 1000) + 2) & ~(uint64_t)1;
    radduke_loop = !!loop;
    radduke_music_reset_synth();
    radduke_info.playing = 1;
    char log[192];
    snprintf(log, sizeof(log), "music: play %s; duration=%u ms notes=%u messages=%u loop=%d\n",
             radduke_info.name, duration, radduke_info.notes, radduke_info.messages, radduke_loop);
    radduke_log(log);
    MUSIC_ErrorCode = MUSIC_Ok;
    return MUSIC_Ok;
}
void MUSIC_SetVolume(int volume) { radduke_volume = volume < 0 ? 0 : volume > 255 ? 255 : volume; }
int MUSIC_GetVolume(void) { return radduke_volume; }
void MUSIC_Pause(void) { radduke_info.paused = 1; }
void MUSIC_Continue(void) { radduke_info.paused = 0; }
int MUSIC_SongPlaying(void) { return radduke_info.playing; }
void MUSIC_SetLoopFlag(int loop) { radduke_loop = !!loop; }
void MUSIC_RegisterTimbreBank(unsigned char *bank) { (void)bank; /* SF2 replaces the FM bank. */ }
void radduke_music_name(const char *name)
{ snprintf(radduke_info.name, sizeof(radduke_info.name), "%s", name ? name : ""); }
void radduke_music_get_info(radduke_music_info *info) { if (info) *info = radduke_info; }

static void radduke_dispatch(const tml_message *m)
{
    switch (m->type) {
    case TML_PROGRAM_CHANGE:
        radduke_programs[m->channel] = (uint8_t)m->program;
        tsf_channel_set_presetnumber(radduke_synth, m->channel, m->program, m->channel == 9);
        radduke_music_apply_pan(m->channel); break;
    case TML_NOTE_ON: tsf_channel_note_on(radduke_synth, m->channel, m->key, m->velocity / 127.0f); break;
    case TML_NOTE_OFF: tsf_channel_note_off(radduke_synth, m->channel, m->key); break;
    case TML_PITCH_BEND: tsf_channel_set_pitchwheel(radduke_synth, m->channel, m->pitch_bend); break;
    case TML_CONTROL_CHANGE:
        tsf_channel_midi_control(radduke_synth, m->channel, m->control, m->control_value);
        if (m->control == TML_PAN_MSB || m->control == TML_PAN_LSB)
            radduke_info.explicit_pan_mask |= 1u << m->channel;
        else if (m->control == TML_ALL_CTRL_OFF) {
            radduke_info.explicit_pan_mask &= ~(1u << m->channel);
            radduke_music_apply_pan(m->channel);
        }
        break;
    default: break;
    }
}
void radduke_music_render(int16_t *stereo, unsigned frames)
{
    memset(stereo, 0, frames * 2u * sizeof(*stereo));
    if (!radduke_info.playing || radduke_info.paused || !radduke_synth) return;
    while (frames) {
        while (!radduke_pending_count) {
            if (radduke_frame >= radduke_loop_frames) {
                if (!radduke_loop) { radduke_info.playing = 0; return; }
                ++radduke_info.loops;
                radduke_music_reset_synth();
            }
            while (radduke_next && (uint64_t)radduke_next->time * 44100 / 1000 <= radduke_frame) {
                radduke_dispatch(radduke_next);
                radduke_next = radduke_next->next;
            }
            uint64_t boundary = radduke_next ? (uint64_t)radduke_next->time * 44100 / 1000 : radduke_loop_frames;
            unsigned n = boundary - radduke_frame > 256 ? 256 : (unsigned)(boundary - radduke_frame);
            float high[256 * 2];
            tsf_render_float(radduke_synth, high, (int)n, 0);
            radduke_pending_count = radduke_downsample(&radduke_filter, high, n, radduke_pending);
            radduke_pending_pos = 0;
            radduke_frame += n;
        }
        unsigned n = frames < radduke_pending_count ? frames : radduke_pending_count;
        for (unsigned i = 0; i < n * 2; ++i) {
            float sample = radduke_pending[radduke_pending_pos * 2 + i] * (32767.0f * radduke_volume / 255.0f);
            stereo[i] = (int16_t)(sample > 32767 ? 32767 : sample < -32768 ? -32768 : sample);
        }
        stereo += n * 2;
        frames -= n;
        radduke_info.output_frames += n;
        radduke_pending_count -= n;
        radduke_pending_pos += n;
    }
}
