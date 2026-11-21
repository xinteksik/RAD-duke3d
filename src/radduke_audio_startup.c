/* GPL-3.0-or-later. SID configuration sequence adapted from frozen RAD-Wolf. */
#include "radduke_audio_config.h"
#include "radduke_music.h"
#include "radduke_platform.h"
#include "radwolf_audio_config.h"
#include <stdio.h>
extern void disableInterrupts(void), enableInterrupts(void);

unsigned radduke_audio_startup(const char *path)
{
    radduke_audio_config config;
    int status = radduke_audio_config_load(&config, path);
    if (status == 1) radduke_log("audio: AUDIO.CFG missing; mono defaults\n");
    if (status < 0) radduke_log("audio: invalid AUDIO.CFG; mono defaults\n");
    /* No secondary output until the complete opt-in configuration succeeds. */
    radwolf_sid2_address = radwolf_sid2_model = 0;
    radwolf_sid2_lut = NULL;
    unsigned boot_model = radwolf_sid1_boot_model();
    disableInterrupts();
    int primary_ok = radwolf_sid1_initialize(config.primary_address, config.primary_model);
    enableInterrupts();
    char report[192];
    snprintf(report, sizeof(report), "audio: SID1 base=$%04X boot=%u requested=%u selected=%u ok=%d\n",
             config.primary_address, boot_model, config.primary_model, radwolf_sid1_model, primary_ok);
    radduke_log(report);
    if (config.stereo && primary_ok) {
        radduke_stereo_width = config.width;
        disableInterrupts();
        int enabled = radwolf_sid2_initialize(config.address, config.model);
        enableInterrupts();
        radduke_log(enabled ? "stereo: SID2 enabled\n" : "stereo: detection failed; mono fallback\n");
        snprintf(report, sizeof(report), "audio: SID2 base=$%04X requested=%u selected=%u width=%u\n",
                 config.address, config.model, radwolf_sid2_model, config.width);
        radduke_log(report);
    } else if (config.stereo) radduke_log("stereo: SID1 initialization failed; retained primary mono output\n");
    unsigned width = radwolf_sid2_address && radwolf_sid2_lut ? config.music_width : 0;
    radduke_music_configure_stereo(width);
    snprintf(report, sizeof(report), "audio: output=%s MIDI_STEREO_WIDTH=%u\n", width || radwolf_sid2_address ? "stereo" : "mono", width);
    radduke_log(report);
    return radwolf_sid2_address != 0;
}
