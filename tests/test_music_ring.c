#include "radduke_music.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
unsigned char soundRingBuffer[16384], radwolf_right_ring[16384];
static unsigned char table[256];
const unsigned char *mahoneyLUT = table, *radwolf_sid2_lut = table;
unsigned radwolf_sid2_address;
uint64_t soundFirstuSecTick;
static uint64_t clock_us, rendered;
static unsigned starts, stops, underruns;
#ifdef RADDUKE_STEREO
static unsigned configured;
unsigned radduke_audio_startup(const char *path)
{ assert(stops==1 && !starts && !strcmp(path,"AUDIO.CFG"));++configured;return 0; }
#endif
uint64_t GetuSec(void) { return clock_us; }
void startSIDSamplePlayer(void) { ++starts; }
void stopSIDSamplePlayer(void) { ++stops; }
void radduke_log(const char *s) { if (strstr(s, "underruns=")) ++underruns; }
void radduke_music_get_info(radduke_music_info *info) { memset(info, 0, sizeof(*info)); info->ready = 1; }
void radduke_music_render(int16_t *p, unsigned frames)
{
    for (unsigned i = 0; i < frames; ++i) { *p++ = 10000; *p++ = 6000; }
    rendered += frames;
    clock_us += 2000; /* Initial synthesis cost must not consume the prefill. */
}
int main(void)
{
    for (unsigned i = 0; i < 256; ++i) table[i] = (unsigned char)i;
    radduke_music_service();
    assert(starts == 1 && stops == 1 && rendered == 2205 && underruns == 0);
    assert(soundFirstuSecTick == clock_us);
#ifdef RADDUKE_STEREO
    assert(configured==1);
#endif
    for (unsigned i = 0; i < 2205; ++i) assert(soundRingBuffer[i] == 159);
    for (unsigned i = 2205; i < 16384; ++i) assert(soundRingBuffer[i] == 128);
    uint64_t before = rendered;
    radduke_music_service(); assert(rendered == before); /* Do not overwrite live/future slots. */
    clock_us += 10000;
    radduke_music_service(); assert(rendered == before + 220);
    clock_us += 300000;
    radduke_music_service(); assert(underruns == 1);
    assert(starts == 1 && stops == 1);
    radwolf_sid2_address = 0xd430;
    clock_us += 800000; /* Ring wrap and a long-load recovery. */
    uint64_t now = (clock_us - soundFirstuSecTick) * 22050 / 1000000;
    radduke_music_service();
    assert(soundRingBuffer[now & 16383] == 167);
    assert(radwolf_right_ring[now & 16383] == 151);
    puts("SID ring prefill epoch, mono/stereo LUT, no early overwrite, underrun and wrap: OK");
    return 0;
}
