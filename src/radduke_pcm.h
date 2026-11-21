/* GPL-3.0-or-later. Single-threaded PCM bridge for pinned JFAudioLib MultiVoc. */
#ifndef RADDUKE_PCM_H
#define RADDUKE_PCM_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    uint64_t frames, callbacks, nonzero_samples, clipped_samples;
    unsigned ready, playing, lock_depth;
} radduke_pcm_info;
int radduke_pcm_init(int *rate,int *channels,int *bits,void *data);
void radduke_pcm_shutdown(void);
int radduke_pcm_begin(char *buffer,int bytes,int divisions,void (*callback)(void));
void radduke_pcm_stop(void);
void radduke_pcm_lock(void);
void radduke_pcm_unlock(void);
int radduke_pcm_ready(void);
void radduke_pcm_get_info(radduke_pcm_info *info);
void radduke_audio_render(int16_t *stereo,unsigned frames);
/* Add effects to an existing stereo buffer, retaining signed-16 saturation. */
void radduke_pcm_mix(int16_t *stereo,unsigned frames);
#ifdef __cplusplus
}
#endif
#endif
