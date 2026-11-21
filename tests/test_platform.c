#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "radduke_platform.h"
char keystatus[256];
int keyfifo[64], keyfifoplc, keyfifoend;
unsigned char keyasciififo[64];
int keyasciififoplc, keyasciififoend;
int totalclock;
static uint64_t radduke_fake_time;
uint64_t radduke_time_us(void) { return radduke_fake_time; }
void radduke_poll(void) {}
extern int inittimer(int, void (*)(void));
extern void sampletimer(void);
int main(void)
{
    const unsigned raw[] = {'I','J','K','L',13,4,'F','A','Z',1,6,32,'1','2','3','4','5','6','7','8','9','0'};
    const unsigned code[] = {0xc8,0xcb,0xd0,0xcd,0x1c,0x1d,0x1d,0x1e,0x2c,0x2a,0x36,0x39,2,3,4,5,6,7,8,9,10,11};
    for (unsigned i = 0; i < sizeof(raw)/sizeof(raw[0]); ++i) {
        radduke_key_event(raw[i], 1); assert(keystatus[code[i]]);
        radduke_key_event(raw[i], 0); assert(!keystatus[code[i]]);
    }
    assert(keyasciififo[0] == 13);
    radduke_key_event(1,1);radduke_key_event('1',1);
    assert(keystatus[0x2a]&&keystatus[2]);
    radduke_key_event('1',0);assert(keystatus[0x2a]&&!keystatus[2]);
    radduke_key_event(1,0);assert(!keystatus[0x2a]);
    radduke_key_event(4, 1); radduke_key_event('F', 1);
    radduke_key_event(4, 0); assert(keystatus[0x1d]);
    radduke_key_event('F', 0); assert(!keystatus[0x1d]);
    radduke_key_event('F', 1); radduke_key_event('F', 1);
    radduke_key_event('F', 0); assert(!keystatus[0x1d]);
    radduke_key_event('i', 1);
    for (unsigned i = 0; i < 100; ++i) radduke_key_event(i & 1 ? 'J' : 'K', (i / 2) & 1);
    radduke_key_event('i', 0); assert(!keystatus[0xc8]);
    radduke_fake_time = UINT64_C(4294967000);
    assert(inittimer(120, NULL) == 0);
    for (unsigned i = 1; i <= 1000; ++i) { radduke_fake_time += 1000; sampletimer(); }
    assert(totalclock == 120); /* crosses old 32-bit microsecond boundary */
    radduke_fake_time += 60000000;
    sampletimer(); assert(totalclock == 7320);
    puts("IJKL/Enter, actions, weapon digits 1-0, Shift independence, alias/FIFO release, 120Hz wrap: OK");
    return 0;
}
