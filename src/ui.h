#ifndef UI_H
#define UI_H

#include "canvas.h"
#include "colors.h"
#include "input.h"

// Posizioni UI
#define UI_TOOLBAR_Y      0
#define UI_TOOLBAR_HEIGHT  40
#define UI_PALETTE_Y       (SCREEN_H - 35)
#define UI_PALETTE_HEIGHT  35

typedef struct {
    int show_toolbar;
    int show_palette;
    int show_help;

    // Status message
    char status_msg[64];
    int status_timer;
} UIState;

void ui_init(UIState *ui);
void ui_set_status(UIState *ui, const char *msg);
void ui_update(UIState *ui);
void ui_render_toolbar(const UIState *ui, const Canvas *canvas, const ColorPalette *palette);
void ui_render_palette(const UIState *ui, const ColorPalette *palette);
void ui_render_cursor(int x, int y, int brush_size, unsigned int color);
void ui_render_help(void);
void ui_render_shape_preview(const Canvas *canvas, int x, int y);

// Controlla se il touch è nell'area della palette, ritorna indice colore o -1
int  ui_palette_hit_test(const UIState *ui, int x, int y);

// Controlla se il touch è nella toolbar
int  ui_toolbar_hit_test(const UIState *ui, int x, int y);

#endif
