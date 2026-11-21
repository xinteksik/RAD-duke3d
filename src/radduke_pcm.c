/* GPL-3.0-or-later. Cooperative PCM sink for JFAudioLib's existing mixer.
 * Buffer/callback sequencing follows its pinned SDL driver. No audio thread,
 * mixer IRQ, direct SID register access or hardware timing is introduced. */
#include "radduke_pcm.h"
#include "radduke_music.h"
#include "radduke_platform.h"
#include <stddef.h>
#include <string.h>
static radduke_pcm_info radduke_pcm_state;
static char *radduke_pcm_buffer;
static unsigned radduke_pcm_bytes,radduke_pcm_divisions,radduke_pcm_current,radduke_pcm_used;
static void (*radduke_pcm_callback)(void);
void radduke_pcm_stop(void)
{
    radduke_pcm_state.playing=0;
    radduke_pcm_callback=NULL;radduke_pcm_buffer=NULL;
}
int radduke_pcm_init(int *rate,int *channels,int *bits,void *data)
{
    (void)data;
    if(!rate||!channels||!bits)return -1;
    radduke_pcm_stop();memset(&radduke_pcm_state,0,sizeof(radduke_pcm_state));
    *rate=22050;*channels=2;*bits=16;radduke_pcm_state.ready=1;
    radduke_log("effects: cooperative MultiVoc PCM ready; 22050 Hz stereo S16\n");
    return 0;
}
void radduke_pcm_shutdown(void) { radduke_pcm_stop();radduke_pcm_state.ready=0; }
int radduke_pcm_begin(char *buffer,int bytes,int divisions,void (*callback)(void))
{
    if(!radduke_pcm_state.ready||!buffer||!callback||bytes<=0||(bytes&3)||divisions<=0||divisions>256)return -1;
    radduke_pcm_stop();
    radduke_pcm_buffer=buffer;radduke_pcm_bytes=(unsigned)bytes;radduke_pcm_divisions=(unsigned)divisions;
    radduke_pcm_current=radduke_pcm_used=0;radduke_pcm_callback=callback;
    callback();++radduke_pcm_state.callbacks; /* Same initial prefill as SDLDrv. */
    radduke_pcm_state.playing=1;
    return 0;
}
void radduke_pcm_lock(void) { ++radduke_pcm_state.lock_depth; }
void radduke_pcm_unlock(void) { if(radduke_pcm_state.lock_depth)--radduke_pcm_state.lock_depth; }
int radduke_pcm_ready(void) { return radduke_pcm_state.ready; }
void radduke_pcm_get_info(radduke_pcm_info *info) { if(info)*info=radduke_pcm_state; }
void radduke_pcm_mix(int16_t *stereo,unsigned frames)
{
    if(!radduke_pcm_state.playing||radduke_pcm_state.lock_depth)return;
    for(unsigned frame=0;frame<frames;++frame) {
        if(radduke_pcm_used==radduke_pcm_bytes) {
            radduke_pcm_callback();++radduke_pcm_state.callbacks;
            radduke_pcm_used=0;
            if(++radduke_pcm_current==radduke_pcm_divisions)radduke_pcm_current=0;
        }
        int16_t sample[2];
        memcpy(sample,radduke_pcm_buffer+radduke_pcm_current*radduke_pcm_bytes+radduke_pcm_used,4);
        radduke_pcm_used+=4;
        for(unsigned c=0;c<2;++c) {
            if(sample[c])++radduke_pcm_state.nonzero_samples;
            int sum=(int)stereo[2*frame+c]+sample[c];
            if(sum>32767){sum=32767;++radduke_pcm_state.clipped_samples;}
            else if(sum< -32768){sum=-32768;++radduke_pcm_state.clipped_samples;}
            stereo[2*frame+c]=(int16_t)sum;
        }
        ++radduke_pcm_state.frames;
    }
}
void radduke_audio_render(int16_t *stereo,unsigned frames)
{
    radduke_music_render(stereo,frames);
    radduke_pcm_mix(stereo,frames);
}
