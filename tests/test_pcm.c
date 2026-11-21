#include "radduke_pcm.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned calls;
void radduke_log(const char *s){(void)s;}
void radduke_music_render(int16_t *p,unsigned frames){memset(p,0,frames*4);}
static void callback(void){++calls;}
int main(void)
{
    int rate=44100,channels=1,bits=8;
    int16_t buffer[]={1000,-1000,2000,-2000,3000,-3000,4000,-4000};
    int16_t a[16]={0},b[16]={0};radduke_pcm_info info;
    assert(radduke_pcm_init(&rate,&channels,&bits,NULL)==0);
    assert(rate==22050&&channels==2&&bits==16);
    assert(radduke_pcm_begin((char*)buffer,7,2,callback)!=0);
    assert(radduke_pcm_begin((char*)buffer,8,2,callback)==0);
    radduke_audio_render(a,8);assert(calls==4);
    radduke_pcm_get_info(&info);assert(info.frames==8&&info.nonzero_samples==16);
    assert(a[0]==1000&&a[4]==3000&&a[8]==1000&&a[12]==3000);
    assert(radduke_pcm_init(&rate,&channels,&bits,NULL)==0);
    assert(radduke_pcm_begin((char*)buffer,8,2,callback)==0);
    for(unsigned i=0;i<8;++i)radduke_audio_render(b+i*2,1);
    assert(!memcmp(a,b,sizeof(a)));
    radduke_pcm_lock();radduke_pcm_lock();
    memset(b,0,sizeof(b));radduke_pcm_mix(b,8);
    assert(!memcmp(b,(int16_t[16]){0},sizeof(b)));
    radduke_pcm_unlock();radduke_pcm_unlock();
    for(unsigned i=0;i<16;++i)b[i]=(i&1)?-32700:32700;
    radduke_pcm_mix(b,8);
    assert(b[0]==32767&&b[1]==-32768);
    radduke_pcm_get_info(&info);assert(info.clipped_samples==16&&!info.lock_depth);
    radduke_pcm_stop();memset(b,0,sizeof(b));radduke_pcm_mix(b,8);assert(!memcmp(b,(int16_t[16]){0},sizeof(b)));
    radduke_pcm_shutdown();assert(!radduke_pcm_ready());
    puts("PCM format negotiation, division callbacks, chunk identity, lock/stop and saturation: OK");
}
