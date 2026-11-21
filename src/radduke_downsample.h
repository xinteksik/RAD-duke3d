/* Adapted for RAD DUKE3D from the frozen RAD-Wolf FIR; algorithm unchanged. */
/* GPL-3.0-or-later. Streaming stereo FIR decimator, exactly 44100 -> 22050 Hz. */
#ifndef RADDUKE_DOWNSAMPLE_H
#define RADDUKE_DOWNSAMPLE_H
typedef struct {
    float history[2][128];
    unsigned next, phase;
} radduke_downsampler;
void radduke_downsample_reset(radduke_downsampler *state);
/* Output capacity must be at least (input_frames + 1) / 2 stereo frames. */
unsigned radduke_downsample(radduke_downsampler *state, const float *input,
                           unsigned input_frames, float *output);
#endif
