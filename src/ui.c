#include "ui.h"
#include <vita2d.h>
#include <string.h>
#include <stdio.h>

static vita2d_pgf *font = NULL;

static const char *tool_names[TOOL_COUNT] = {
    "Pencil", "Eraser", "Line", "Rect", "Circle", "FillRect", "FillCirc", "Spray"
};

void ui_init(UIState *ui) {
    ui->show_toolbar = 1;
    ui->show_palette = 1;
    ui->show_help = 0;
    ui->status_msg[0] = '\0';
    ui->status_timer = 0;

    font = vita2d_load_default_pgf();
}

void ui_set_status(UIState *ui, const char *msg) {
    strncpy(ui->status_msg, msg, sizeof(ui->status_msg) - 1);
    ui->status_msg[sizeof(ui->status_msg) - 1] = '\0';
    ui->status_timer = 120; // ~2 secondi a 60fps
}

void ui_update(UIState *ui) {
    if (ui->status_timer > 0) {
        ui->status_timer--;
    }
}

void ui_render_toolbar(const UIState *ui, const Canvas *canvas, const ColorPalette *palette) {
    if (!ui->show_toolbar) return;

    // Sfondo toolbar
    vita2d_draw_rectangle(0, UI_TOOLBAR_Y, SCREEN_W, UI_TOOLBAR_HEIGHT, COLOR_UI_BG);
    vita2d_draw_line(0, UI_TOOLBAR_HEIGHT, SCREEN_W, UI_TOOLBAR_HEIGHT, COLOR_UI_BORDER);

    // Colore corrente (quadrato preview)
    vita2d_draw_fill_circle(20, 20, 12, palette_get_current(palette));
    vita2d_draw_rectangle(5, 5, 30, 30, COLOR_UI_BORDER);

    // Tool corrente
    char tool_info[128];
    snprintf(tool_info, sizeof(tool_info), "Tool: %s  |  Size: %d  |  L/R: Color  |  SELECT: Help",
             tool_names[canvas->tool], canvas->brush_size);
    vita2d_pgf_draw_text(font, 45, 25, COLOR_WHITE, 0.8f, tool_info);

    // Status message
    if (ui->status_timer > 0) {
        vita2d_pgf_draw_text(font, SCREEN_W / 2 - 60, SCREEN_H / 2 - 50,
                             COLOR_YELLOW, 1.0f, ui->status_msg);
    }
}

void ui_render_palette(const UIState *ui, const ColorPalette *palette) {
    if (!ui->show_palette) return;

    // Sfondo palette
    vita2d_draw_rectangle(0, UI_PALETTE_Y, SCREEN_W, UI_PALETTE_HEIGHT, COLOR_UI_BG);
    vita2d_draw_line(0, UI_PALETTE_Y, SCREEN_W, UI_PALETTE_Y, COLOR_UI_BORDER);

    int box_size = 25;
    int spacing = 3;
    int total_w = NUM_PALETTE_COLORS * (box_size + spacing);
    int start_x = (SCREEN_W - total_w) / 2;
    int y = UI_PALETTE_Y + 5;

    for (int i = 0; i < NUM_PALETTE_COLORS; i++) {
        int x = start_x + i * (box_size + spacing);

        // Bordo selezionato
        if (i == palette->selected) {
            vita2d_draw_rectangle(x - 2, y - 2, box_size + 4, box_size + 4, COLOR_UI_SELECTED);
        }

        // Colore
        vita2d_draw_rectangle(x, y, box_size, box_size, palette->colors[i]);

        // Bordo sottile
        vita2d_draw_line(x, y, x + box_size, y, COLOR_UI_BORDER);
        vita2d_draw_line(x, y + box_size, x + box_size, y + box_size, COLOR_UI_BORDER);
        vita2d_draw_line(x, y, x, y + box_size, COLOR_UI_BORDER);
        vita2d_draw_line(x + box_size, y, x + box_size, y + box_size, COLOR_UI_BORDER);
    }
}

void ui_render_cursor(int x, int y, int brush_size, unsigned int color) {
    // Cursore circolare
    int r = brush_size / 2;
    if (r < 2) r = 2;
    vita2d_draw_fill_circle(x, y, r, color);
    // Contorno
    // Disegno un cerchio di contorno approssimato
    for (float a = 0; a < 6.283f; a += 0.1f) {
        int cx = x + (int)(r * cosf(a));
        int cy = y + (int)(r * sinf(a));
        vita2d_draw_pixel(cx, cy, COLOR_BLACK);
    }
}

void ui_render_help(void) {
    int x = 100, y = 80;
    int w = 760, h = 400;

    vita2d_draw_rectangle(x, y, w, h, RGBA8(20, 20, 20, 240));
    vita2d_draw_line(x, y, x + w, y, COLOR_UI_BORDER);
    vita2d_draw_line(x, y + h, x + w, y + h, COLOR_UI_BORDER);
    vita2d_draw_line(x, y, x, y + h, COLOR_UI_BORDER);
    vita2d_draw_line(x + w, y, x + w, y + h, COLOR_UI_BORDER);

    vita2d_pgf *f = vita2d_load_default_pgf();

    int line = y + 30;
    int step = 28;

    vita2d_pgf_draw_text(f, x + 20, line, COLOR_YELLOW, 1.0f, "=== DrawApp - Help ===");
    line += step + 10;
    vita2d_pgf_draw_text(f, x + 20, line, COLOR_WHITE, 0.85f, "Touch screen: Draw on canvas");
    line += step;
    vita2d_pgf_draw_text(f, x + 20, line, COLOR_WHITE, 0.85f, "D-Pad UP/DOWN: Change brush size");
    line += step;
    vita2d_pgf_draw_text(f, x + 20, line, COLOR_WHITE, 0.85f, "L/R triggers: Prev/Next color");
    line += step;
    vita2d_pgf_draw_text(f, x + 20, line, COLOR_WHITE, 0.85f, "Triangle: Cycle tools (Pencil/Eraser/Line/...)");
    line += step;
    vita2d_pgf_draw_text(f, x + 20, line, COLOR_WHITE, 0.85f, "Square: Clear canvas");
    line += step;
    vita2d_pgf_draw_text(f, x + 20, line, COLOR_WHITE, 0.85f, "Circle: Undo last action");
    line += step;
    vita2d_pgf_draw_text(f, x + 20, line, COLOR_WHITE, 0.85f, "Cross: Toggle toolbar/palette visibility");
    line += step;
    vita2d_pgf_draw_text(f, x + 20, line, COLOR_WHITE, 0.85f, "SELECT: Show/Hide this help");
    line += step;
    vita2d_pgf_draw_text(f, x + 20, line, COLOR_WHITE, 0.85f, "START: Exit application");
    line += step + 10;
    vita2d_pgf_draw_text(f, x + 20, line, COLOR_CYAN, 0.85f, "Touch palette bar to select color directly!");

    vita2d_free_pgf(f);
}

void ui_render_shape_preview(const Canvas *canvas, int x, int y) {
    if (!canvas->shape_drawing) return;

    unsigned int preview_color = RGBA8(200, 200, 200, 150);

    switch (canvas->tool) {
        case TOOL_LINE:
            vita2d_draw_line(canvas->shape_start_x, canvas->shape_start_y, x, y, preview_color);
            break;
        case TOOL_RECT:
        case TOOL_FILL_RECT: {
            int sx = canvas->shape_start_x;
            int sy = canvas->shape_start_y;
            int minx = (sx < x) ? sx : x;
            int miny = (sy < y) ? sy : y;
            int w = abs(x - sx);
            int h = abs(y - sy);
            vita2d_draw_line(minx, miny, minx + w, miny, preview_color);
            vita2d_draw_line(minx, miny + h, minx + w, miny + h, preview_color);
            vita2d_draw_line(minx, miny, minx, miny + h, preview_color);
            vita2d_draw_line(minx + w, miny, minx + w, miny + h, preview_color);
            break;
        }
        case TOOL_CIRCLE:
        case TOOL_FILL_CIRCLE: {
            int dx = x - canvas->shape_start_x;
            int dy = y - canvas->shape_start_y;
            int r = (int)sqrtf((float)(dx * dx + dy * dy));
            // Preview circle outline usando vita2d
            for (float a = 0; a < 6.283f; a += 0.02f) {
                int px = canvas->shape_start_x + (int)(r * cosf(a));
                int py = canvas->shape_start_y + (int)(r * sinf(a));
                vita2d_draw_pixel(px, py, preview_color);
            }
            break;
        }
        default:
            break;
    }
}

int ui_palette_hit_test(const UIState *ui, int x, int y) {
    if (!ui->show_palette) return -1;
    if (y < UI_PALETTE_Y || y > UI_PALETTE_Y + UI_PALETTE_HEIGHT) return -1;

    int box_size = 25;
    int spacing = 3;
    int total_w = NUM_PALETTE_COLORS * (box_size + spacing);
    int start_x = (SCREEN_W - total_w) / 2;

    for (int i = 0; i < NUM_PALETTE_COLORS; i++) {
        int bx = start_x + i * (box_size + spacing);
        if (x >= bx && x <= bx + box_size) {
            return i;
        }
    }
    return -1;
}

int ui_toolbar_hit_test(const UIState *ui, int x, int y) {
    if (!ui->show_toolbar) return 0;
    return (y >= UI_TOOLBAR_Y && y <= UI_TOOLBAR_Y + UI_TOOLBAR_HEIGHT);
}
