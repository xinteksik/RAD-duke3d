#include "radduke_audio_config.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
unsigned radwolf_sid1_address=0xd418, radwolf_sid1_model=8580;
unsigned radwolf_sid2_address, radwolf_sid2_model;
const unsigned char *radwolf_sid2_lut;
static const unsigned char table[256];
static unsigned primary_base, primary_type, second_base, second_type, width, calls, locks, unlocks;
static int primary_ok=1, second_ok=1, locked;
unsigned radwolf_sid1_boot_model(void) { return 8580; }
void disableInterrupts(void) { assert(!locked); locked=1; ++locks; }
void enableInterrupts(void) { assert(locked); locked=0; ++unlocks; }
int radwolf_sid1_initialize(unsigned base, unsigned type)
{ assert(locked); primary_base=base;primary_type=type;return primary_ok; }
int radwolf_sid2_initialize(unsigned base, unsigned type)
{
    assert(locked); ++calls; second_base=base;second_type=type;
    if (second_ok) { radwolf_sid2_address=base+0x18;radwolf_sid2_model=type;radwolf_sid2_lut=table; }
    return second_ok;
}
void radduke_music_configure_stereo(unsigned value) { width=value; }
void radduke_log(const char *text) { (void)text; }
static void put(const char *path, const void *text, size_t size)
{ FILE *f=fopen(path,"wb");assert(f);assert(fwrite(text,1,size,f)==size);fclose(f); }
int main(int argc, char **argv)
{
    assert(argc==2);
    const char *path=argv[1];
    remove(path);
    assert(!radduke_audio_startup(path));
    assert(primary_base==0xd400 && primary_type==0 && !calls && width==0);
    const char *valid="STEREO=ON\nSID1_ADDRESS=$D400\nSID1_TYPE=8580\nSID2_ADDRESS=$D500\nSID2_TYPE=6581\nSTEREO_WIDTH=20\nMIDI_STEREO_WIDTH=85\n";
    put(path,valid,strlen(valid));
    assert(radduke_audio_startup(path));
    assert(primary_type==8580 && second_type==6581 && second_base==0xd500 && width==85);
    assert(calls==1 && radduke_stereo_width==20);
    primary_ok=0;
    assert(!radduke_audio_startup(path) && calls==1 && width==0 && !radwolf_sid2_address);
    primary_ok=1;second_ok=0;
    assert(!radduke_audio_startup(path) && calls==2 && width==0 && !radwolf_sid2_lut);
    second_ok=1;
    const char *bad="STEREO=ON\nSID1_ADDRESS=$DE00\nSID2_ADDRESS=$DC00\n";
    put(path,bad,strlen(bad));
    assert(!radduke_audio_startup(path) && calls==2 && primary_base==0xd400 && primary_type==0);
    const char nul[]="STEREO=ON\0SID2_ADDRESS=$D500";
    put(path,nul,sizeof(nul)); assert(!radduke_audio_startup(path) && width==0 && calls==2);
    char huge[1024];memset(huge,'#',sizeof(huge));put(path,huge,sizeof(huge));
    assert(!radduke_audio_startup(path) && width==0 && calls==2);
    assert(!locked && locks==unlocks);
    remove(path);
    puts("SID config startup, missing/invalid/NUL/oversize fallback, primary/secondary failure, IRQ lock scope: OK");
    return 0;
}
