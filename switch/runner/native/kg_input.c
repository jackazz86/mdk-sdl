/* Controller-to-SDL3 bridge for MDK: the pad becomes keyboard and mouse events. MIT. */
#include <stdio.h>
#include <switch.h>
#include "nroforge_config.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static PadState pad;
static bool initialized;
static Uint64 last_tick;
static Uint64 last_memory_tick;
static uint8_t stick_keys[4];
/* The Capture button: the system tells the application (and takes no screenshot) when the
   application asks for the short-press message. Needs someone to run appletMainLoop (SDL does). */
static AppletHookCookie capture_hook;
static volatile bool capture_pressed;
static int load_tap;                       /* frames left of a synthetic F9 press */
static int save_tap;                       /* frames left of a synthetic F2 press */
static Uint64 minus_down_ns;               /* when Minus went down (real time: a slow frame must not stretch the 2 s) */
static bool minus_fired;                   /* the long press already loaded */

static void on_applet_event(AppletHookType type, void *param)
{
    (void)param;
    if (type == AppletHookType_OnCaptureButtonShortPressed) capture_pressed = true;
}
extern void kg_memory_log(const char *phase);

static void key(SDL_Window *window, SDL_Scancode scan, bool down)
{
    SDL_Event event = {0};
    event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    event.key.windowID = SDL_GetWindowID(window);
    event.key.scancode = scan;
    event.key.key = SDL_GetKeyFromScancode(scan, SDL_KMOD_NONE, false);
    event.key.down = down;
    SDL_PushEvent(&event);
}

static void mouse_button(SDL_Window *window, Uint8 button, bool down)
{
    SDL_Event event = {0};
    event.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
    event.button.windowID = SDL_GetWindowID(window);
    event.button.button = button;
    event.button.down = down;
    event.button.clicks = 1;
    SDL_PushEvent(&event);
}

/* A stick direction held as a key: pushes only the changes. */
static void stick_key(SDL_Window *window, int slot, SDL_Scancode scan, bool held)
{
    if (stick_keys[slot] != held) {
        stick_keys[slot] = held;
        key(window, scan, held);
    }
}


/* ---- Gyro: the console's six-axis sensor as mouse motion -------------------------------------
 * Same principles as the other Switch ports:
 *  - read the whole sensor LIFO (17 states) and average only the FRESH samples, then multiply by
 *    the frame time (one sample per frame loses the motion in between);
 *  - turning is computed in player space: the angular velocity projected on the gravity direction
 *    measured by the accelerometer ("spostamento orizzontale"), plus the wrist-in/wrist-out
 *    gesture (device Y, "inclinazione orizzontale"); looking up and down is device X;
 *  - the pad that is actually paired wins over the console's own sensor.
 * libnx reports angular velocity in rotations per second. Gains and signs live in
 * <SD root>/gyro.cfg (key value per line) so a wrong direction is a minus sign, not a rebuild. */
enum { S_HANDHELD, S_FULLKEY, S_DUAL_R, S_LEFT, S_RIGHT, S_COUNT };
static HidSixAxisSensorHandle gyro_handle[S_COUNT];
static uint64_t gyro_last[S_COUNT];
static bool gyro_started;
static bool gyro_on = true;
static float cfg_sweep = -1.5f, cfg_tilt = -1.5f, cfg_pitch = -1.8f, cfg_deadzone = 2.0f;
static int cfg_debug = 0;
/* Mouse units per degree: MDK turns 0.15 degrees per unit of mouse motion at sensitivity 1. */
#define MOUSE_UNITS_PER_DEGREE (1.0f / 0.15f)

static void gyro_load_config(void)
{
    FILE *f = fopen(NROFORGE_SD_ROOT "/gyro.cfg", "r");
    if (f) {
        char line[128];
        while (fgets(line, sizeof line, f)) {
            char name[64];
            float value;
            if (line[0] == '#' || sscanf(line, "%63s %f", name, &value) != 2) continue;
            if (!strcmp(name, "gyro")) gyro_on = value != 0.0f;
            else if (!strcmp(name, "gyro_sweep")) cfg_sweep = value;
            else if (!strcmp(name, "gyro_tilt")) cfg_tilt = value;
            else if (!strcmp(name, "gyro_pitch")) cfg_pitch = value;
            else if (!strcmp(name, "gyro_deadzone")) cfg_deadzone = value;
            else if (!strcmp(name, "gyro_debug")) cfg_debug = (int)value;
        }
        fclose(f);
    }
    fprintf(stderr, "[gyro] cfg %s: on=%d sweep=%.2f tilt=%.2f pitch=%.2f deadzone=%.2f debug=%d\n",
        f ? "read" : "absent (defaults)", gyro_on, cfg_sweep, cfg_tilt, cfg_pitch, cfg_deadzone, cfg_debug);
    fflush(stderr);
}

static void gyro_init(void)
{
    gyro_load_config();
    hidGetSixAxisSensorHandles(&gyro_handle[S_HANDHELD], 1, HidNpadIdType_Handheld, HidNpadStyleTag_NpadHandheld);
    hidGetSixAxisSensorHandles(&gyro_handle[S_FULLKEY], 1, HidNpadIdType_No1, HidNpadStyleTag_NpadFullKey);
    HidSixAxisSensorHandle dual[2];
    hidGetSixAxisSensorHandles(dual, 2, HidNpadIdType_No1, HidNpadStyleTag_NpadJoyDual);
    gyro_handle[S_DUAL_R] = dual[1];
    hidGetSixAxisSensorHandles(&gyro_handle[S_LEFT], 1, HidNpadIdType_No1, HidNpadStyleTag_NpadJoyLeft);
    hidGetSixAxisSensorHandles(&gyro_handle[S_RIGHT], 1, HidNpadIdType_No1, HidNpadStyleTag_NpadJoyRight);
    for (int i = 0; i < S_COUNT; i++) hidStartSixAxisSensor(gyro_handle[i]);
    gyro_started = true;
}

/* The sensor of the pad that is paired to player 1, else the console's own. */
static int gyro_active(void)
{
    uint32_t p1 = hidGetNpadStyleSet(HidNpadIdType_No1);
    if (p1 & HidNpadStyleTag_NpadFullKey) return S_FULLKEY;
    if (p1 & HidNpadStyleTag_NpadJoyDual) return S_DUAL_R;
    if (p1 & HidNpadStyleTag_NpadJoyRight) return S_RIGHT;
    if (p1 & HidNpadStyleTag_NpadJoyLeft) return S_LEFT;
    return S_HANDHELD;
}

static double dead(double v, double dz)
{
    if (fabs(v) <= dz) return 0.0;
    return v > 0 ? v - dz : v + dz;
}

/* Mouse motion (x, y in mouse units) from the gyro over this frame; false: nothing fresh. */
static bool gyro_motion(float delta, float *mx, float *my)
{
    if (!gyro_started) gyro_init();
    if (!gyro_on || delta <= 0.0f) return false;
    int which = gyro_active();
    HidSixAxisSensorState st[17];
    int n = hidGetSixAxisSensorStates(gyro_handle[which], st, 17);
    if (n <= 0) return false;
    /* newest first: the fresh ones are those after the last sampling number consumed */
    double w[3] = {0, 0, 0}, a[3] = {0, 0, 0};
    int fresh = 0;
    for (int i = 0; i < n; i++) {
        if (st[i].sampling_number <= gyro_last[which]) break;
        w[0] += st[i].angular_velocity.x; w[1] += st[i].angular_velocity.y; w[2] += st[i].angular_velocity.z;
        a[0] += st[i].acceleration.x; a[1] += st[i].acceleration.y; a[2] += st[i].acceleration.z;
        fresh++;
    }
    gyro_last[which] = st[0].sampling_number;
    if (fresh == 0) return false;
    for (int k = 0; k < 3; k++) { w[k] = w[k] / fresh * 360.0; a[k] /= fresh; }   /* deg/s */
    double alen = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    double sweep = alen > 0.1 ? (w[0] * a[0] + w[1] * a[1] + w[2] * a[2]) / alen : 0.0;
    /* Measured on the console (handheld): with the screen upright, rotating about device Y is rotating
       about the vertical, and the gravity projection of the same motion comes out with the OPPOSITE
       sign (accelerometer y = -0.95). Added as they were, the two terms cancelled. The wrist-in/out
       term (device Y) is therefore taken with the sign that agrees with the projection, from where the
       gravity points: it holds on any device and pose, no per-device switch. Skipped when Y is
       nearly horizontal (a sign would be noise). */
    double ay = alen > 0.1 ? a[1] / alen : 0.0;
    double tilt = fabs(ay) > 0.2 ? w[1] * (ay > 0.0 ? 1.0 : -1.0) : 0.0;
    double nod = w[0];
    if (cfg_debug) {
        static float t_log;
        t_log += delta;
        if (t_log >= 0.25f) {
            t_log = 0;
            fprintf(stderr, "[gyro] sensor=%d fresh=%d w=(%.1f %.1f %.1f) deg/s a=(%.2f %.2f %.2f) sweep=%.1f\n",
                which, fresh, w[0], w[1], w[2], a[0], a[1], a[2], sweep);
            fflush(stderr);
        }
    }
    /* positive mouse X turns right, positive Y looks down */
    *mx = (float)((-cfg_sweep * dead(sweep, cfg_deadzone) - cfg_tilt * dead(tilt, cfg_deadzone)) * delta * MOUSE_UNITS_PER_DEGREE);
    *my = (float)((cfg_pitch * dead(nod, cfg_deadzone)) * delta * MOUSE_UNITS_PER_DEGREE);
    return true;
}

void kg_pump_input(SDL_Window *window)
{
    if (!initialized) {
        padConfigureInput(1, HidNpadStyleSet_NpadStandard);
        padInitializeDefault(&pad);
        appletHook(&capture_hook, on_applet_event, NULL);
        appletSetRequiresCaptureButtonShortPressedMessage(true);
        last_tick = SDL_GetTicksNS();
        initialized = true;
    }
    padUpdate(&pad);
    uint64_t down = padGetButtonsDown(&pad), up = padGetButtonsUp(&pad);
    const struct { uint64_t button; SDL_Scancode scan; } keys[] = {
        {HidNpadButton_A, SDL_SCANCODE_SPACE},          /* jump, menu accept */
        {HidNpadButton_B, SDL_SCANCODE_LSHIFT},         /* turbo */
        {HidNpadButton_X, SDL_SCANCODE_RETURN},         /* use the item, menu accept */
        {HidNpadButton_Y, SDL_SCANCODE_TAB},            /* next item */
        {HidNpadButton_L, SDL_SCANCODE_LEFTBRACKET},    /* previous item / ammo */
        {HidNpadButton_R, SDL_SCANCODE_RIGHTBRACKET},   /* next item / ammo */
        {HidNpadButton_ZR, SDL_SCANCODE_LCTRL},         /* fire */
        {HidNpadButton_Plus, SDL_SCANCODE_ESCAPE},      /* pause menu / back */
        {HidNpadButton_Left, SDL_SCANCODE_LEFT},
        {HidNpadButton_Right, SDL_SCANCODE_RIGHT},
        {HidNpadButton_Up, SDL_SCANCODE_UP},
        {HidNpadButton_Down, SDL_SCANCODE_DOWN},
        /* The sniper scope's zoom, as the author's own pad map has it: D-pad up zooms in, down zooms out
           (the same keys as the arrows: Kurt stands still in the scope, and the menus need the arrows). */
        {HidNpadButton_Up, SDL_SCANCODE_PAGEUP},
        {HidNpadButton_Down, SDL_SCANCODE_PAGEDOWN},
    };
    for (size_t i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
        if (down & keys[i].button) key(window, keys[i].scan, true);
        if (up & keys[i].button) key(window, keys[i].scan, false);
    }
    /* ZL: sniper mode, the right mouse button. */
    if (down & HidNpadButton_ZL) mouse_button(window, SDL_BUTTON_RIGHT, true);
    if (up & HidNpadButton_ZL) mouse_button(window, SDL_BUTTON_RIGHT, false);

    Uint64 tick = SDL_GetTicksNS();
    if (tick - last_memory_tick >= 5000000000ULL) {
        last_memory_tick = tick;
        kg_memory_log("frame");
    }
    float delta = fminf((tick - last_tick) / 1000000000.0f, 0.05f);
    last_tick = tick;

    /* Left stick: W A S D. */
    HidAnalogStickState left = padGetStickPos(&pad, 0);
    const int threshold = 11000;
    stick_key(window, 0, SDL_SCANCODE_W, left.y > threshold);
    stick_key(window, 1, SDL_SCANCODE_S, left.y < -threshold);
    stick_key(window, 2, SDL_SCANCODE_A, left.x < -threshold);
    stick_key(window, 3, SDL_SCANCODE_D, left.x > threshold);

    /* Minus: a short press is the quick save (F2, on release), held for 2 seconds the quick load (F9). */
    if (down & HidNpadButton_Minus) {
        minus_down_ns = tick;
        minus_fired = false;
    }
    if (padGetButtons(&pad) & HidNpadButton_Minus) {
        if (!minus_fired && tick - minus_down_ns >= 2000000000ULL) {
            minus_fired = true;
            load_tap = 2;
        }
    }
    if ((up & HidNpadButton_Minus) && !minus_fired) save_tap = 2;
    if (save_tap == 2) key(window, SDL_SCANCODE_F2, true);
    if (save_tap == 1) key(window, SDL_SCANCODE_F2, false);
    if (save_tap > 0) save_tap--;
    /* The Capture button: also a quick load, a two-frame F9 press so the game sees a key press. */
    if (capture_pressed) {
        capture_pressed = false;
        load_tap = 2;
    }
    if (load_tap == 2) key(window, SDL_SCANCODE_F9, true);
    if (load_tap == 1) key(window, SDL_SCANCODE_F9, false);
    if (load_tap > 0) load_tap--;
    if (down & HidNpadButton_StickR) {
        gyro_on = !gyro_on;
        fprintf(stderr, "[gyro] %s\n", gyro_on ? "on" : "off");
        fflush(stderr);
    }

    /* Look: the right stick plus the gyro, as relative mouse motion. Only in play, where the game
       has captured the mouse (the menus would see the pointer move). */
    float lx = 0.0f, ly = 0.0f;
    HidAnalogStickState right = padGetStickPos(&pad, 1);
    if (abs(right.x) > 4000 || abs(right.y) > 4000) {
        const float pixels_per_second = 700.0f;
        lx = right.x / 32768.0f * pixels_per_second * delta;
        ly = -right.y / 32768.0f * pixels_per_second * delta;
    }
    float gx = 0.0f, gy = 0.0f;
    if (gyro_motion(delta, &gx, &gy)) {
        lx += gx;
        ly += gy;
    }
    if ((lx != 0.0f || ly != 0.0f) && SDL_GetWindowRelativeMouseMode(window)) {
        SDL_Event event = {0};
        event.type = SDL_EVENT_MOUSE_MOTION;
        event.motion.windowID = SDL_GetWindowID(window);
        event.motion.xrel = lx;
        event.motion.yrel = ly;
        SDL_PushEvent(&event);
    }
}

const char *kg_prompt_path(const char *initial, bool save)
{
    (void)initial; (void)save;
    return NULL;
}

void *getsym_KGSwitch(const char *name)
{
    if (!strcmp(name, "kg_pump_input")) return (void *)kg_pump_input;
    if (!strcmp(name, "kg_prompt_path")) return (void *)kg_prompt_path;
    return NULL;
}
