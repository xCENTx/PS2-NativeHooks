#include "ui.h"

/* 1. Immediate-mode layout and widgets */
/* Native drawing primitives remain independent of window layout. */
void DrawMenuRect(float x, float y, float w, float h, Vec4 color);
void DrawCheckbox(float x, float y, float size, int checked, Vec4 color);
void DrawText(const char *text, float x, float y, float scale, Vec4 color)
{
    DrawString2D(text, x, y, scale, color);
}
void UI_BeginWindow(UIContext *ui, UIState *state, const UILayout *layout,
                    const UIStyle *style, unsigned pass, unsigned pressed)
{
    unsigned i;
    ui->state=state; ui->layout=*layout; ui->style=*style;
    ui->pass=pass; ui->pressed=pressed; ui->count=0;
    ui->focusSeen=0; ui->first=0; ui->overflow=0;
    ui->content=(UIRect){layout->x+layout->padding, layout->y+layout->padding,
                         layout->width-2*layout->padding,
                         state->height>2*layout->padding ? state->height-2*layout->padding : 0};
    ui->cursorY=ui->content.y;
    ui->lastRow=(UIRect){0,0,0,0};
    if (pass == UI_INPUT && state->count) {
        for (i=0; i<state->count && state->ids[i]!=state->focus; ++i) {}
        if (i==state->count) i=0;
        if (pressed & UI_UP) i=(i+state->count-1)%state->count;
        else if (pressed & UI_DOWN) i=(i+1)%state->count;
        state->focus=state->ids[i];
    }
    if (pass == UI_DRAW) {
        DrawMenuRect(layout->x, layout->y, layout->width, state->height, style->background);
        DrawMenuRect(layout->x, layout->y, layout->width, style->accentHeight, style->accent);
    }
}
void UI_EndWindow(UIContext *ui)
{
    ui->content.height=ui->cursorY-ui->content.y;
    if (ui->pass==UI_INPUT) {
        ui->state->count=ui->count;
        if (!ui->focusSeen) ui->state->focus=ui->first;
        ui->state->height=ui->content.height+2*ui->layout.padding;
    }
}
void UI_Spacing(UIContext *ui, float pixels)
{
    if (pixels>0) ui->cursorY+=pixels;
}
void UI_Text(UIContext *ui, const char *text, float scale, Vec4 color)
{
    float height;
    if (!(scale>0)) return;
    height=ui->style.textHeight*scale;
    if (ui->pass==UI_DRAW)
        DrawText(text, ui->content.x, ui->cursorY+height*0.5f+
                 ui->style.baselineOffset*scale, scale, color);
    ui->cursorY+=height;
}
static int row(UIContext *ui, u32 id, const char *label, float scale)
{
    float height=ui->style.textHeight*scale+2*ui->style.rowPadding;
    int focused;
    if (height<ui->style.rowHeight) height=ui->style.rowHeight;
    ui->lastRow=(UIRect){ui->content.x,ui->cursorY,ui->content.width,height};
    ui->cursorY+=height+ui->style.rowGap;
    if (!id || ui->count>=UI_MAX_ITEMS) { ui->overflow=1; return 0; }
    if (!ui->first) ui->first=id;
    if (!ui->state->focus && ui->pass==UI_INPUT) ui->state->focus=id;
    focused=ui->state->focus==id;
    if (focused) ui->focusSeen=1;
    if (ui->pass==UI_INPUT) ui->state->ids[ui->count]=id;
    ++ui->count;
    if (ui->pass==UI_DRAW) {
        if (focused) {
            DrawMenuRect(ui->lastRow.x,ui->lastRow.y,ui->lastRow.width,height,ui->style.highlight);
            DrawMenuRect(ui->lastRow.x,ui->lastRow.y,3,height,ui->style.accent);
        }
        DrawText(label,ui->lastRow.x+ui->style.rowPadding,
                 ui->lastRow.y+height*0.5f+ui->style.baselineOffset*scale,
                 scale,ui->style.text);
    }
    return focused && ui->pass==UI_INPUT;
}
static float valueX(const UIContext *ui)
{
    return ui->lastRow.x+ui->lastRow.width*ui->style.valueFraction;
}
int UI_Checkbox(UIContext *ui, u32 id, const char *label, bool *value, float scale)
{
    int changed=row(ui,id,label,scale) && (ui->pressed & (UI_LEFT|UI_RIGHT|UI_ACTIVATE));
    if (changed) *value=!*value;
    if (ui->pass==UI_DRAW) {
        float size=16*scale;
        DrawCheckbox(valueX(ui),ui->lastRow.y+(ui->lastRow.height-size)*0.5f,
                     size,*value,ui->state->focus==id ? ui->style.accent : ui->style.muted);
    }
    return changed;
}
int UI_Combo(UIContext *ui, u32 id, const char *label, unsigned *value,
             const char *const *names, unsigned count, float scale)
{
    int active=row(ui,id,label,scale), changed=0;
    unsigned index=count ? *value%count : 0;
    if (count && active && (ui->pressed & (UI_LEFT|UI_RIGHT|UI_ACTIVATE))) {
        index=(index+((ui->pressed & UI_LEFT) ? count-1 : 1))%count;
        *value=index; changed=1;
    }
    if (count && ui->pass==UI_DRAW)
        DrawText(names[index],valueX(ui),ui->lastRow.y+ui->lastRow.height*0.5f+
                 ui->style.baselineOffset*scale,scale,
                 ui->state->focus==id ? ui->style.accent : ui->style.muted);
    return changed;
}
int UI_Button(UIContext *ui, u32 id, const char *label, float scale)
{
    return row(ui,id,label,scale) && (ui->pressed & UI_ACTIVATE)!=0;
}


/* Coordinates are the square's top-left in native HUD pixels.
 * The unpainted gap lets the existing menu/highlight show through. */
void DrawCheckbox(float x, float y, float size, int checked, Vec4 color)
{
    const float border = 1.0f;
    const float inset = 2.0f; /* one pixel of outline, then one pixel of gap */
    if (!(size >= 6.0f)) return;
    DrawMenuRect(x, y, size, border, color);
    DrawMenuRect(x, y + size - border, size, border, color);
    DrawMenuRect(x, y + border, border, size - 2.0f * border, color);
    DrawMenuRect(x + size - border, y + border, border, size - 2.0f * border, color);
    if (checked)
        DrawMenuRect(x + inset, y + inset, size - 2.0f * inset,
                     size - 2.0f * inset, color);
}

/* 2. Native text/rectangle renderer (verified debug build) */
/* Entered from game call sites with the game's GP. The build uses no GP data. */
void NativeTextMakePacket97205(void *object, const u32 *relocator, int depth)
{
    C2DString_MakePacket(object, relocator, depth);
}
void NativeMenuTriangle97205(void *object, void *camera)
{
    //  typedef void (*Function)(void *, void *);
    //  ((Function)(u64)0x003010e0)(object, camera);
    C2DPoly_MakePacket(object, camera);
}
#include <stddef.h>
#include <stdint.h>

/* Addresses verified against SHA256
 * 00c9cee7f75b921fda1e848d04f64881b5a0b7841300591cac91371bb23d4f8a.
 * This is a packet-builder descriptor, NOT a constructed C++ object.
 * Never pass it to Load, SetString, Draw, Tick, or a destructor.
 */
typedef struct {
    u32 unused00[3];              /* 00: unused child list */
    u32 vtable;                    /* 0c */
    u32 unused10;
    u8 enabled, flags, unused16[2];
    u32 unused18;
    u32 font;                      /* 1c */
    u32 text;                      /* 20: borrowed, NUL-terminated suffix */
    u32 unused24;
    int32_t length;                     /* 28: length of this line */
    u32 unused2c[7];
    float scale;                       /* 48 */
    u32 unused4c;
    float rgba[4];                     /* 50 */
    u32 unused60[4];
    u8 unused70, centered, unused72[2];
    int32_t first;                      /* 74: first byte to render */
    int32_t width_out;                  /* 78: written by MakePacket */
    u32 unused7c;
    int32_t x16, y16;                   /* 80,84: signed pixel * 16 */
} NativeTextPacket;

_Static_assert(sizeof(void *) == 4, "Compile for the 32-bit PS2 EE address space");
_Static_assert(sizeof(NativeTextPacket) == 0x88, "descriptor size");
_Static_assert(offsetof(NativeTextPacket, font) == 0x1c, "font offset");
_Static_assert(offsetof(NativeTextPacket, text) == 0x20, "text offset");
_Static_assert(offsetof(NativeTextPacket, length) == 0x28, "length offset");
_Static_assert(offsetof(NativeTextPacket, scale) == 0x48, "scale offset");
_Static_assert(offsetof(NativeTextPacket, rgba) == 0x50, "color offset");
_Static_assert(offsetof(NativeTextPacket, centered) == 0x71, "centering offset");
_Static_assert(offsetof(NativeTextPacket, first) == 0x74, "start offset");
_Static_assert(offsetof(NativeTextPacket, x16) == 0x80, "position offset");


static u32 read_u32(u32 p) { return *(volatile u32 *)(u64)p; }
static float read_float(u32 p) { return *(volatile float *)(u64)p; }

static float channel(float v, float limit)
{
    if (!(v > 0.0f)) return 0.0f; /* includes NaN */
    return v < limit ? v : limit;
}

void DrawString2D(const char *text, float x, float y, float scale, Vec4 color)
{
    NativeTextPacket packet __attribute__((aligned(16)));
    u32 relocator = g_vft_BatchRelocator;   //  0x00472aa0; /* same one-word adapter as native Draw */
    u32 hud, font, handle, texture;
    float font_scale, line_step;
    const char *line;

    if (!text || !*text || !(scale > 0.0f && scale <= 64.0f)) return;
    /* Protect float-to-int conversion; GS scissoring still belongs to the game.
     * Callers should use visible HUD coordinates, not these broad guard bounds. */
    if (!(x >= -1000000.0f && x <= 1000000.0f &&
          y >= -1000000.0f && y <= 1000000.0f)) return;
    hud = read_u32(gHud);          /* theHUD */
    if (!hud) return;
    font = read_u32(hud + 0x288);
    if (!font || !read_u32(font + 8) || !read_u32(font + 12)) return;
    handle = read_u32(font + 0x10);
    if (!handle) return;
    texture = read_u32(handle + 0x0c);
    if (!texture) return;
    font_scale = read_float(font + 0x2c);
    if (!(font_scale > 0.0f && font_scale <= 64.0f)) return;

    /* Explicit stores avoid a compiler-generated memset dependency at -nostdlib. */
    {
        volatile unsigned char *bytes = (volatile unsigned char *)&packet;
        unsigned int i;
        for (i = 0; i < sizeof(packet); ++i) bytes[i] = 0;
    }
    packet.vtable = g_vft_2DString_2; // symbols build: 0x00472cb0;
    packet.enabled = 1;
    packet.flags = 0x29;               /* constructor flags: on + textured */
    packet.font = font;
    packet.scale = scale;
    packet.rgba[0] = channel(color.x, 255.0f);
    packet.rgba[1] = channel(color.y, 255.0f);
    packet.rgba[2] = channel(color.z, 255.0f);
    packet.rgba[3] = channel(color.w, 128.0f);
    packet.x16 = (int32_t)(x * 16.0f);

    /* Wrapper policy, not the native C2DString newline implementation:
     * topy/boty are font +18/+1c. Native GetSpacing returns zero in this build. */
    line_step = ((float)(int32_t)read_u32(font + 0x1c) -
                 (float)(int32_t)read_u32(font + 0x18) + 1.0f) * scale * font_scale;
    if (!(line_step > 0.0f && line_step < 1000000.0f)) return;
    line = text;
    for (;;) {
        int32_t n = 0;
        const char *next;
        while (line[n] && line[n] != '\n' && line[n] != '\r') {
            if (n == 0x7ffffffe) return;
            ++n;
        }
        if (!(y >= -1000000.0f && y <= 1000000.0f)) return;
        packet.text = (u32)(u64)line;
        packet.length = n;
        packet.y16 = (int32_t)(y * 16.0f);
        packet.width_out = 0;
        if (n) NativeTextMakePacket97205(&packet, &relocator, 0);
        next = line + n;
        if (!*next) break;
        if (*next == '\r' && next[1] == '\n') ++next;
        line = next + 1;
        y += line_step;
    }
}
#include <stdint.h>
#include <stddef.h>

/* Only fields touched by C2DPoly::MakePacket are initialized. */
typedef struct {
    u8 base[0x30];
    Vec4 rgba[3];                     /* +30 */
    Vec4 stq[3];                      /* +60 */
    Vec4 xyz[3];                      /* +90 */
    u32 texture;                 /* +c0 */
} NativeTriangle;
_Static_assert(offsetof(NativeTriangle, rgba) == 0x30, "color offset");
_Static_assert(offsetof(NativeTriangle, stq) == 0x60, "uv offset");
_Static_assert(offsetof(NativeTriangle, xyz) == 0x90, "position offset");
_Static_assert(offsetof(NativeTriangle, texture) == 0xc0, "texture offset");

void DrawMenuRect(float x, float y, float w, float h, Vec4 color)
{
    NativeTriangle t __attribute__((aligned(16)));
    unsigned i;
    volatile u8 *bytes = (volatile u8 *)&t;
    if (!(w > 0 && h > 0)) return;
    for (i = 0; i < sizeof(t); ++i) bytes[i] = 0;
    t.base[0x15] = 0x24;              /* enabled, alpha; untextured */
    for (i = 0; i < 3; ++i) {
        t.rgba[i] = color;
        t.stq[i] = (Vec4){0, 0, 1, 0};
    }
    /* Same 640x448 HUD anchor as text. C2DPoly rounds and shifts by 4. */
    x += 1728.0f;
    y += 1824.0f;
    t.xyz[0] = (Vec4){x, y, 0, 1};
    t.xyz[1] = (Vec4){x + w, y, 0, 1};
    t.xyz[2] = (Vec4){x, y + h, 0, 1};
    NativeMenuTriangle97205(&t, 0);
    t.xyz[0] = (Vec4){x + w, y, 0, 1};
    t.xyz[1] = (Vec4){x + w, y + h, 0, 1};
    NativeMenuTriangle97205(&t, 0);
}

void MenuFrame97205(void)
{
    u32 hud = *(volatile u32 *)gHud;
    u32 font;
    MenuEnsureInitialized();
    if (!hud) return;
    font = *(volatile u32 *)(u64)(hud + 0x288);
    if (!font || !*(volatile u32 *)(u64)(font + 0x10)) return;
    MenuDraw();
}

/* Runs after CInput::Tick and before the world's pause/scheduler decision. */
void MenuInput97205(void)
{
    u32 hud = *(volatile u32 *)gHud;
    u32 pad = *(volatile u32 *)g_CInput_pads; // debug 0x00474e30;
    u32 app = *(volatile u32 *)gAppCamera; // debug 0x00475908;
    u32 body = app ? *(volatile u32 *)(u64)(app + 0x3c) : 0;
    u32 controller = body ? *(volatile u32 *)(u64)(body + 0xc0) : 0;
    volatile u8 *state = controller ? (volatile u8 *)(u64)(controller + 0x221) : 0;
    unsigned held = 0, i, was_open;
    MenuEnsureInitialized();
    if (!hud || !pad) {
        g_menu.open = 0;
        MenuPauseState(controller, state, 0);
        g_menu.previous = g_menu.release_mask = 0;
        return;
    }
    {
        for (i = 0; i < 16; ++i) {
            u8 key = *(volatile u8 *)(u64)(pad + 0x13 + i);
            /* KEY_FALLING=1 (new press), KEY_DOWN=2, KEY_RISING=3 (release). */
            if (key == 1 || key == 2) held |= 1u << i;
        }
    }
    /* Let the native pause UI retain ownership if it is already active. */
    if (*(volatile u8 *)(u64)(hud + 0x14038)) {
        MenuPauseState(controller, state, 1);
        g_menu.previous = held;
        g_menu.release_mask = 0;
        return;
    }
    was_open = g_menu.open;
    MenuUpdate(held);
    MenuPauseState(controller, state, 0);
    if (was_open || g_menu.open) g_menu.release_mask = held;
    else g_menu.release_mask &= held;
    if (was_open || g_menu.open || g_menu.release_mask) {
        MenuSuppressPad((volatile u8 *)(u64)pad);
    }
}

/* 3. Input, persistent state and native pause */

void MenuSuppressPad(volatile u8 *pad)
{
    unsigned i;
    /* Consume processed controls after the menu's snapshot. Do not clear
     * cached m_Data at +A8: CPad::Tick does not poll hardware every tick.
     * Its next UpdateButton pass needs that cached held-button mask. */
    for (i = 0; i < 16; ++i) {
        pad[0x13 + i] = 0;
        pad[0x23 + i] = 0;
        pad[0x33 + i] = 0;
        *(volatile float *)(pad + 0x44 + i * 4) = -3.0f;
    }
    for (i = 0; i < 4; ++i) {
        *(volatile float *)(pad + 0xd4 + i * 4) = 0.0f;
        *(volatile float *)(pad + 0xe8 + i * 4) = 0.0f;
    }
}

/* Never exported to PNACH. Initialized by running code, not repeated writes. */
MenuState g_menu __attribute__((section(".menu_state"))) = {0};
#define MENU_STATE_MAGIC 0x4e485536u

void MenuEnsureInitialized(void)
{
    if (g_menu.magic == MENU_STATE_MAGIC) return;
    g_menu.open = 0;
    unsigned i,j;
    g_menu.page=0;
    for (j=0;j<2;++j) {
        g_menu.ui[j].focus=0; g_menu.ui[j].count=0; g_menu.ui[j].height=0;
        for(i=0;i<UI_MAX_ITEMS;++i) g_menu.ui[j].ids[i]=0;
    }
    g_menu.watermark = 1;
    g_menu.scale_index = 1;
    g_menu.accent_index = 0;
    g_menu.previous = 0;
    g_menu.release_mask = 0;
    g_menu.pause_controller = 0;
    g_menu.saved_menu_state = 0;
    g_menu.magic = MENU_STATE_MAGIC; /* publish only after all fields initialized */
}

/* Controller identity prevents restoring state through a stale character pointer. */
void MenuPauseState(u32 controller, volatile u8 *state, int native_ui)
{
    if (g_menu.pause_controller != controller) g_menu.pause_controller = 0;
    if (!controller || !state) { g_menu.open = 0; return; }
    if (native_ui) { g_menu.open = 0; g_menu.pause_controller = 0; return; }
    if (g_menu.open && !g_menu.pause_controller) {
        /* Do not take ownership of another native menu. */
        if (*state != 0) { g_menu.open = 0; return; }
        g_menu.saved_menu_state = *state;
        g_menu.pause_controller = controller;
        *state = 3; /* CSealCtrl::statePauseTestMenu, user-verified in game */
    } else if (g_menu.pause_controller && !g_menu.open) {
        if (*state == 3) *state = (u8)g_menu.saved_menu_state;
        g_menu.pause_controller = 0;
    } else if (g_menu.pause_controller && *state != 3) {
        g_menu.open = 0; /* Game changed state: yield without overwriting it. */
        g_menu.pause_controller = 0;
    }
}
void MenuUpdate(unsigned held)
{
    unsigned pressed, actions=0;
    MenuEnsureInitialized();
    pressed=held & ~g_menu.previous;
    g_menu.previous=held;
    if (pressed & MENU_SELECT) {
        g_menu.open=!g_menu.open;
        if (g_menu.open) MenuBuild(UI_INPUT,0);
        return;
    }
    if (!g_menu.open) return;
    if (pressed & MENU_CIRCLE) { g_menu.open=0; return; }
    if (pressed & MENU_UP) actions|=UI_UP;
    if (pressed & MENU_DOWN) actions|=UI_DOWN;
    if (pressed & MENU_LEFT) actions|=UI_LEFT;
    if (pressed & MENU_RIGHT) actions|=UI_RIGHT;
    if (pressed & MENU_CROSS) actions|=UI_ACTIVATE;
    MenuBuild(UI_INPUT,actions);
    /* Re-measure after actions that changed scale; no input is replayed. */
    if (actions) MenuBuild(UI_INPUT,0);
}
#include <stdint.h>
typedef void (*NativeVoid)(void);
/* Replace existing calls, leaving native function entries untouched.
 * The compiler handles ordinary C calls and returns; no trampoline is needed. */
__attribute__((section(".hook_menu"), noinline, used))
void MenuHook97205(void)
{
    MenuFrame97205();
    zVid_ZTestOn(); // ((NativeVoid)(u64)0x0031ED90)(); /* original zVid_ZTestOn */
}

__attribute__((section(".hook_input"), noinline, used))
void MenuInputHook97205(void)
{
    MenuInput97205();
    recoTick(); // ((NativeVoid)(u64)0x00336440)(); /* original recoTick */
}

/* Original CHUD::PauseGame entry stays untouched and callable. */
__attribute__((section(".hook_pause"), noinline, used))
int MenuPauseHook97205(void *hud)
{
    typedef int (*NativePause)(void *);
    MenuEnsureInitialized();
    if (g_menu.open) return 1;
    return CHUD_PauseGame(hud); // return ((NativePause)(u64)0x003ba040)(hud);
}
