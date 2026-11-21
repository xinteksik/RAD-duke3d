/* RAD DUKE3D music adapter; component licenses remain separate. */
#ifndef RADDUKE_MUSIC_H
#define RADDUKE_MUSIC_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RADDUKE_MUSIC_RATE 22050u
typedef struct {
    uint64_t output_frames, loops;
    unsigned length_ms, messages, notes, programs, channels;
    int ready, playing, paused;
    unsigned stereo_width, explicit_pan_mask;
    char name[64];
} radduke_music_info;
void radduke_music_render(int16_t *stereo, unsigned frames);
void radduke_music_name(const char *name);
void radduke_music_get_info(radduke_music_info *info);
void radduke_music_configure_stereo(unsigned width);
void radduke_music_service(void); /* main-thread platform sink, never IRQ */
#ifdef __cplusplus
}
#endif
#endif
