/* Uses the shared raw C64 input boundary and observes actual Duke game state. */
#include "duke3d.h"
#include "radduke_platform.h"
#include <stdio.h>
#include <stdlib.h>
extern int radduke_host_weapon_probe(unsigned frame);

struct radduke_step { unsigned ms, raw; int down; const char *name; };
static const struct radduke_step radduke_steps[] = {
    {0,0,0,"initial"}, {500,'I',1,"forward_down"}, {900,0,0,"walking"},
    {1000,1,1,"run_down"}, {1300,0,0,"running"}, {1400,'I',0,"forward_up"},
    {1450,1,0,"run_up"}, {1800,0,0,"settled"},
    {1900,4,1,"commodore_down"}, {2200,'F',1,"f_down"},
    {2300,4,0,"commodore_up"}, {2500,0,0,"fire_alias_held"},
    {2600,'F',0,"f_up"}, {2900,0,0,"after_fire"},
    {3100,'A',1,"jump_down"}, {3200,'A',0,"jump_up"},
    {3400,0,0,"airborne"}, {4300,0,0,"landed"},
    {4500,'Z',1,"crouch_down"}, {4800,0,0,"crouched"},
    {4900,'Z',0,"crouch_up"}, {5200,0,0,"standing"},
    {5400,32,1,"use_down"}, {5450,0,0,"using"},
    {5500,32,0,"use_up"}, {5800,0,0,"released"}
};
int radduke_host_game_probe(unsigned frame)
{
    if(getenv("RADDUKE_WEAPON_TEST"))return radduke_host_weapon_probe(frame);
    static uint64_t start;
    static unsigned step;
    static FILE *log;
    if (!(ps[0].gm & MODE_GAME) || ps[0].cursectnum < 0) {
        if (frame == 20 || frame == 40 || frame == 60) radduke_key_event(13, 1);
        if (frame == 22 || frame == 42 || frame == 62) radduke_key_event(13, 0);
        return 0;
    }
    if (!start) {
        start = radduke_time_us();
        log = fopen("gameplay-input.csv", "w");
        if (!log) return -1;
        fputs("stage,ms,x,y,z,gm,ammo,on_ground,weapon,bits,fvel,svel\n", log);
    }
    uint64_t ms = (radduke_time_us() - start) / 1000;
    if (step == sizeof(radduke_steps)/sizeof(radduke_steps[0])) {
        if (ms > 6100) { fclose(log); return 1; }
        return 0;
    }
    const struct radduke_step *s = &radduke_steps[step];
    if (ms < s->ms) return 0;
    ++step;
    fprintf(log, "%s,%llu,%d,%d,%d,%u,%d,%u,%d,%u,%d,%d\n", s->name,
            (unsigned long long)ms, ps[0].posx, ps[0].posy, ps[0].posz,
            ps[0].gm, ps[0].ammo_amount[1], ps[0].on_ground, ps[0].curr_weapon,
            sync[0].bits, sync[0].fvel, sync[0].svel);
    fflush(log);
    if (s->raw) radduke_key_event(s->raw, s->down);
    return 0;
}
