#ifndef COLORS_H
#define COLORS_H

#include <vita2d.h>

// Colori RGBA predefiniti
#define COLOR_WHITE       RGBA8(255, 255, 255, 255)
#define COLOR_BLACK       RGBA8(0, 0, 0, 255)
#define COLOR_RED         RGBA8(255, 0, 0, 255)
#define COLOR_GREEN       RGBA8(0, 255, 0, 255)
#define COLOR_BLUE        RGBA8(0, 0, 255, 255)
#define COLOR_YELLOW      RGBA8(255, 255, 0, 255)
#define COLOR_CYAN        RGBA8(0, 255, 255, 255)
#define COLOR_MAGENTA     RGBA8(255, 0, 255, 255)
#define COLOR_ORANGE      RGBA8(255, 165, 0, 255)
#define COLOR_PURPLE      RGBA8(128, 0, 128, 255)
#define COLOR_PINK        RGBA8(255, 192, 203, 255)
#define COLOR_BROWN       RGBA8(139, 69, 19, 255)
#define COLOR_GRAY        RGBA8(128, 128, 128, 255)
#define COLOR_LIGHT_GRAY  RGBA8(200, 200, 200, 255)
#define COLOR_DARK_GRAY   RGBA8(64, 64, 64, 255)
#define COLOR_DARK_RED    RGBA8(139, 0, 0, 255)
#define COLOR_DARK_GREEN  RGBA8(0, 100, 0, 255)
#define COLOR_DARK_BLUE   RGBA8(0, 0, 139, 255)
#define COLOR_LIME        RGBA8(50, 205, 50, 255)
#define COLOR_TEAL        RGBA8(0, 128, 128, 255)

#define COLOR_UI_BG       RGBA8(40, 40, 40, 230)
#define COLOR_UI_BORDER   RGBA8(100, 100, 100, 255)
#define COLOR_UI_SELECTED RGBA8(255, 255, 0, 200)
#define COLOR_TRANSPARENT RGBA8(0, 0, 0, 0)

#define NUM_PALETTE_COLORS 20

typedef struct {
    unsigned int colors[NUM_PALETTE_COLORS];
    int selected;
} ColorPalette;

void palette_init(ColorPalette *palette);
unsigned int palette_get_current(const ColorPalette *palette);
void palette_select_next(ColorPalette *palette);
void palette_select_prev(ColorPalette *palette);
void palette_select_index(ColorPalette *palette, int index);

#endif
