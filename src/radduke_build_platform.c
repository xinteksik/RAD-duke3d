/* New platform implementation of the existing JFBuild base services ABI.
 * No SDL emulation. New application-facing services have radduke_ names.
 * Used privately with pinned JFDuke3D/JFBuild; no aggregate license claim. */
#include "build.h"
#include "baselayer.h"
#include "baselayer_priv.h"
#include "osd.h"
#include "a.h"
#include "radduke_platform.h"
#include <stdarg.h>

int displaycnt = 1;
static uint8_t radduke_frame[320 * 200];
static uint8_t radduke_raw_down[256];
static uint16_t radduke_code_holds[256];
static unsigned radduke_timer_hz;
static uint64_t radduke_timer_sample;
static void (*radduke_timer_callback)(void);

int radduke_main(void)
{
    static const char *args[] = {"radduke", "-nosetup", "-nm", "-ns", NULL};
    _buildargc = 4;
    _buildargv = args;
    startwin_open();
    baselayer_init();
    return app_main(_buildargc, args);
}
int initsystem(void) { radduke_log("RADDUKE base services: indexed 320x200, silent\n"); return 0; }
void uninitsystem(void) { uninittimer(); }
int initinput(void) { releaseallbuttons(); inputdevices = 1; return 0; }
void uninitinput(void) { releaseallbuttons(); }
void releaseallbuttons(void)
{
    memset(keystatus, 0, sizeof(keystatus));
    memset(radduke_raw_down, 0, sizeof(radduke_raw_down));
    memset(radduke_code_holds, 0, sizeof(radduke_code_holds));
}
const char *getkeyname(int n)
{
    switch (n) {
    case 0xc8: return "I"; case 0xd0: return "K";
    case 0xcb: return "J"; case 0xcd: return "L";
    case 0x1c: return "Return"; case 0x1d: return "Commodore / F";
    case 0x1e: return "A"; case 0x2c: return "Z";
    case 0x39: return "Space";
    case 0x2a: return "Left Shift"; case 0x36: return "Right Shift";
    case 0x02: return "1"; case 0x03: return "2";
    case 0x04: return "3"; case 0x05: return "4";
    case 0x06: return "5"; case 0x07: return "6";
    case 0x08: return "7"; case 0x09: return "8";
    case 0x0a: return "9"; case 0x0b: return "0";
    default: return "Key";
    }
}
const char *getjoyname(int w, int n) { (void)w; (void)n; return NULL; }
int initmouse(void) { return 0; }
void uninitmouse(void) {}
void grabmouse(int a) { (void)a; }
void readmousexy(int *x, int *y) { *x = *y = 0; }
void readmousebstatus(int *b) { *b = 0; }

void radduke_key_event(unsigned key, int down)
{
    int code = 0, ascii = 0;
    if (key > 255) return;
    if (key >= 'a' && key <= 'z') key -= 'a' - 'A';
    switch (key) {
    case 'I': code = 0xc8; break; /* Up */
    case 'K': code = 0xd0; break; /* Down */
    case 'J': code = 0xcb; break; /* Left */
    case 'L': code = 0xcd; break; /* Right */
    case 4: case 'F': code = 0x1d; break; /* Commodore/F -> Fire (LCtrl) */
    case 'A': code = 0x1e; break; /* Jump */
    case 'Z': code = 0x2c; break; /* Crouch */
    case 1: code = 0x2a; break; /* C64 left Shift -> Run */
    case 6: code = 0x36; break; /* C64 right Shift -> Run */
    case 13: code = 0x1c; ascii = 13; break;
    case 95: case 27: code = 1; ascii = 27; break;
    case 32: code = 0x39; ascii = 32; break;
    case 'Y': code = 0x15; ascii = 'y'; break;
    case 'N': code = 0x31; ascii = 'n'; break;
    default:
        if (key >= '1' && key <= '9') code = 0x02 + (int)(key - '1');
        else if (key == '0') code = 0x0b;
        else return;
        ascii = (int)key;
        break;
    }
    down = !!down;
    if (radduke_raw_down[key] == down) return;
    radduke_raw_down[key] = (uint8_t)down;
    if (down) ++radduke_code_holds[code];
    else if (radduke_code_holds[code]) --radduke_code_holds[code];
    down = radduke_code_holds[code] != 0;
    if (!!keystatus[code] == down) return;
    keystatus[code] = (char)down;
    /* Never lose releases, even when the FIFO is full. Held state above is
     * authoritative; drop the oldest edge to retain the new edge. */
    int next = (keyfifoend + 2) & (KEYFIFOSIZ - 1);
    if (next == keyfifoplc) keyfifoplc = (keyfifoplc + 2) & (KEYFIFOSIZ - 1);
    keyfifo[keyfifoend] = code;
    keyfifo[(keyfifoend + 1) & (KEYFIFOSIZ - 1)] = down;
    keyfifoend = next;
    if (down && ascii) {
        next = (keyasciififoend + 1) & (KEYFIFOSIZ - 1);
        if (next != keyasciififoplc) {
            keyasciififo[keyasciififoend] = (uint8_t)ascii;
            keyasciififoend = next;
        }
    }
}
int handleevents(void) { radduke_poll(); sampletimer(); return 0; }
void radduke_wait_ms(unsigned ms)
{
    uint64_t end = radduke_time_us() + (uint64_t)ms * 1000;
    while (radduke_time_us() < end) { radduke_poll(); sampletimer(); }
}
int inittimer(int hz, void (*callback)(void))
{
    if (hz <= 0) return -1;
    radduke_timer_hz = (unsigned)hz;
    radduke_timer_sample = radduke_time_us() * radduke_timer_hz / 1000000;
    radduke_timer_callback = callback;
    return 0;
}
void uninittimer(void) { radduke_timer_hz = 0; radduke_timer_callback = NULL; }
void sampletimer(void)
{
    if (!radduke_timer_hz) return;
    uint64_t now = radduke_time_us() * radduke_timer_hz / 1000000;
    uint64_t n = now - radduke_timer_sample;
    radduke_timer_sample = now;
    totalclock += (int)n;
    if (radduke_timer_callback) while (n--) radduke_timer_callback();
}
unsigned int getticks(void) { return (unsigned)(radduke_time_us() / 1000); }
unsigned int getusecticks(void) { return (unsigned)radduke_time_us(); }
int gettimerfreq(void) { return (int)radduke_timer_hz; }
void getvalidmodes(void)
{
    if (!validmodecnt) {
        addvalidmode(320, 200, 8, 0, 0, 0, 0);
        addvalidmode(320, 200, 8, 1, 0, 50, 0);
    }
}
int setvideomode(int width, int height, int bits, int fs)
{
    if (width != 320 || height != 200 || bits != 8) return -1;
    if (baselayer_videomodewillchange) baselayer_videomodewillchange();
    memset(radduke_frame, 0, sizeof(radduke_frame));
    xres = width; yres = height; bpp = bits; fullscreen = fs;
    bytesperline = width; imageSize = width * height; numpages = 1;
    frameplace = (intptr_t)radduke_frame;
    setvlinebpl(bytesperline);
    for (int i = 0; i <= height; ++i) ylookup[i] = i * bytesperline;
    videomodereset = 0;
    if (baselayer_videomodedidchange) baselayer_videomodedidchange();
    OSD_ResizeDisplay(width, height);
    setpalettefade(palfadergb.r, palfadergb.g, palfadergb.b, palfadedelta);
    return 0;
}
void showframe(void)
{
    if (frameplace) radduke_present((const uint8_t *)frameplace, xres, yres,
                                   bytesperline, (const uint8_t *)curpalettefaded);
    radduke_poll(); sampletimer();
}
int setpalette(int start, int num, unsigned char *p) { (void)start; (void)num; (void)p; return 0; }
int setsysgamma(float s, float g) { (void)s; (void)g; return -1; }
const char *getdisplayname(int d) { return d == 0 ? "RAD PAL C64" : NULL; }
static void radduke_message(const char *format, va_list args)
{
    char text[1024]; vsnprintf(text, sizeof(text), format, args); radduke_log(text);
}
int wm_msgbox(const char *name, const char *format, ...)
{ (void)name; va_list a; va_start(a, format); radduke_message(format, a); va_end(a); return 0; }
int wm_ynbox(const char *name, const char *format, ...)
{ (void)name; va_list a; va_start(a, format); radduke_message(format, a); va_end(a); return 0; }
int wm_filechooser(const char *a, const char *b, const char *c, int d, char **e)
{ (void)a; (void)b; (void)c; (void)d; if (e) *e = NULL; return -1; }
int wm_idle(void *p) { (void)p; radduke_poll(); return 0; }
void wm_allowbackgroundidle(int a) { (void)a; }
void wm_allowtaskswitching(int a) { (void)a; }
void wm_setapptitle(const char *a) { (void)a; }
void wm_setwindowtitle(const char *a) { (void)a; }
void initputs(const char *s) { radduke_log(s); }
void debugprintf(const char *f, ...) { va_list a; va_start(a, f); radduke_message(f, a); va_end(a); }
