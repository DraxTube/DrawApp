#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>
#include <vita2d.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "canvas.h"
#include "colors.h"
#include "input.h"
#include "ui.h"

static int is_shape_tool(ToolType tool) {
    return (tool == TOOL_LINE || tool == TOOL_RECT || tool == TOOL_CIRCLE ||
            tool == TOOL_FILL_RECT || tool == TOOL_FILL_CIRCLE);
}

int main(void) {
    vita2d_init();
    vita2d_set_clear_color(RGBA8(50, 50, 50, 255));
    vita2d_set_vblank_wait(1);

    InputState input;
    memset(&input, 0, sizeof(InputState));
    input_init();

    Canvas canvas;
    if (canvas_init(&canvas) < 0) {
        sceKernelExitProcess(0);
        return -1;
    }

    ColorPalette palette;
    palette_init(&palette);
    canvas.current_color = palette_get_current(&palette);

    UIState ui;
    ui_init(&ui);
    ui_set_status(&ui, "Welcome to DrawApp!");

    int running = 1;

    while (running) {
        input_update(&input);

        /* START = esci */
        if (input_button_pressed(&input, SCE_CTRL_START)) {
            running = 0;
            continue;
        }

        /* SELECT = help */
        if (input_button_pressed(&input, SCE_CTRL_SELECT)) {
            ui.show_help = !ui.show_help;
        }

        /* Se help aperta, render e skip */
        if (ui.show_help) {
            vita2d_start_drawing();
            vita2d_clear_screen();
            canvas_update_texture(&canvas);
            canvas_render(&canvas);
            ui_render_toolbar(&ui, &canvas, &palette);
            ui_render_palette(&ui, &palette);
            ui_render_help();
            vita2d_end_drawing();
            vita2d_swap_buffers();
            sceDisplayWaitVblankStart();
            continue;
        }

        /* Triangle = cambia tool */
        if (input_button_pressed(&input, SCE_CTRL_TRIANGLE)) {
            canvas.tool = (canvas.tool + 1) % TOOL_COUNT;
            canvas.shape_drawing = 0;
            const char *names[] = {
                "Pencil", "Eraser", "Line", "Rect",
                "Circle", "FillRect", "FillCircle", "Spray"
            };
            char msg[64];
            snprintf(msg, sizeof(msg), "Tool: %s", names[canvas.tool]);
            ui_set_status(&ui, msg);
        }

        /* Square = clear */
        if (input_button_pressed(&input, SCE_CTRL_SQUARE)) {
            canvas_save_undo(&canvas);
            canvas_clear(&canvas, canvas.bg_color);
            canvas.shape_drawing = 0;
            ui_set_status(&ui, "Canvas cleared!");
        }

        /* Circle = undo */
        if (input_button_pressed(&input, SCE_CTRL_CIRCLE)) {
            canvas_undo(&canvas);
            ui_set_status(&ui, "Undo!");
        }

        /* Cross = toggle UI */
        if (input_button_pressed(&input, SCE_CTRL_CROSS)) {
            ui.show_toolbar = !ui.show_toolbar;
            ui.show_palette = !ui.show_palette;
        }

        /* D-Pad UP = brush + */
        if (input_button_pressed(&input, SCE_CTRL_UP)) {
            canvas.brush_size++;
            if (canvas.brush_size > BRUSH_SIZE_MAX)
                canvas.brush_size = BRUSH_SIZE_MAX;
            char msg[32];
            snprintf(msg, sizeof(msg), "Brush: %d", canvas.brush_size);
            ui_set_status(&ui, msg);
        }

        /* D-Pad DOWN = brush - */
        if (input_button_pressed(&input, SCE_CTRL_DOWN)) {
            canvas.brush_size--;
            if (canvas.brush_size < BRUSH_SIZE_MIN)
                canvas.brush_size = BRUSH_SIZE_MIN;
            char msg[32];
            snprintf(msg, sizeof(msg), "Brush: %d", canvas.brush_size);
            ui_set_status(&ui, msg);
        }

        /* L = colore precedente */
        if (input_button_pressed(&input, SCE_CTRL_LTRIGGER)) {
            palette_select_prev(&palette);
            canvas.current_color = palette_get_current(&palette);
        }

        /* R = colore successivo */
        if (input_button_pressed(&input, SCE_CTRL_RTRIGGER)) {
            palette_select_next(&palette);
            canvas.current_color = palette_get_current(&palette);
        }

        /* ===== TOUCH DRAWING ===== */
        if (input.front_touching) {
            int tx = input.front_x;
            int ty = input.front_y;

            /* Tocco sulla palette? */
            int pal_index = ui_palette_hit_test(&ui, tx, ty);
            if (pal_index >= 0) {
                palette_select_index(&palette, pal_index);
                canvas.current_color = palette_get_current(&palette);
            }
            /* Tocco sulla toolbar? */
            else if (ui_toolbar_hit_test(&ui, tx, ty)) {
                /* ignora */
            }
            /* Disegno sul canvas */
            else {
                unsigned int draw_color = canvas.current_color;
                if (canvas.tool == TOOL_ERASER) {
                    draw_color = canvas.bg_color;
                }

                if (is_shape_tool(canvas.tool)) {
                    /* Primo tocco: salva punto iniziale */
                    if (input.front_just_pressed) {
                        canvas_save_undo(&canvas);
                        canvas.shape_start_x = tx;
                        canvas.shape_start_y = ty;
                        canvas.shape_drawing = 1;
                    }
                } else {
                    /* Strumenti continui */
                    if (input.front_just_pressed) {
                        canvas_save_undo(&canvas);
                        if (canvas.tool == TOOL_SPRAY) {
                            canvas_draw_spray(&canvas, tx, ty,
                                              canvas.brush_size * 3,
                                              draw_color);
                        } else {
                            canvas_draw_brush(&canvas, tx, ty,
                                              canvas.brush_size,
                                              draw_color);
                        }
                    } else {
                        /* Disegno continuo interpolato */
                        if (canvas.tool == TOOL_SPRAY) {
                            canvas_draw_spray(&canvas, tx, ty,
                                              canvas.brush_size * 3,
                                              draw_color);
                        } else {
                            canvas_draw_line_brush(&canvas,
                                                   input.front_prev_x,
                                                   input.front_prev_y,
                                                   tx, ty,
                                                   canvas.brush_size,
                                                   draw_color);
                        }
                    }
                }
            }
        }

        /* ===== RILASCIO TOUCH: FINALIZZA SHAPE ===== */
        if (input.front_just_released && canvas.shape_drawing) {
            int tx = input.front_prev_x;
            int ty = input.front_prev_y;
            unsigned int draw_color = canvas.current_color;
            int dx, dy, r;

            switch (canvas.tool) {
                case TOOL_LINE:
                    canvas_draw_line(&canvas,
                                     canvas.shape_start_x,
                                     canvas.shape_start_y,
                                     tx, ty,
                                     canvas.brush_size, draw_color);
                    break;

                case TOOL_RECT:
                    canvas_draw_rect(&canvas,
                                     canvas.shape_start_x,
                                     canvas.shape_start_y,
                                     tx, ty, draw_color);
                    break;

                case TOOL_FILL_RECT:
                    canvas_draw_filled_rect(&canvas,
                                            canvas.shape_start_x,
                                            canvas.shape_start_y,
                                            tx, ty, draw_color);
                    break;

                case TOOL_CIRCLE:
                    dx = tx - canvas.shape_start_x;
                    dy = ty - canvas.shape_start_y;
                    r = (int)sqrtf((float)(dx * dx + dy * dy));
                    canvas_draw_circle(&canvas,
                                       canvas.shape_start_x,
                                       canvas.shape_start_y,
                                       r, draw_color);
                    break;

                case TOOL_FILL_CIRCLE:
                    dx = tx - canvas.shape_start_x;
                    dy = ty - canvas.shape_start_y;
                    r = (int)sqrtf((float)(dx * dx + dy * dy));
                    canvas_draw_filled_circle(&canvas,
                                              canvas.shape_start_x,
                                              canvas.shape_start_y,
                                              r, draw_color);
                    break;

                default:
                    break;
            }
            canvas.shape_drawing = 0;
        }

        /* Aggiorna UI */
        ui_update(&ui);

        /* ===== RENDERING ===== */
        vita2d_start_drawing();
        vita2d_clear_screen();

        canvas_update_texture(&canvas);
        canvas_render(&canvas);

        /* Preview shape */
        if (canvas.shape_drawing && input.front_touching) {
            ui_render_shape_preview(&canvas, input.front_x, input.front_y);
        }

        /* Cursore touch */
        if (input.front_touching) {
            ui_render_cursor(input.front_x, input.front_y,
                             canvas.brush_size, canvas.current_color);
        }

        /* Toolbar e palette */
        ui_render_toolbar(&ui, &canvas, &palette);
        ui_render_palette(&ui, &palette);

        vita2d_end_drawing();
        vita2d_swap_buffers();
        sceDisplayWaitVblankStart();
    }

    /* Cleanup */
    canvas_destroy(&canvas);
    vita2d_fini();
    sceKernelExitProcess(0);
    return 0;
}
