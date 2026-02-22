#include "colors.h"

void palette_init(ColorPalette *palette) {
    palette->colors[0]  = COLOR_BLACK;
    palette->colors[1]  = COLOR_WHITE;
    palette->colors[2]  = COLOR_RED;
    palette->colors[3]  = COLOR_GREEN;
    palette->colors[4]  = COLOR_BLUE;
    palette->colors[5]  = COLOR_YELLOW;
    palette->colors[6]  = COLOR_CYAN;
    palette->colors[7]  = COLOR_MAGENTA;
    palette->colors[8]  = COLOR_ORANGE;
    palette->colors[9]  = COLOR_PURPLE;
    palette->colors[10] = COLOR_PINK;
    palette->colors[11] = COLOR_BROWN;
    palette->colors[12] = COLOR_GRAY;
    palette->colors[13] = COLOR_LIGHT_GRAY;
    palette->colors[14] = COLOR_DARK_GRAY;
    palette->colors[15] = COLOR_DARK_RED;
    palette->colors[16] = COLOR_DARK_GREEN;
    palette->colors[17] = COLOR_DARK_BLUE;
    palette->colors[18] = COLOR_LIME;
    palette->colors[19] = COLOR_TEAL;
    palette->selected = 0;
}

unsigned int palette_get_current(const ColorPalette *palette) {
    return palette->colors[palette->selected];
}

void palette_select_next(ColorPalette *palette) {
    palette->selected = (palette->selected + 1) % NUM_PALETTE_COLORS;
}

void palette_select_prev(ColorPalette *palette) {
    palette->selected = (palette->selected - 1 + NUM_PALETTE_COLORS) % NUM_PALETTE_COLORS;
}

void palette_select_index(ColorPalette *palette, int index) {
    if (index >= 0 && index < NUM_PALETTE_COLORS) {
        palette->selected = index;
    }
}
