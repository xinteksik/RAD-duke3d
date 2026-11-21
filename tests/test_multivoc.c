/* Use the pinned VOC decoder/mixer; fake neither PCM nor completion callbacks. */
#include "fx_man.h"
#include "radduke_pcm.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned finished;
void radduke_log(const char *s){fputs(s,stderr);}
void radduke_wait_ms(unsigned ms){(void)ms;}
void radduke_music_render(int16_t *p,unsigned frames){memset(p,0,frames*4);}
static void complete(unsigned value){assert(value==123);++finished;}
int main(int argc,char **argv)
{
    assert(argc>1);
    int channels=2,bits=16,rate=22050;
    assert(FX_Init(0,16,&channels,&bits,&rate,NULL)==FX_Ok);
    assert(FX_SetCallBack(complete)==FX_Ok);FX_SetVolume(255);
    for(int arg=1;arg<argc;++arg){
        FILE *f=fopen(argv[arg],"rb");assert(f);fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
        char *data=malloc((size_t)size);assert(data&&fread(data,1,(size_t)size,f)==(size_t)size);fclose(f);
        unsigned before=finished;
        int handle=FX_PlayAuto(data,(unsigned)size,0,255,255,255,1,123);assert(handle>0);
        int peak=0;uint64_t samples=0;
        for(unsigned frame=0;frame<22050*15;++frame){
            int16_t pcm[2];radduke_audio_render(pcm,1);
            for(unsigned c=0;c<2;++c){int v=abs(pcm[c]);if(v>peak)peak=v;if(v)++samples;}
            if(frame>2048&&!FX_SoundActive(handle))break;
        }
        assert(!FX_SoundActive(handle)&&finished==before+1&&peak>100&&samples>100);
        printf("VOC %s: peak=%d nonzero=%llu completion=OK\n",argv[arg],peak,(unsigned long long)samples);
        /* Natural completion must not leave a repeating/stuck sound. */
        for(unsigned n=0;n<8192;++n){int16_t pcm[2];radduke_audio_render(pcm,1);}
        int16_t tail[1024*2];radduke_audio_render(tail,1024);
        for(unsigned n=0;n<2048;++n)assert(tail[n]==0);
        free(data);
    }
    assert(FX_Shutdown()==FX_Ok);
    puts("Pinned VOC decoding, mixing, voice completion, callback and silent drain: OK");
}
