#include "radduke_music.h"
#include "music.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void radduke_log(const char *s){fputs(s,stderr);}
int main(int argc,char **argv)
{
    assert(argc>1);assert(MUSIC_Init(0,NULL)==MUSIC_Ok);
    radduke_music_info info;radduke_music_get_info(&info);assert(info.ready);
    for(int i=1;i<argc;++i){
        FILE *f=fopen(argv[i],"rb");assert(f);fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);
        void *data=malloc((size_t)n);assert(data&&fread(data,1,(size_t)n,f)==(size_t)n);fclose(f);
        radduke_music_configure_stereo(0);
        assert(MUSIC_PlaySong(data,(unsigned)n,1)==MUSIC_Ok);
        double left=0,right=0,difference=0;unsigned rendered=0;
        for(unsigned frame=0;frame<22050*3;frame+=256){
            int16_t pcm[512];radduke_music_render(pcm,256);
            for(unsigned j=0;j<256;++j){double l=pcm[2*j],r=pcm[2*j+1];left+=l*l;right+=r*r;difference+=(l-r)*(l-r);}
            rendered+=256;
        }
        radduke_music_get_info(&info);
        assert(info.stereo_width==0&&left>10000&&right>10000);
        /* Some authored openings are centered. GRABBAG/STALKER demonstrate
         * that zero extra width still preserves actual left/right differences. */
        if(strstr(argv[i],"GRABBAG")||strstr(argv[i],"STALKER"))assert(info.explicit_pan_mask&&difference>10000);
        printf("{\"file\":\"%s\",\"width\":0,\"explicit_pan_mask\":%u,\"lr_difference_rms\":%.6f}\n",
               argv[i],info.explicit_pan_mask,sqrt(difference/rendered));
        MUSIC_StopSong();free(data);
    }
    MUSIC_Shutdown();return 0;
}
