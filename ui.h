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

#include <stdint.h>
/* Set to 1 to show the input diagnostic overlay again. */
#ifndef MENU_DEBUG_OVERLAY
#define MENU_DEBUG_OVERLAY 0
#endif

#define UI_MAX_ITEMS 32
/* Stable, nonzero IDs belong to the caller, not a row's position. */
enum { UI_UP=1, UI_DOWN=2, UI_LEFT=4, UI_RIGHT=8, UI_ACTIVATE=16 };
enum { UI_INPUT, UI_DRAW };
typedef struct { float x, y, width, height; } UIRect;
typedef struct { float x, y, width, padding; } UILayout;
typedef struct {
    float rowHeight, rowGap, rowPadding, valueFraction;
    float textHeight, baselineOffset, accentHeight;
    Vec4 background, highlight, text, muted, accent;
} UIStyle;
typedef struct {
    u32 focus, ids[UI_MAX_ITEMS];
    unsigned count;
    float height;
} UIState;
typedef struct {
    UIState *state;
    UILayout layout;
    UIStyle style;
    UIRect content, lastRow;
    float cursorY;
    unsigned pass, pressed, count, focusSeen, overflow;
    u32 first;
} UIContext;
void DrawText(const char *text, float x, float y, float scale, Vec4 color);
void UI_BeginWindow(UIContext *ui, UIState *state, const UILayout *layout,
                    const UIStyle *style, unsigned pass, unsigned pressed);
void UI_EndWindow(UIContext *ui);
void UI_Text(UIContext *ui, const char *text, float scale, Vec4 color);
void UI_Spacing(UIContext *ui, float pixels);
int UI_Checkbox(UIContext *ui, u32 id, const char *label, bool *value, float scale);
int UI_Combo(UIContext *ui, u32 id, const char *label, unsigned *value,
             const char *const *names, unsigned count, float scale);
int UI_Button(UIContext *ui, u32 id, const char *label, float scale);

//  /* Exact PAD_BUTTON values from SCUS_972.05 .debug. */
enum {
    MENU_SELECT = 1u << 1, 
    MENU_RIGHT = 1u << 2,
    MENU_LEFT = 1u << 3, 
    MENU_UP = 1u << 4, 
    MENU_DOWN = 1u << 5,
    MENU_CIRCLE = 1u << 7, 
    MENU_CROSS = 1u << 9,
    MENU_STICK_CLICKS = (1u << 14) | (1u << 15)
};
typedef struct {
    unsigned open, scale_index, accent_index, previous, page;
    bool watermark;
    UIState ui[2];
    unsigned release_mask, magic;
    u32 pause_controller;
    unsigned saved_menu_state;
} MenuState;

extern MenuState g_menu;
/* True when ALL requested buttons are pressed/held (states 1 or 2).
 * Combine enum masks with |. Zero/invalid masks or no pad return false.
 * Call before MenuSuppressPad; this does not detect press edges. */
bool GetButtonState(unsigned buttons);
void MenuUpdate(unsigned held);
void MenuEnsureInitialized(void);
void MenuPauseState(u32 controller, volatile u8 *state, int native_ui);
void MenuDraw(void);
void MenuFrame97205(void);
void MenuInput97205(void);
void MenuSuppressPad(volatile u8 *pad);
void DrawMenuRect(float x, float y, float w, float h, Vec4 color);
/* Draw only: caller owns the boolean; size >= 6, top-left HUD coordinates. */
void DrawCheckbox(float x, float y, float size, int checked, Vec4 color);

void DrawString2D(const char *, float, float, float, Vec4);
void NativeTextMakePacket97205(void *, const u32 *, int);
void NativeMenuTriangle97205(void *, void *);
/* main.c owns these: feature values, declarations and watermark. */
void MenuBuild(unsigned pass, unsigned pressed);
void MenuDraw(void);
#endif
