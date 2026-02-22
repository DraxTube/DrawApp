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

        if (input_button_pressed(&input, SCE_CTRL_START)) {
            running = 0;
            continue;
        }

        if (input_button_pressed(&input, SCE_CTRL_SELECT)) {
            ui.show_help = !ui.show_help;
        }

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

        if (input_button_pressed(&input, SCE_CTRL_TRIANGLE)) {
            canvas.tool = (canvas.tool + 1) % TOOL_COUNT;
            canvas.shape_drawing = 0;
            char msg[64];
            const char *names[] = {
                "Pencil", "Eraser", "Line", "Rect",
                "Circle", "FillRect", "FillCircle", "Spray"
            };
            snprintf(msg, sizeof(msg), "Tool: %s", names[canvas.tool]);
            ui_set_status(&ui, msg);
        }

        if (input_button_pressed(&input, SCE_CTRL_SQUARE)) {
            canvas_save_undo(&canvas);
            canvas_clear(&canvas, canvas.bg_color);
            canvas.shape_drawing = 0;
            ui_set_status(&ui, "Canvas cleared!");
        }

        if (input_button_pressed(&input, SCE_CTRL_CIRCLE)) {
            canvas_undo(&canvas);
            ui_set_status(&ui, "Undo!");
        }

        if (input_button_pressed(&input, SCE_CTRL_CROSS)) {
            ui.show_toolbar = !ui.show_toolbar;
            ui.show_palette = !ui.show_palette;
        }

        if (input_button_pressed(&input, SCE_CTRL_UP)) {
            canvas.brush_size++;
            if (canvas.brush_size > BRUSH_SIZE_MAX)
                canvas.brush_size = BRUSH_SIZE_MAX;
            char msg[32];
            snprintf(msg, sizeof(msg), "Brush: %d", canvas.brush_size);
            ui_set_status(&ui, msg);
        }
        if (input_button_pressed(&input, SCE_CTRL_DOWN)) {
            canvas.brush_size--;
            if (canvas.brush_size < BRUSH_SIZE_MIN)
                canvas.brush_size = BRUSH_SIZE_MIN;
            char msg[32];
            snprintf(msg, sizeof(msg), "Brush: %d", canvas.brush_size);
            ui_set_status(&ui, msg);
        }

        if (input_button_pressed(&input, SCE_CTRL_LTRIGGER)) {
            palette_select_prev(&palette);
            canvas.current_color = palette_get_current(&palette);
        }
        if (input_button_pressed(&input, SCE_CTRL_RTRIGGER)) {
            palette_select_next(&palette);
            canvas.current_color = palette_get_current(&palette);
        }

        if (input.front_touching) {
            int tx = input.front_x;
            int ty = input.front_y;

            int pal_index = ui_palette_hit_test(&ui, tx, ty);
            if (pal_index >= 0) {
                palette_select_index(&palette, pal_index);
                canvas.current_color = palette_get_current(&palette);
            } else if (ui_toolbar_hit_test(&ui, tx, ty)) {
                /* ignore drawing on toolbar */
            } else {
                unsigned int draw_color = canvas.current_color;
                if (canvas.tool == TOOL_ERASER) {
                    draw_color = canvas.bg_color;
                }

                if (is_shape_tool(canvas.tool)) {
                    if (input.front_just_pressed) {
                        canvas_save_undo(&canvas);
                        canvas.shape_start_x = tx;
                        canvas.shape_start_y = ty;
                        canvas.shape_drawing = 1;
                    }
                } else {
                    if (input.front_just_pressed) {
                        canvas_save_undo(&canvas);
                        if (canvas.tool == TOOL_SPRAY) {
                            canvas_draw_spray(&canvas, tx, ty,
                                              canvas.brush_size * 3, draw_color);
                        } else {
                            canvas_draw_brush(&canvas, tx, ty,
                                              canvas.brush_size, draw_color);
                        }
                    } else {
                        if (canvas.tool == TOOL_SPRAY) {
                            canvas_draw_spray(&canvas, tx, ty,
                                              canvas.brush_size * 3, draw_color);
                        } else {
                            canvas_draw_line_brush(&canvas,
                                                   input.front_prev_x,
                                                   input.front_prev_y,
                                                   tx, ty,
                                                   canvas.brush_size, draw_color);
                        }
                    }
                }
            }
        }

        if (input.front_just_released && canvas.shape_drawing) {
            int tx = input.front_prev_x;
            int ty = input.front_prev_y;
            unsigned int draw_color = canvas.current_color;

            switch (canvas.tool) {
                case TOOL_LINE:
                    canvas_draw_line(&canvas,
                                     canvas.shape_start_x, canvas.shape_start_y,
                                     tx, ty, canvas.brush_size, draw_color);
                    break;
                case TOOL_RECT:
                    canvas_draw_rect(&canvas,
                                     canvas.shape_start_x, canvas.shape_start_y,
                                     tx, ty, draw_color);
                    break;
                case TOOL_FILL_RECT:
                    canvas_draw_filled_rect(&canvas,
                                            canvas.shape_start_x, canvas.shape_start_y,
                                            tx, ty, draw_color);
                    break;
                case 
