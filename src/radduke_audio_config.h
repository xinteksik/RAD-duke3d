/* GPL-3.0-or-later. Configuration schema retained from frozen RAD-Wolf. */
#ifndef RADDUKE_AUDIO_CONFIG_H
#define RADDUKE_AUDIO_CONFIG_H
#ifdef __cplusplus
extern "C" {
#endif
#define RADDUKE_SID_ULTISID 64u
typedef struct {
    unsigned stereo, address, model, width;
    unsigned primary_address, primary_model, music_width;
} radduke_audio_config;
int radduke_audio_config_parse(radduke_audio_config *config, const char *text);
/* 0: loaded; 1: missing/defaults; -1: invalid/defaults. */
int radduke_audio_config_load(radduke_audio_config *config, const char *path);
unsigned radduke_audio_startup(const char *path);
extern unsigned radduke_stereo_width;
void radduke_spatial_attenuation(int forward, int side, int *left, int *right, unsigned width);
#ifdef __cplusplus
}
#endif
#endif
