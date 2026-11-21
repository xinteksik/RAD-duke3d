/* Exercise the actual SF2 renderer using private MIDI copies, not injected PCM. */
#include "radduke_music.h"
#include "music.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void radduke_log(const char *text) { fputs(text, stderr); }
static void *read_file(const char *path, unsigned *length)
{
    FILE *f = fopen(path, "rb"); assert(f);
    assert(!fseek(f, 0, SEEK_END)); long size = ftell(f); assert(size > 0 && size < 1048576);
    rewind(f); void *data = malloc((size_t)size); assert(data);
    assert(fread(data, 1, (size_t)size, f) == (size_t)size); fclose(f);
    *length = (unsigned)size; return data;
}
static void silent(const int16_t *p, unsigned frames)
{ for (unsigned i = 0; i < 2*frames; ++i) assert(!p[i]); }
int main(int argc, char **argv)
{
    assert(argc > 1);
    assert(MUSIC_Init(0, NULL) == MUSIC_Ok);
    radduke_music_info info; radduke_music_get_info(&info); assert(info.ready);
    assert(MUSIC_PlaySong("bad", 3, 1) == MUSIC_Warning);
    const unsigned frames = 3 * RADDUKE_MUSIC_RATE;
    int16_t *a = malloc(frames * 4), *b = malloc(frames * 4); assert(a && b);
    for (int track = 1; track < argc; ++track) {
        unsigned length; void *data = read_file(argv[track], &length);
        const char *name = strrchr(argv[track], '/'); name = name ? name+1 : argv[track];
        radduke_music_name(name);
        MUSIC_SetVolume(192);
        assert(MUSIC_PlaySong(data, length, 1) == MUSIC_Ok);
        radduke_music_render(a, frames);
        int peak = 0; double energy = 0;
        for (unsigned i = 0; i < frames*2; ++i) {
            int v = abs(a[i]); if (v > peak) peak = v; energy += (double)a[i]*a[i];
        }
        assert(peak > 10 && energy > frames);
        assert(MUSIC_PlaySong(data, length, 1) == MUSIC_Ok);
        for (unsigned offset = 0; offset < frames;) {
            unsigned n = 1 + offset % 509; if (n > frames - offset) n = frames - offset;
            radduke_music_render(b + offset*2, n); offset += n;
        }
        assert(!memcmp(a, b, frames*4)); /* chunk-independent MIDI/FIR scheduling */
        radduke_music_get_info(&info); uint64_t before = info.output_frames;
        MUSIC_Pause(); radduke_music_render(b, 1000); silent(b, 1000);
        radduke_music_get_info(&info); assert(info.output_frames == before);
        MUSIC_Continue(); radduke_music_render(b, 1000);
        radduke_music_get_info(&info); assert(info.output_frames == before + 1000);
        MUSIC_SetVolume(0); radduke_music_render(b, 1000); silent(b, 1000);
        MUSIC_StopSong(); radduke_music_render(b, 1000); silent(b, 1000);
        assert(!MUSIC_SongPlaying());
        MUSIC_SetVolume(192);
        assert(MUSIC_PlaySong(data, length, 1) == MUSIC_Ok);
        radduke_music_get_info(&info);
        unsigned duration = info.length_ms;
        uint64_t total = (uint64_t)duration * RADDUKE_MUSIC_RATE / 1000 + 512;
        for (uint64_t n = 0; n < total;) {
            unsigned part = total-n > 1024 ? 1024 : (unsigned)(total-n);
            radduke_music_render(b, part); n += part;
        }
        radduke_music_get_info(&info); assert(info.loops >= 1);
        assert(MUSIC_PlaySong(data, length, 0) == MUSIC_Ok);
        for (uint64_t n = 0; n < total;) {
            unsigned part = total-n > 1024 ? 1024 : (unsigned)(total-n);
            radduke_music_render(b, part); n += part;
        }
        assert(!MUSIC_SongPlaying());
        printf("{\"name\":\"%s\",\"duration_ms\":%u,\"notes\":%u,\"messages\":%u,\"channels\":%u,\"programs\":%u,\"preview_peak\":%d,\"preview_rms\":%.3f,\"chunk_identity\":true,\"pause_resume\":true,\"mute_stop\":true,\"loop_and_play_once\":true}\n",
               name, duration, info.notes, info.messages, info.channels, info.programs, peak, sqrt(energy/(frames*2)));
        fflush(stdout); free(data);
    }
    MUSIC_Shutdown(); free(a); free(b);
    return 0;
}
