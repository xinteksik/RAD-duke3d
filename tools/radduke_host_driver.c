/* Host implementation of the small services boundary used by the RAD backend.
 * SDL is confined here; the JFBuild adapter itself has no SDL dependency. */
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include "radduke_platform.h"
#ifdef RADDUKE_MUSIC
#include "radduke_music.h"
#ifdef RADDUKE_EFFECTS
#include "radduke_pcm.h"
static void radduke_save_effects_stats(void)
{
    if(!getenv("RADDUKE_CAPTURE_MUSIC"))return;
    radduke_pcm_info info;radduke_pcm_get_info(&info);
    FILE *f=fopen("effects-stats.json","w");
    if(f){fprintf(f,"{\"frames\":%llu,\"callbacks\":%llu,\"nonzero_samples\":%llu,\"clipped_samples\":%llu}\n",
       (unsigned long long)info.frames,(unsigned long long)info.callbacks,
       (unsigned long long)info.nonzero_samples,(unsigned long long)info.clipped_samples);fclose(f);}
}
#endif
#ifdef RADDUKE_STEREO
#include "radduke_audio_config.h"
#endif
static SDL_AudioDeviceID radduke_audio;
static FILE *radduke_pcm_capture;
void radduke_music_service(void)
{
    if (!radduke_audio) return;
    while (SDL_GetQueuedAudioSize(radduke_audio) < 2205 * 4) {
        int16_t pcm[256 * 2];
#ifdef RADDUKE_EFFECTS
        radduke_audio_render(pcm,256);
#else
        radduke_music_render(pcm, 256);
#endif
        if (radduke_pcm_capture) fwrite(pcm, sizeof(pcm), 1, radduke_pcm_capture);
        SDL_QueueAudio(radduke_audio, pcm, sizeof(pcm));
    }
}
#endif
static SDL_Window *radduke_window;
static unsigned radduke_frames;
static int radduke_automatic;
static int radduke_gameplay;
extern int radduke_host_game_probe(unsigned frame);
extern int current_menu;
extern short probey;
static void radduke_watchdog(int signal_number)
{
    void *frames[32];
    (void)signal_number;
    int count = backtrace(frames, 32);
    backtrace_symbols_fd(frames, count, 2);
    _Exit(70);
}
uint64_t radduke_time_us(void)
{ return SDL_GetPerformanceCounter() * UINT64_C(1000000) / SDL_GetPerformanceFrequency(); }
void radduke_log(const char *s) { fputs(s, stderr); }
void radduke_poll(void)
{
#ifdef RADDUKE_MUSIC
    radduke_music_service();
#endif
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) exit(0);
        if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            unsigned key = (unsigned)e.key.keysym.sym;
            if (key == SDLK_ESCAPE) key = 95;
            if (key == SDLK_LSHIFT) key = 1;
            if (key == SDLK_RSHIFT) key = 6;
            if (key == SDLK_LCTRL) key = 4;
            radduke_key_event(key, e.type == SDL_KEYDOWN);
        }
    }
}
void radduke_present(const uint8_t *p, int w, int h, int pitch, const uint8_t *pal)
{
    SDL_Surface *s = SDL_GetWindowSurface(radduke_window);
    SDL_LockSurface(s);
    for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
        const uint8_t *c = pal + 4 * p[y * pitch + x];
        ((uint32_t *)((uint8_t *)s->pixels + y * s->pitch))[x] = SDL_MapRGB(s->format, c[0], c[1], c[2]);
    }
    SDL_UnlockSurface(s);
    SDL_UpdateWindowSurface(radduke_window);
    ++radduke_frames;
    if (radduke_gameplay) {
        int result = radduke_host_game_probe(radduke_frames);
        if (result) exit(result > 0 ? 0 : 71);
    } else if (radduke_automatic) {
        if (radduke_frames == 10 || radduke_frames == 30 || radduke_frames == 50 || radduke_frames == 80) {
            char path[64]; snprintf(path, sizeof(path), "private-menu-%u.bmp", radduke_frames);
            SDL_SaveBMP(s, path);
            FILE *proof = fopen("menu-input.csv", radduke_frames == 10 ? "w" : "a");
            if (proof) { fprintf(proof, "%u,%d,%d\n", radduke_frames, current_menu, probey); fclose(proof); }
        }
        if (radduke_frames == 20) radduke_key_event('K', 1);
        if (radduke_frames == 22) radduke_key_event('K', 0);
        if (radduke_frames == 40) radduke_key_event('I', 1);
        if (radduke_frames == 42) radduke_key_event('I', 0);
        if (radduke_frames == 60) radduke_key_event(13, 1);
        if (radduke_frames == 62) radduke_key_event(13, 0);
        if (radduke_frames == 100) { fprintf(stderr, "RADDUKE_ADAPTER_MENU_PROBE_DONE\n"); exit(0); }
    }
    SDL_Delay(20);
}
int main(void)
{
    radduke_automatic = getenv("RADDUKE_AUTOMATIC") != NULL;
    radduke_gameplay = getenv("RADDUKE_GAMEPLAY") != NULL;
#ifdef RADDUKE_EFFECTS
    atexit(radduke_save_effects_stats);
#endif
    if (radduke_automatic || radduke_gameplay) { signal(SIGALRM, radduke_watchdog); alarm(radduke_gameplay ? 20 : 10); }
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) return 1;
#ifdef RADDUKE_MUSIC
    if (SDL_InitSubSystem(SDL_INIT_AUDIO)) return 2;
    SDL_AudioSpec desired = {0}, obtained;
    desired.freq = 22050; desired.format = AUDIO_S16SYS; desired.channels = 2; desired.samples = 512;
    radduke_audio = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
    if (!radduke_audio) return 3;
    if (getenv("RADDUKE_CAPTURE_MUSIC")) radduke_pcm_capture = fopen("music-output.pcm", "wb");
    SDL_PauseAudioDevice(radduke_audio, 0);
#ifdef RADDUKE_STEREO
    radduke_audio_config config;
    const char *config_path = getenv("RADDUKE_AUDIO_CONFIG");
    int config_status = radduke_audio_config_load(&config, config_path ? config_path : "AUDIO.CFG");
    radduke_music_configure_stereo(config.stereo ? config.music_width : 0);
    fprintf(stderr, "host audio: config=%d stereo-requested=%u MIDI_STEREO_WIDTH=%u (no physical SID detection)\n",
            config_status, config.stereo, config.stereo ? config.music_width : 0);
#endif
#endif
    radduke_window = SDL_CreateWindow("RAD DUKE3D services host", 0, 0, 320, 200, 0);
    if (!radduke_window) return 1;
    return radduke_main();
}
