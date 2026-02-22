#include "canvas.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

int canvas_init(Canvas *canvas) {
    canvas->pixels = (unsigned int *)malloc(SCREEN_W * SCREEN_H * sizeof(unsigned int));
    if (!canvas->pixels) return -1;

    canvas->undo_buffer = (unsigned int *)malloc(SCREEN_W * SCREEN_H * sizeof(unsigned int));
    if (!canvas->undo_buffer) {
        free(canvas->pixels);
        return -1;
    }

    canvas->texture = vita2d_create_empty_texture_format(SCREEN_W, SCREEN_H, SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
    if (!canvas->texture) {
        free(canvas->pixels);
        free(canvas->undo_buffer);
        return -1;
    }

    canvas->bg_color = RGBA8(255, 255, 255, 255);
    canvas->current_color = RGBA8(0, 0, 0, 255);
    canvas->brush_size = 3;
    canvas->tool = TOOL_PENCIL;
    canvas->shape_drawing = 0;
    canvas->has_undo = 0;

    canvas_clear(canvas, canvas->bg_color);

    return 0;
}

void canvas_destroy(Canvas *canvas) {
    if (canvas->pixels) free(canvas->pixels);
    if (canvas->undo_buffer) free(canvas->undo_buffer);
    if (canvas->texture) vita2d_free_texture(canvas->texture);
}

void canvas_clear(Canvas *canvas, unsigned int color) {
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
        canvas->pixels[i] = color;
    }
}

void canvas_draw_pixel(Canvas *canvas, int x, int y, unsigned int color) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        canvas->pixels[y * SCREEN_W + x] = color;
    }
}

void canvas_draw_brush(Canvas *canvas, int x, int y, int size, unsigned int color) {
    if (size <= 1) {
        canvas_draw_pixel(canvas, x, y, color);
        return;
    }
    int r = size / 2;
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx * dx + dy * dy <= r * r) {
                canvas_draw_pixel(canvas, x + dx, y + dy, color);
            }
        }
    }
}

// Bresenham line
void canvas_draw_line(Canvas *canvas, int x0, int y0, int x1, int y1, int size, unsigned int color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        canvas_draw_brush(canvas, x0, y0, size, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

void canvas_draw_line_brush(Canvas *canvas, int x0, int y0, int x1, int y1, int size, unsigned int color) {
    canvas_draw_line(canvas, x0, y0, x1, y1, size, color);
}

void canvas_draw_rect(Canvas *canvas, int x0, int y0, int x1, int y1, unsigned int color) {
    // Ordina coordinate
    int minx = (x0 < x1) ? x0 : x1;
    int maxx = (x0 > x1) ? x0 : x1;
    int miny = (y0 < y1) ? y0 : y1;
    int maxy = (y0 > y1) ? y0 : y1;

    for (int x = minx; x <= maxx; x++) {
        canvas_draw_pixel(canvas, x, miny, color);
        canvas_draw_pixel(canvas, x, maxy, color);
    }
    for (int y = miny; y <= maxy; y++) {
        canvas_draw_pixel(canvas, minx, y, color);
        canvas_draw_pixel(canvas, maxx, y, color);
    }
}

void canvas_draw_filled_rect(Canvas *canvas, int x0, int y0, int x1, int y1, unsigned int color) {
    int minx = (x0 < x1) ? x0 : x1;
    int maxx = (x0 > x1) ? x0 : x1;
    int miny = (y0 < y1) ? y0 : y1;
    int maxy = (y0 > y1) ? y0 : y1;

    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            canvas_draw_pixel(canvas, x, y, color);
        }
    }
}

// Midpoint circle
void canvas_draw_circle(Canvas *canvas, int cx, int cy, int radius, unsigned int color) {
    int x = radius, y = 0;
    int err = 0;

    while (x >= y) {
        canvas_draw_pixel(canvas, cx + x, cy + y, color);
        canvas_draw_pixel(canvas, cx + y, cy + x, color);
        canvas_draw_pixel(canvas, cx - y, cy + x, color);
        canvas_draw_pixel(canvas, cx - x, cy + y, color);
        canvas_draw_pixel(canvas, cx - x, cy - y, color);
        canvas_draw_pixel(canvas, cx - y, cy - x, color);
        canvas_draw_pixel(canvas, cx + y, cy - x, color);
        canvas_draw_pixel(canvas, cx + x, cy - y, color);

        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        }
        if (err > 0) {
            x--;
            err -= 2 * x + 1;
        }
    }
}

void canvas_draw_filled_circle(Canvas *canvas, int cx, int cy, int radius, unsigned int color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                canvas_draw_pixel(canvas, cx + x, cy + y, color);
            }
        }
    }
}

void canvas_draw_spray(Canvas *canvas, int x, int y, int radius, unsigned int color) {
    for (int i = 0; i < radius * radius; i++) {
        int dx = (rand() % (radius * 2 + 1)) - radius;
        int dy = (rand() % (radius * 2 + 1)) - radius;
        if (dx * dx + dy * dy <= radius * radius) {
            canvas_draw_pixel(canvas, x + dx, y + dy, color);
        }
    }
}

void canvas_update_texture(Canvas *canvas) {
    unsigned int *tex_data = (unsigned int *)vita2d_texture_get_datap(canvas->texture);
    int stride = vita2d_texture_get_stride(canvas->texture) / sizeof(unsigned int);

    for (int y = 0; y < SCREEN_H; y++) {
        memcpy(&tex_data[y * stride], &canvas->pixels[y * SCREEN_W], SCREEN_W * sizeof(unsigned int));
    }
}

void canvas_render(const Canvas *canvas) {
    vita2d_draw_texture(canvas->texture, 0, 0);
}

void canvas_save_undo(Canvas *canvas) {
    memcpy(canvas->undo_buffer, canvas->pixels, SCREEN_W * SCREEN_H * sizeof(unsigned int));
    canvas->has_undo = 1;
}

void canvas_undo(Canvas *canvas) {
    if (canvas->has_undo) {
        // Swap buffers
        unsigned int *temp = (unsigned int *)malloc(SCREEN_W * SCREEN_H * sizeof(unsigned int));
        if (temp) {
            memcpy(temp, canvas->pixels, SCREEN_W * SCREEN_H * sizeof(unsigned int));
            memcpy(canvas->pixels, canvas->undo_buffer, SCREEN_W * SCREEN_H * sizeof(unsigned int));
            memcpy(canvas->undo_buffer, temp, SCREEN_W * SCREEN_H * sizeof(unsigned int));
            free(temp);
        }
    }
}
