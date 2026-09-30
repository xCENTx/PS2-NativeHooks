#ifndef NATIVEHOOKS_UI_H
#define NATIVEHOOKS_UI_H
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#ifdef UI_HOST_TEST
#include "ui_test_types.h"
#else
#include "games/SOCOM/structs.h"
#include "games/SOCOM/game.h"
#endif

/* Set to 1 to show the input diagnostic overlay again. */
#ifndef MENU_DEBUG_OVERLAY
#define MENU_DEBUG_OVERLAY 0
#endif

#define UI_MAX_ITEMS 32
/* Stable, nonzero IDs belong to the caller, not a row's position. */
enum
{
    UI_UP = 1,
    UI_DOWN = 2,
    UI_LEFT = 4,
    UI_RIGHT = 8,
    UI_ACTIVATE = 16
};
enum
{
    UI_INPUT,
    UI_DRAW
};
typedef struct
{
    f32 x, y, width, height;
} UIRect;
typedef struct
{
    f32 x, y, width, padding;
} UILayout;
typedef struct
{
    f32 rowHeight, rowGap, rowPadding, valueFraction;
    f32 textHeight, baselineOffset, accentHeight;
    Vec4 background, highlight, text, muted, accent;
} UIStyle;
typedef struct
{
    u32 focus, ids[UI_MAX_ITEMS];
    u32 count;
    f32 height;
} UIState;
typedef struct
{
    UIState* state;
    UILayout layout;
    UIStyle style;
    UIRect content, lastRow;
    f32 cursorY;
    u32 pass, pressed, count, focusSeen, overflow;
    u32 first;
} UIContext;
void DrawText(const char* text, f32 x, f32 y, f32 scale, Vec4 color);
void UI_BeginWindow(UIContext* ui, UIState* state, const UILayout* layout, const UIStyle* style, u32 pass, u32 pressed);
void UI_EndWindow(UIContext* ui);
void UI_Text(UIContext* ui, const char* text, f32 scale, Vec4 color);
void UI_Spacing(UIContext* ui, f32 pixels);
int UI_Checkbox(UIContext* ui, u32 id, const char* label, bool* value, f32 scale);
int UI_Combo(UIContext* ui, u32 id, const char* label, u32* value, const char* const* names, u32 count, f32 scale);
int UI_Button(UIContext* ui, u32 id, const char* label, f32 scale);

// Processed CPad button masks.
enum
{
    MENU_SELECT = 1u << 1,
    MENU_RIGHT = 1u << 2,
    MENU_LEFT = 1u << 3,
    MENU_UP = 1u << 4,
    MENU_DOWN = 1u << 5,
    MENU_CIRCLE = 1u << 7,
    MENU_CROSS = 1u << 9,
    MENU_STICK_CLICKS = (1u << 14) | (1u << 15)
};
typedef struct
{
    u32 open, scale_index, accent_index, previous, page;
    bool watermark;
    UIState ui[2];
    u32 release_mask, magic;
    u32 pause_controller;
    u32 saved_menu_state;
} MenuState;

extern MenuState g_menu;
/* True when ALL requested buttons are pressed/held (states 1 or 2).
 * Combine enum masks with |. Zero/invalid masks or no pad return false.
 * Call before MenuSuppressPad; this does not detect press edges. */
bool GetButtonState(u32 buttons);
void MenuUpdate(u32 held);
void MenuEnsureInitialized(void);
void MenuPauseState(u32 controller, volatile u8* state, int native_ui);
void MenuDraw(void);
void MenuFrame97205(void);
void MenuInput97205(void);
void MenuSuppressPad(volatile u8* pad);
void DrawMenuRect(f32 x, f32 y, f32 w, f32 h, Vec4 color);
/* Draw only: caller owns the boolean; size >= 6, top-left HUD coordinates. */
void DrawCheckbox(f32 x, f32 y, f32 size, int checked, Vec4 color);

void DrawString2D(const char*, f32, f32, f32, Vec4);
// Centers a short label horizontally around x; y retains the normal text anchor.
void DrawTextCentered(const char* text, f32 x, f32 y, f32 scale, Vec4 color);
void NativeTextMakePacket97205(void*, const u32*, int);
void NativeMenuTriangle97205(void*, void*);
/* main.c owns these: feature values, declarations and watermark. */
void MenuBuild(u32 pass, u32 pressed);
#endif
