#include "radduke_platform.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#ifdef RADDUKE_MUSIC
#include "radduke_music.h"
#endif

extern void readKeyDoom(unsigned int *, unsigned char *);
extern "C" void disableInterrupts(void);
extern "C" void enableInterrupts(void);
extern "C" uint32_t *radduke_rgb_frame;
extern "C" int (*radduke_video_functions[])(void);
/* Existing ring ABI retained by the hardware transport. */
extern "C" { unsigned char radwolf_right_ring[16384]; }
uint64_t radduke_memory_bytes;
extern "C" unsigned char getCurrentSample(void) { return 0; }
extern "C" void handleMouseUpdate(uint8_t *data) { (void)data; }

static FILE *radduke_boot_log;
extern "C" uint64_t radduke_time_us(void)
{
    uint64_t counter, frequency;
    asm volatile("isb; mrs %0, CNTPCT_EL0; mrs %1, CNTFRQ_EL0" : "=r"(counter), "=r"(frequency));
    return counter / frequency * UINT64_C(1000000) + counter % frequency * UINT64_C(1000000) / frequency;
}
extern "C" void radduke_capture_keys(const unsigned int *events, unsigned char count)
{
    for (unsigned i = 0; i < count; ++i)
        radduke_key_event(events[i] & 255u, !!(events[i] & 256u));
}
extern "C" void radduke_poll(void)
{
    static uint64_t last;
    uint64_t now = radduke_time_us();
    if (now - last < 1000) return;
    last = now;
    unsigned events[64]; unsigned char count = 0;
    disableInterrupts();
    readKeyDoom(events, &count); /* also delivers events from inside blits */
    enableInterrupts();
#ifdef RADDUKE_MUSIC
    radduke_music_service();
#endif
}
extern "C" void radduke_log(const char *text)
{
    if (radduke_boot_log) { fputs(text, radduke_boot_log); fflush(radduke_boot_log); }
}
extern "C" void radduke_present(const uint8_t *indices, int width, int height,
                                 int pitch, const uint8_t *palette)
{
    static bool first = true;
    if (width != 320 || height != 200 || pitch < 320) return;
    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
        const uint8_t *c = palette + 4 * indices[y * pitch + x];
        radduke_rgb_frame[y * 320 + x] = (uint32_t)c[0] << 16 | (uint32_t)c[1] << 8 | c[2];
    }
    if (first) radduke_log("first indexed frame -> RAD PAL transfer\n");
#ifdef RADDUKE_MUSIC
    radduke_music_service();
#endif
    (*radduke_video_functions[1])();
#ifdef RADDUKE_MUSIC
    radduke_music_service();
#endif
    if (first) radduke_log("first RAD PAL transfer returned\n");
    first = false;
}
extern "C" void radduke_run(void)
{
    radduke_boot_log = fopen("SD:RADDUKE/boot.log", "w");
#ifdef RADDUKE_EFFECTS
    radduke_log("RAD DUKE3D audio candidate: MIDI/SF2 + PCM effects\n");
#elif defined(RADDUKE_MUSIC)
    radduke_log("RAD DUKE3D music candidate: MIDI/SF2; effects disabled\n");
#else
    radduke_log("RAD DUKE3D menu candidate: silent; IJKL and Enter\n");
#endif
    if (chdir("SD:RADDUKE") != 0) { radduke_log("ERROR: chdir SD:RADDUKE failed\n"); return; }
    radduke_log("data directory mounted\n");
    radduke_main();
    radduke_log("game returned\n");
}
