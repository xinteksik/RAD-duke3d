/* GPL-3.0-or-later. Cooperative ring producer derived from the frozen RAD-Wolf mixer. */
#include "radduke_music.h"
#include "radduke_platform.h"
#ifdef RADDUKE_EFFECTS
#include "radduke_pcm.h"
#endif
#include "radwolf_audio_config.h"
#ifdef RADDUKE_STEREO
#include "radduke_audio_config.h"
#endif
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
extern uint64_t soundFirstuSecTick, GetuSec(void);
extern unsigned char soundRingBuffer[16384];
extern const unsigned char *mahoneyLUT;
extern void startSIDSamplePlayer(void), stopSIDSamplePlayer(void);
void radduke_music_service(void)
{
    static bool started, pumping, timer_started;
    static uint64_t written, underruns;
    if (pumping) return;
    radduke_music_info info;
    radduke_music_get_info(&info);
#ifdef RADDUKE_EFFECTS
    if (!info.ready && !radduke_pcm_ready()) return;
#else
    if (!info.ready) return;
#endif
    pumping = true;
    if (!started) {
        stopSIDSamplePlayer();
#ifdef RADDUKE_STEREO
        radduke_audio_startup("AUDIO.CFG");
#endif
        memset(soundRingBuffer, mahoneyLUT[128], 16384);
        memset(radwolf_right_ring, radwolf_sid2_lut ? radwolf_sid2_lut[128] : 0, 16384);
        soundFirstuSecTick = GetuSec();
        written = 0;
        started = true;
        radduke_log("music: cooperative 22050 Hz SID ring, 100 ms lookahead\n");
    }
    uint64_t now = timer_started ? (GetuSec() - soundFirstuSecTick) * 22050 / 1000000 : 0;
    if (written < now) {
        written = now;
        ++underruns;
        if (!(underruns & (underruns - 1))) {
            char report[96];
            snprintf(report, sizeof(report), "music: buffer underruns=%llu\n", (unsigned long long)underruns);
            radduke_log(report);
        }
    }
    uint64_t target = now + 2205;
    while (written < target) {
        int16_t pcm[256 * 2];
        unsigned n = target - written > 256 ? 256 : (unsigned)(target - written);
#ifdef RADDUKE_EFFECTS
        radduke_audio_render(pcm, n);
#else
        radduke_music_render(pcm, n);
#endif
        for (unsigned i = 0; i < n; ++i) {
            unsigned slot = written++ & 16383;
            int left = pcm[2*i], right = pcm[2*i+1];
            int mono = radwolf_sid2_address ? left : (left + right) / 2;
            if (radwolf_sid2_address && radwolf_sid2_lut)
                radwolf_right_ring[slot] = radwolf_sid2_lut[(right + 32768) >> 8];
            soundRingBuffer[slot] = mahoneyLUT[(mono + 32768) >> 8];
        }
    }
    if (!timer_started) {
        soundFirstuSecTick = GetuSec(); /* Epoch starts after the initial prefill. */
        startSIDSamplePlayer(); timer_started = true;
    }
    pumping = false;
}
