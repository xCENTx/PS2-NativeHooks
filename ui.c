#include "ui.h"
#include <stddef.h>

// ------------------------------------------------------------
// Layout and widgets
// ------------------------------------------------------------
/* Native drawing primitives remain independent of window layout. */
void DrawMenuRect(f32 x, f32 y, f32 w, f32 h, Vec4 color);
void DrawCheckbox(f32 x, f32 y, f32 size, int checked, Vec4 color);
void DrawText(const char* text, f32 x, f32 y, f32 scale, Vec4 color)
{
    DrawString2D(text, x, y, scale, color);
}

void UI_BeginWindow(UIContext* ui, UIState* state, const UILayout* layout, const UIStyle* style, u32 pass, u32 pressed)
{
    u32 i;
    ui->state = state;
    ui->layout = *layout;
    ui->style = *style;
    ui->pass = pass;
    ui->pressed = pressed;
    ui->count = 0;
    ui->focusSeen = 0;
    ui->first = 0;
    ui->overflow = 0;
    ui->content =
        (UIRect){layout->x + layout->padding, layout->y + layout->padding, layout->width - 2 * layout->padding,
                 state->height > 2 * layout->padding ? state->height - 2 * layout->padding : 0};
    ui->cursorY = ui->content.y;
    ui->lastRow = (UIRect){0, 0, 0, 0};
    if (pass == UI_INPUT && state->count)
    {
        for (i = 0; i < state->count && state->ids[i] != state->focus; ++i)
        {
        }
        if (i == state->count)
        {
            i = 0;
        }
        if (pressed & UI_UP)
        {
            i = (i + state->count - 1) % state->count;
        }
        else if (pressed & UI_DOWN)
        {
            i = (i + 1) % state->count;
        }
        state->focus = state->ids[i];
    }
    if (pass == UI_DRAW)
    {
        DrawMenuRect(layout->x, layout->y, layout->width, state->height, style->background);
        DrawMenuRect(layout->x, layout->y, layout->width, style->accentHeight, style->accent);
    }
}

void UI_EndWindow(UIContext* ui)
{
    ui->content.height = ui->cursorY - ui->content.y;
    if (ui->pass == UI_INPUT)
    {
        ui->state->count = ui->count;
        if (!ui->focusSeen)
        {
            ui->state->focus = ui->first;
        }
        ui->state->height = ui->content.height + 2 * ui->layout.padding;
    }
}

void UI_Spacing(UIContext* ui, f32 pixels)
{
    if (pixels > 0)
    {
        ui->cursorY += pixels;
    }
}

void UI_Text(UIContext* ui, const char* text, f32 scale, Vec4 color)
{
    f32 height;
    if (!(scale > 0))
    {
        return;
    }
    height = ui->style.textHeight * scale;
    if (ui->pass == UI_DRAW)
    {
        DrawText(text, ui->content.x, ui->cursorY + height * 0.5f + ui->style.baselineOffset * scale, scale, color);
    }
    ui->cursorY += height;
}

static int UI_BeginRow(UIContext* ui, u32 id, const char* label, f32 scale)
{
    f32 height = ui->style.textHeight * scale + 2 * ui->style.rowPadding;
    int focused;
    if (height < ui->style.rowHeight)
    {
        height = ui->style.rowHeight;
    }
    ui->lastRow = (UIRect){ui->content.x, ui->cursorY, ui->content.width, height};
    ui->cursorY += height + ui->style.rowGap;
    if (!id || ui->count >= UI_MAX_ITEMS)
    {
        ui->overflow = 1;
        return 0;
    }
    if (!ui->first)
    {
        ui->first = id;
    }
    if (!ui->state->focus && ui->pass == UI_INPUT)
    {
        ui->state->focus = id;
    }
    focused = ui->state->focus == id;
    if (focused)
    {
        ui->focusSeen = 1;
    }
    if (ui->pass == UI_INPUT)
    {
        ui->state->ids[ui->count] = id;
    }
    ++ui->count;
    if (ui->pass == UI_DRAW)
    {
        if (focused)
        {
            DrawMenuRect(ui->lastRow.x, ui->lastRow.y, ui->lastRow.width, height, ui->style.highlight);
            DrawMenuRect(ui->lastRow.x, ui->lastRow.y, 3, height, ui->style.accent);
        }
        DrawText(label, ui->lastRow.x + ui->style.rowPadding,
                 ui->lastRow.y + height * 0.5f + ui->style.baselineOffset * scale, scale, ui->style.text);
    }
    return focused && ui->pass == UI_INPUT;
}

static f32 UI_GetValueX(const UIContext* ui)
{
    return ui->lastRow.x + ui->lastRow.width * ui->style.valueFraction;
}

int UI_Checkbox(UIContext* ui, u32 id, const char* label, bool* value, f32 scale)
{
    int changed = UI_BeginRow(ui, id, label, scale) && (ui->pressed & (UI_LEFT | UI_RIGHT | UI_ACTIVATE));
    if (changed)
    {
        *value = !*value;
    }
    if (ui->pass == UI_DRAW)
    {
        f32 size = 16 * scale;
        DrawCheckbox(UI_GetValueX(ui), ui->lastRow.y + (ui->lastRow.height - size) * 0.5f, size, *value,
                     ui->state->focus == id ? ui->style.accent : ui->style.muted);
    }
    return changed;
}

int UI_Combo(UIContext* ui, u32 id, const char* label, u32* value, const char* const* names, u32 count, f32 scale)
{
    int active = UI_BeginRow(ui, id, label, scale), changed = 0;
    u32 index = count ? *value % count : 0;
    if (count && active && (ui->pressed & (UI_LEFT | UI_RIGHT | UI_ACTIVATE)))
    {
        index = (index + ((ui->pressed & UI_LEFT) ? count - 1 : 1)) % count;
        *value = index;
        changed = 1;
    }
    if (count && ui->pass == UI_DRAW)
    {
        DrawText(names[index], UI_GetValueX(ui),
                 ui->lastRow.y + ui->lastRow.height * 0.5f + ui->style.baselineOffset * scale, scale,
                 ui->state->focus == id ? ui->style.accent : ui->style.muted);
    }
    return changed;
}

int UI_Button(UIContext* ui, u32 id, const char* label, f32 scale)
{
    return UI_BeginRow(ui, id, label, scale) && (ui->pressed & UI_ACTIVATE) != 0;
}

/* Coordinates are the square's top-left in native HUD pixels.
 * The unpainted gap lets the existing menu/highlight show through. */
void DrawCheckbox(f32 x, f32 y, f32 size, int checked, Vec4 color)
{
    const f32 border = 1.0f;
    const f32 inset = 2.0f; /* one pixel of outline, then one pixel of gap */
    if (!(size >= 6.0f))
    {
        return;
    }
    DrawMenuRect(x, y, size, border, color);
    DrawMenuRect(x, y + size - border, size, border, color);
    DrawMenuRect(x, y + border, border, size - 2.0f * border, color);
    DrawMenuRect(x + size - border, y + border, border, size - 2.0f * border, color);
    if (checked)
    {
        DrawMenuRect(x + inset, y + inset, size - 2.0f * inset, size - 2.0f * inset, color);
    }
}

// ------------------------------------------------------------
// Native rendering
// ------------------------------------------------------------
/* Entered from game call sites with the game's GP. The build uses no GP data. */
void NativeTextMakePacket97205(void* object, const u32* relocator, int depth)
{
    C2DString_MakePacket(object, relocator, depth);
}

void NativeMenuTriangle97205(void* object, void* camera)
{
    C2DPoly_MakePacket(object, camera);
}

// Caller-owned packet descriptor for MakePacket only.
// Do not pass it to native string loading or destruction methods.
typedef struct
{
    u8 unknown00[0x0C];
    u32 vtable; /* 0x0C */
    u32 unused10;
    u8 enabled, flags, unused16[2];
    u32 unused18;
    u32 font; /* 0x1C */
    u32 text; /* 0x20 */
    u32 unknown24;
    s32 length; /* 0x28 */
    u8 unknown2C[0x20];
    f32 scale; /* 0x4C */
    u32 unknown50;
    f32 rgba[4]; /* 0x54 */
    u8 unknown64[0x11];
    u8 centered;      /* 0x75 */
    u8 right_aligned; /* 0x76 */
    u8 unknown77;
    s32 first;     /* 0x78 */
    s32 width_out; /* 0x7C */
    u32 unknown80;
    s32 x16; /* 0x84 */
    s32 y16; /* 0x88 */
} NativeTextPacket;
_Static_assert(sizeof(NativeTextPacket) == 0x8C, "descriptor size");
_Static_assert(offsetof(NativeTextPacket, font) == 0x1c, "font offset");
_Static_assert(offsetof(NativeTextPacket, text) == 0x20, "text offset");
_Static_assert(offsetof(NativeTextPacket, length) == 0x28, "length offset");
_Static_assert(offsetof(NativeTextPacket, scale) == 0x4C, "scale offset");
_Static_assert(offsetof(NativeTextPacket, rgba) == 0x54, "color offset");
_Static_assert(offsetof(NativeTextPacket, centered) == 0x75, "centering offset");
_Static_assert(offsetof(NativeTextPacket, first) == 0x78, "start offset");
_Static_assert(offsetof(NativeTextPacket, x16) == 0x84, "position offset");

static u32 ReadU32(u32 p)
{
    return *(volatile u32*)(u64)p;
}

static f32 ReadFloat(u32 p)
{
    return *(volatile f32*)(u64)p;
}

static f32 ClampColorChannel(f32 v, f32 limit)
{
    if (!(v > 0.0f))
    {
        return 0.0f; /* includes NaN */
    }
    return v < limit ? v : limit;
}

void DrawString2D(const char* text, f32 x, f32 y, f32 scale, Vec4 color)
{
    NativeTextPacket packet __attribute__((aligned(16)));
    u32 relocator = g_vft_BatchRelocator; // Same one-word adapter as native Draw.
    u32 hud, font, handle, texture;
    f32 font_scale, line_step;
    const char* line;

    if (!text || !*text || !(scale > 0.0f && scale <= 64.0f))
    {
        return;
    }
    /* Protect f32-to-int conversion; GS scissoring still belongs to the game.
     * Callers should use visible HUD coordinates, not these broad guard bounds. */
    if (!(x >= -1000000.0f && x <= 1000000.0f && y >= -1000000.0f && y <= 1000000.0f))
    {
        return;
    }
    hud = ReadU32(gHud); /* theHUD */
    if (!hud)
    {
        return;
    }
    font = ReadU32(hud + 0x288);
    if (!font || !ReadU32(font + 8) || !ReadU32(font + 12))
    {
        return;
    }
    handle = ReadU32(font + 0x10);
    if (!handle)
    {
        return;
    }
    texture = ReadU32(handle + 0x0c);
    if (!texture)
    {
        return;
    }
    font_scale = ReadFloat(font + 0x2c);
    if (!(font_scale > 0.0f && font_scale <= 64.0f))
    {
        return;
    }

    /* Explicit stores avoid a compiler-generated memset dependency at -nostdlib. */
    {
        volatile u8* bytes = (volatile u8*)&packet;
        u32 i;
        for (i = 0; i < sizeof(packet); ++i)
        {
            bytes[i] = 0;
        }
    }
    packet.vtable = g_vft_2DString_2;
    packet.enabled = 1;
    packet.flags = 0x29; /* constructor flags: on + textured */
    packet.font = font;
    packet.scale = scale;
    packet.rgba[0] = ClampColorChannel(color.x, 255.0f);
    packet.rgba[1] = ClampColorChannel(color.y, 255.0f);
    packet.rgba[2] = ClampColorChannel(color.z, 255.0f);
    packet.rgba[3] = ClampColorChannel(color.w, 128.0f);
    packet.x16 = (s32)(x * 16.0f);

    /* Wrapper policy, not the native C2DString newline implementation:
     * topy/boty are font +18/+1c. Native GetSpacing returns zero in this build. */
    line_step = ((f32)(s32)ReadU32(font + 0x1c) - (f32)(s32)ReadU32(font + 0x18) + 1.0f) * scale * font_scale;
    if (!(line_step > 0.0f && line_step < 1000000.0f))
    {
        return;
    }
    line = text;
    for (;;)
    {
        s32 n = 0;
        const char* next;
        while (line[n] && line[n] != '\n' && line[n] != '\r')
        {
            if (n == 0x7ffffffe)
            {
                return;
            }
            ++n;
        }
        if (!(y >= -1000000.0f && y <= 1000000.0f))
        {
            return;
        }
        packet.text = (u32)(u64)line;
        packet.length = n;
        packet.y16 = (s32)(y * 16.0f);
        packet.width_out = 0;
        if (n)
        {
            NativeTextMakePacket97205(&packet, &relocator, 0);
        }
        next = line + n;
        if (!*next)
        {
            break;
        }
        if (*next == '\r' && next[1] == '\n')
        {
            ++next;
        }
        line = next + 1;
        y += line_step;
    }
}

/* Only fields touched by C2DPoly::MakePacket are initialized. */
typedef struct
{
    u8 base[0x30];
    Vec4 rgba[3]; /* +30 */
    Vec4 stq[3];  /* +60 */
    Vec4 xyz[3];  /* +90 */
    u32 texture;  /* +c0 */
} NativeTriangle;
_Static_assert(offsetof(NativeTriangle, rgba) == 0x30, "color offset");
_Static_assert(offsetof(NativeTriangle, stq) == 0x60, "uv offset");
_Static_assert(offsetof(NativeTriangle, xyz) == 0x90, "position offset");
_Static_assert(offsetof(NativeTriangle, texture) == 0xc0, "texture offset");

void DrawMenuRect(f32 x, f32 y, f32 w, f32 h, Vec4 color)
{
    NativeTriangle t __attribute__((aligned(16)));
    u32 i;
    volatile u8* bytes = (volatile u8*)&t;
    if (!(w > 0 && h > 0))
    {
        return;
    }
    for (i = 0; i < sizeof(t); ++i)
    {
        bytes[i] = 0;
    }
    t.base[0x15] = 0x24; /* enabled, alpha; untextured */
    for (i = 0; i < 3; ++i)
    {
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

/* Private MenuDebug helpers and state (C uses prefixes instead of namespaces).
 * Collection stays active; MENU_DEBUG_OVERLAY controls on-screen rendering.
 * Persistent diagnostic values must use patch=0. */
static struct
{
    u32 ticks, pad, hud, raw, select_state, held, pressed, toggles, after;
} g_menuDebug __attribute__((section(".menu_state"))) = {0};

#if MENU_DEBUG_OVERLAY
static void MenuDebug_DrawLine(const char* label, u32 value, f32 y)
{
    char line[48];
    const char* hex = "0123456789ABCDEF";
    u32 n = 0, i;
    while (*label && n < 36)
    {
        line[n++] = *label++;
    }
    for (i = 0; i < 8; ++i)
    {
        line[n++] = hex[(value >> (28 - i * 4)) & 15];
    }
    line[n] = 0;
    DrawText(line, 32, y, 0.6f, (Vec4){255, 255, 0, 128});
}

static void MenuDebug_Draw(void)
{
    MenuDebug_DrawLine("INPUT TICKS ", g_menuDebug.ticks, 42);
    MenuDebug_DrawLine("PAD ", g_menuDebug.pad, 62);
    MenuDebug_DrawLine("HUD ", g_menuDebug.hud, 82);
    MenuDebug_DrawLine("RAW BUTTONS ", g_menuDebug.raw, 102);
    MenuDebug_DrawLine("SELECT STATE ", g_menuDebug.select_state, 122);
    MenuDebug_DrawLine("HELD ", g_menuDebug.held, 142);
    MenuDebug_DrawLine("PRESS EDGES ", g_menuDebug.pressed, 162);
    MenuDebug_DrawLine("COMBO EDGES ", g_menuDebug.toggles, 182);
    MenuDebug_DrawLine("OPEN AFTER INPUT ", g_menuDebug.after, 202);
    MenuDebug_DrawLine("OPEN AT DRAW ", g_menu.open, 222);
}

#endif /* MENU_DEBUG_OVERLAY */

void MenuFrame97205(void)
{
    u32 hud = *(volatile u32*)gHud;
    u32 font;
    MenuEnsureInitialized();
    if (!hud)
    {
        return;
    }
    font = *(volatile u32*)(u64)(hud + 0x288);
    if (!font || !*(volatile u32*)(u64)(font + 0x10))
    {
        return;
    }
    MenuDraw();
#if MENU_DEBUG_OVERLAY
    MenuDebug_Draw();
#endif
}

static u32 ReadHeldButtons(void)
{
    u32 pad = ((volatile u32*)g_CInput_pads)[4];
    u32 held = 0, i;
    if (!pad)
    {
        return 0;
    }
    for (i = 0; i < 16; ++i)
    {
        u8 state = *(volatile u8*)(u64)(pad + 0x13 + i);
        if (state == 1 || state == 2)
        {
            held |= 1u << i;
        }
    }
    return held;
}

bool GetButtonState(u32 buttons)
{
    if (!buttons || (buttons & ~0xFFFFu))
    {
        return false;
    }
    return (ReadHeldButtons() & buttons) == buttons;
}

/* Runs after CInput::Tick and before the world's pause/scheduler decision. */
void MenuInput97205(void)
{
    u32 hud = *(volatile u32*)gHud;
    u32 pad = ((volatile u32*)g_CInput_pads)[4];
    u32 held = 0;
    u32 was_open;

    MenuEnsureInitialized();

    ++g_menuDebug.ticks;
    g_menuDebug.pad = pad;
    g_menuDebug.hud = hud;
    g_menuDebug.raw = pad ? *(volatile u32*)(u64)(pad + 0xE8) : 0;
    g_menuDebug.select_state = pad ? *(volatile u8*)(u64)(pad + 0x13 + 1) : 0;
    g_menuDebug.held = g_menuDebug.pressed = g_menuDebug.after = 0;

    if (!hud || !pad)
    {
        g_menu.open = 0;
        g_menu.previous = 0;
        g_menu.release_mask = 0;
        return;
    }

    held = ReadHeldButtons();

    g_menuDebug.held = held;
    g_menuDebug.pressed = held & ~g_menu.previous;
    if ((held & MENU_STICK_CLICKS) == MENU_STICK_CLICKS && (g_menu.previous & MENU_STICK_CLICKS) != MENU_STICK_CLICKS)
    {
        ++g_menuDebug.toggles;
    }
    was_open = g_menu.open;
    MenuUpdate(held);
    g_menuDebug.after = g_menu.open;

    /* Consume held buttons until released after closing the menu. */
    if (was_open || g_menu.open)
    {
        g_menu.release_mask = held;
    }
    else
    {
        g_menu.release_mask &= held;
    }

    if (was_open || g_menu.open || g_menu.release_mask)
    {
        MenuSuppressPad((volatile u8*)(u64)pad);
    }
}

// ------------------------------------------------------------
// Menu state and input
// ------------------------------------------------------------

void MenuSuppressPad(volatile u8* pad)
{
    u32 i;

    for (i = 0; i < 16; ++i)
    {
        pad[0x13 + i] = 0;

        /* Retail per-button input value. */
        *(volatile f32*)(pad + 0x24 + i * 4) = 0.0f;
    }

    for (i = 0; i < 4; ++i)
    {
        *(volatile f32*)(pad + 0x114 + i * 4) = 0.0f;
        *(volatile f32*)(pad + 0x128 + i * 4) = 0.0f;
    }
}

/* Never exported to PNACH. Initialized by running code, not repeated writes. */
MenuState g_menu __attribute__((section(".menu_state"))) = {0};
#define MENU_STATE_MAGIC 0x4e485536u

void MenuEnsureInitialized(void)
{
    if (g_menu.magic == MENU_STATE_MAGIC)
    {
        return;
    }
    g_menu.open = 0;
    u32 i, j;
    g_menu.page = 0;
    for (j = 0; j < 2; ++j)
    {
        g_menu.ui[j].focus = 0;
        g_menu.ui[j].count = 0;
        g_menu.ui[j].height = 0;
        for (i = 0; i < UI_MAX_ITEMS; ++i)
        {
            g_menu.ui[j].ids[i] = 0;
        }
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
void MenuPauseState(u32 controller, volatile u8* state, int native_ui)
{
    if (g_menu.pause_controller != controller)
    {
        g_menu.pause_controller = 0;
    }
    if (!controller || !state)
    {
        g_menu.open = 0;
        return;
    }
    if (native_ui)
    {
        g_menu.open = 0;
        g_menu.pause_controller = 0;
        return;
    }
    if (g_menu.open && !g_menu.pause_controller)
    {
        /* Do not take ownership of another native menu. */
        if (*state != 0)
        {
            g_menu.open = 0;
            return;
        }
        g_menu.saved_menu_state = *state;
        g_menu.pause_controller = controller;
        *state = 3; /* CSealCtrl::statePauseTestMenu, user-verified in game */
    }
    else if (g_menu.pause_controller && !g_menu.open)
    {
        if (*state == 3)
        {
            *state = (u8)g_menu.saved_menu_state;
        }
        g_menu.pause_controller = 0;
    }
    else if (g_menu.pause_controller && *state != 3)
    {
        g_menu.open = 0; /* Game changed state: yield without overwriting it. */
        g_menu.pause_controller = 0;
    }
}

void MenuUpdate(u32 held)
{
    u32 pressed, previous, actions = 0;
    MenuEnsureInitialized();
    previous = g_menu.previous;
    pressed = held & ~previous;
    g_menu.previous = held;
    if ((held & MENU_STICK_CLICKS) == MENU_STICK_CLICKS && (previous & MENU_STICK_CLICKS) != MENU_STICK_CLICKS)
    {
        g_menu.open = !g_menu.open;
        if (g_menu.open)
        {
            MenuBuild(UI_INPUT, 0);
        }
        return;
    }
    if (!g_menu.open)
    {
        return;
    }
    if (pressed & MENU_CIRCLE)
    {
        g_menu.open = 0;
        return;
    }
    if (pressed & MENU_UP)
    {
        actions |= UI_UP;
    }
    if (pressed & MENU_DOWN)
    {
        actions |= UI_DOWN;
    }
    if (pressed & MENU_LEFT)
    {
        actions |= UI_LEFT;
    }
    if (pressed & MENU_RIGHT)
    {
        actions |= UI_RIGHT;
    }
    if (pressed & MENU_CROSS)
    {
        actions |= UI_ACTIVATE;
    }
    MenuBuild(UI_INPUT, actions);
    /* Re-measure after actions that changed scale; no input is replayed. */
    if (actions)
    {
        MenuBuild(UI_INPUT, 0);
    }
}
typedef void (*NativeVoid)(void);
/* Replace existing calls, leaving native function entries untouched.
 * The compiler handles ordinary C calls and returns; no trampoline is needed. */
__attribute__((section(".hook_menu"), noinline, used)) void MenuHook97205(void)
{
    MenuFrame97205();
    zVid_ZTestOn();
}

__attribute__((section(".hook_input"), noinline, used)) void MenuInputHook97205(void)
{
    MenuInput97205();
    recoTick();
}

/* Original CHUD::PauseGame entry stays untouched and callable. */
__attribute__((section(".hook_pause"), noinline, used)) int MenuPauseHook97205(void* hud)
{
    typedef int (*NativePause)(void*);
    MenuEnsureInitialized();
    if (g_menu.open)
    {
        return 1;
    }
    return CHUD_PauseGame(hud);
}
