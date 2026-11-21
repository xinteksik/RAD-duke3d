#include "radduke_music.h"
#include "music.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void radduke_log(const char *text) { fputs(text,stderr); }
/* Self-authored SMF fixture, no game assets. 96 PPQN, one note, one second. */
static unsigned make_song(unsigned char *out, unsigned channel, unsigned program, int pan, int reset)
{
    unsigned char track[64]; unsigned n=0;
    if(pan>=0) { track[n++]=0;track[n++]=0xb0|channel;track[n++]=10;track[n++]=(unsigned char)pan; }
    if(reset) { track[n++]=0;track[n++]=0xb0|channel;track[n++]=121;track[n++]=0; }
    track[n++]=0;track[n++]=0xc0|channel;track[n++]=program;
    track[n++]=0;track[n++]=0x90|channel;track[n++]=channel==9?36:60;track[n++]=100;
    track[n++]=96;track[n++]=0x80|channel;track[n++]=channel==9?36:60;track[n++]=0;
    track[n++]=96;track[n++]=0xff;track[n++]=0x2f;track[n++]=0;
    const unsigned char header[]={ 'M','T','h','d',0,0,0,6,0,0,0,1,0,96,'M','T','r','k',0,0,0,0 };
    memcpy(out,header,sizeof(header));out[21]=(unsigned char)n;memcpy(out+22,track,n);return n+22;
}
static void render(const unsigned char *song,unsigned size,unsigned width,int16_t *pcm)
{
    radduke_music_configure_stereo(width);
    assert(MUSIC_PlaySong((void*)song,size,0)==MUSIC_Ok);
    radduke_music_render(pcm,4096);
}
static double energy(const int16_t *p,unsigned channel)
{ double value=0;for(unsigned i=0;i<4096;++i)value+=(double)p[2*i+channel]*p[2*i+channel];return value; }
int main(void)
{
    assert(MUSIC_Init(0,NULL)==MUSIC_Ok);
    radduke_music_info info;radduke_music_get_info(&info);assert(info.ready);
    unsigned char song[128];int16_t a[4096*2],b[4096*2];unsigned n;
    n=make_song(song,0,0,-1,0);render(song,n,0,a);render(song,n,100,b);
    assert(energy(b,0)>2*energy(b,1) && memcmp(a,b,sizeof(a)));
    n=make_song(song,1,0,-1,0);render(song,n,100,b);assert(energy(b,1)>2*energy(b,0));
    n=make_song(song,0,0,127,0);render(song,n,0,a);render(song,n,100,b);assert(!memcmp(a,b,sizeof(a)));
    radduke_music_get_info(&info);assert(info.explicit_pan_mask==1);
    n=make_song(song,0,0,127,1);render(song,n,100,b);assert(energy(b,0)>2*energy(b,1));
    radduke_music_get_info(&info);assert(!info.explicit_pan_mask);
    n=make_song(song,1,32,-1,0);render(song,n,0,a);render(song,n,100,b);assert(!memcmp(a,b,sizeof(a)));
    n=make_song(song,9,0,-1,0);render(song,n,0,a);render(song,n,100,b);assert(!memcmp(a,b,sizeof(a)));
    radduke_music_configure_stereo(999);radduke_music_get_info(&info);assert(info.stereo_width==100);
    MUSIC_Shutdown();
    puts("MIDI auto stereo, explicit pan precedence/reset, centered bass/percussion, width clamp: OK");
    return 0;
}
