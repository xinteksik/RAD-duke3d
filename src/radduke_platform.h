/* RAD DUKE3D private platform integration. Component licenses remain separate. */
#ifndef RADDUKE_PLATFORM_H
#define RADDUKE_PLATFORM_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
uint64_t radduke_time_us(void);
extern uint64_t radduke_memory_bytes;
void radduke_poll(void);
void radduke_wait_ms(unsigned milliseconds);
void radduke_present(const uint8_t *indices, int width, int height, int pitch,
                     const uint8_t *rgba_palette);
void radduke_log(const char *text);
void radduke_key_event(unsigned raw_key, int down);
int radduke_main(void);
#ifdef __cplusplus
}
#endif
#endif
