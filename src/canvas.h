#ifndef CANVAS_H
#define CANVAS_H

#include <vita2d.h>

#define SCREEN_W 960
#define SCREEN_H 544

// Dimensione massima brush
#define BRUSH_SIZE_MIN 1
#define BRUSH_SIZE_MAX 30

// Tool types
typedef enum {
    TOOL_PENCIL,
    TOOL_ERASER,
    TOOL_LINE,
    TOOL_RECT,
    TOOL_CIRCLE,
    TOOL_FILL_RECT,
    TOOL_FILL_CIRCLE,
    TOOL_SPRAY,
    TOOL_COUNT
} ToolType;

typedef struct {
    // Buffer pixel del canvas (SCREEN_W * SCREEN_H)
    unsigned int *pixels;
    vita2d_texture *texture;

    // Stato corrente
    unsigned int current_color;
    unsigned int bg_color;
    int brush_size;
    ToolType tool;

    // Per strumenti che richiedono 2 punti (linea, rettangolo, cerchio)
    int shape_start_x;
    int shape_start_y;
    int shape_drawing;  // 1 se stiamo definendo il secondo punto

    // Undo: salvataggio di un singolo stato
    unsigned int *undo_buffer;
    int has_undo;
} Canvas;

int  canvas_init(Canvas *canvas);
void canvas_destroy(Canvas *canvas);
void canvas_clear(Canvas *canvas, unsigned int color);
void canvas_draw_pixel(Canvas *canvas, int x, int y, unsigned int color);
void canvas_draw_brush(Canvas *canvas, int x, int y, int size, unsigned int color);
void canvas_draw_line(Canvas *canvas, int x0, int y0, int x1, int y1, int size, unsigned int color);
void canvas_draw_rect(Canvas *canvas, int x0, int y0, int x1, int y1, unsigned int color);
void canvas_draw_filled_rect(Canvas *canvas, int x0, int y0, int x1, int y1, unsigned int color);
void canvas_draw_circle(Canvas *canvas, int cx, int cy, int radius, unsigned int color);
void canvas_draw_filled_circle(Canvas *canvas, int cx, int cy, int radius, unsigned int color);
void canvas_draw_spray(Canvas *canvas, int x, int y, int radius, unsigned int color);
void canvas_update_texture(Canvas *canvas);
void canvas_render(const Canvas *canvas);
void canvas_save_undo(Canvas *canvas);
void canvas_undo(Canvas *canvas);

// Interpolazione per disegno continuo touch
void canvas_draw_line_brush(Canvas *canvas, int x0, int y0, int x1, int y1, int size, unsigned int color);

#endif
