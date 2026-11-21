/* Host-only diagnostic: real ownership and edition rules remain unchanged. */
#include "duke3d.h"
#include "radduke_platform.h"
#include <stdio.h>
#include <stdlib.h>
int radduke_host_weapon_probe(unsigned frame)
{
    static uint64_t start;
    static unsigned slot,phase;
    static FILE *log;
    static const char keys[]="1234567890";
    if(!(ps[0].gm&MODE_GAME)||ps[0].cursectnum<0){
        if(frame==20||frame==40||frame==60)radduke_key_event(13,1);
        if(frame==22||frame==42||frame==62)radduke_key_event(13,0);
        return 0;
    }
    if(!start){
        /* Explicit host-only fixture; never compiled into the firmware. */
        if (getenv("RADDUKE_WEAPON_FIXTURE")) {
            for (unsigned w=0; w<6; ++w) {
                ps[0].gotweapon[w]=1;
                if (w) ps[0].ammo_amount[w]=20;
            }
        }
        start=radduke_time_us();log=fopen("weapons.csv","w");if(!log)return -1;
        fputs("digit,phase,ms,weapon,request,shareware,weapon_pos,kickback,xrepeat,owned,game_tics\n",log);
    }
    unsigned ms=(unsigned)((radduke_time_us()-start)/1000);
    if(slot==10){if(ms>12500){fclose(log);return 1;}return 0;}
    unsigned due=2500+slot*900+(phase==0?0:phase==1?120:700);
    if(ms<due)return 0;
    if(phase==0)radduke_key_event((unsigned)keys[slot],1);
    else{
        fprintf(log,"%c,%s,%u,%d,%u,%d,%d,%d,%d,%d,%d\n",keys[slot],phase==1?"held":"settled",ms,
                ps[0].curr_weapon,(sync[0].bits>>8)&15,VOLUMEONE?1:0,
                ps[0].weapon_pos,ps[0].kickback_pic,sprite[ps[0].i].xrepeat,
                ps[0].gotweapon[slot],totalclock);fflush(log);
        if(phase==1)radduke_key_event((unsigned)keys[slot],0);
    }
    if(++phase==3){phase=0;++slot;}
    return 0;
}
