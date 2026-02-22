#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>
#include <vita2d.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "canvas.h"
#include "colors.h"
#include "input.h"
#include "ui.h"

// Helper: verifica se il tool è di tipo "shape" (richiede 2 punti)
static int is_shape_tool(ToolType tool) {
    return (tool == TOOL_LINE || tool == TOOL_RECT || tool == TOOL_CIRCLE ||
            tool == TOOL_FILL_RECT || tool == TOOL_FILL_CIRCLE);
}

int main(void) {
    // Inizializzazione vita2d
    vita2d_init();
    vita2d_set_clear_color(RGBA8(50, 50, 50, 255));
    vita2d_set_vblank_wait(1);

    // Inizializzazione componenti
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
        // Aggiorna input
        input_update(&input);

        // === GESTIONE PULSANTI ===

        // START → Esci
        if (input_button_pressed(&input, SCE_CTRL_START)) {
            running = 0;
            continue;
        }

        // SELECT → Help
        if (input_button_pressed(&input, SCE_CTRL_SELECT)) {
            ui.show_help = !ui.show_help;
        }

        // Se la help è aperta, non processare altri input
        if (ui.show_help) {
            // Rendering
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

        // Triangle → Cambia tool
        if (input_button_pressed(&input, SCE_CTRL_TRIANGLE)) {
            canvas.tool = (canvas.tool + 1) % TOOL_COUNT;
            canvas.shape_drawing = 0;
            char msg[64];
            const char *names[] = {"Pencil", "Eraser", "Line", "Rect", "Circle", "FillRect", "FillCircle", "Spray"};
            snprintf(msg, sizeof(msg), "Tool: %s", names[canvas.tool]);
            ui_set_status(&ui, msg);
        }

        // Square → Clear
        if (input_button_pressed(&input, SCE_CTRL_SQUARE)) {
            canvas_save_undo(&canvas);
            canvas_clear(&canvas, canvas.bg_color);
            canvas.shape_drawing = 0;
            ui_set_status(&ui, "Canvas cleared!");
        }

        // Circle → Undo
        if (input_button_pressed(&input, SCE_CTRL_CIRCLE)) {
            canvas_undo(&canvas);
            ui_set_status(&ui, "Undo!");
        }

        // Cross → Toggle UI
        if (input_button_pressed(&input, SCE_CTRL_CROSS)) {
            ui.show_toolbar = !ui.show_toolbar;
            ui.show_palette = !ui.show_palette;
        }

        // D-Pad UP/DOWN → Brush size
        if (input_button_pressed(&input, SCE_CTRL_UP)) {
            canvas.brush_size++;
            if (canvas.brush_size > BRUSH_SIZE_MAX) canvas.brush_size = BRUSH_SIZE_MAX;
            char msg[32];
            snprintf(msg, sizeof(msg), "Brush: %d", canvas.brush_size);
            ui_set_status(&ui, msg);
        }
        if (input_button_pressed(&input, SCE_CTRL_DOWN)) {
            canvas.brush_size--;
            if (canvas.brush_size < BRUSH_SIZE_MIN) canvas.brush_size = BRUSH_SIZE_MIN;
            char msg[32];
            snprintf(msg, sizeof(msg), "Brush: %d", canvas.brush_size);
            ui_set_status(&ui, msg);
        }

        // L/R → Colore
        if (input_button_pressed(&input, SCE_CTRL_LTRIGGER)) {
            palette_select_prev(&palette);
            canvas.current_color = palette_get_current(&palette);
        }
        if (input_button_pressed(&input, SCE_CTRL_RTRIGGER)) {
            palette_select_next(&palette);
            canvas.current_color = palette_get_current(&palette);
        }

        // === GESTIONE TOUCH ===
        if (input.front_touching) {
            int tx = input.front_x;
            int ty = input.front_y;

            // Controlla se il touch è sulla palette
            int pal_index = ui_palette_hit_test(&ui, tx, ty);
            if (pal_index >= 0) {
                palette_select_index(&palette, pal_index);
                canvas.current_color = palette_get_current(&palette);
            }
            // Controlla se è sulla toolbar (ignora disegno)
            else if (ui_toolbar_hit_test(&ui, tx, ty)) {
                // Non disegnare sulla toolbar
            }
            else {
                // Disegno sul canvas
                unsigned int draw_color = canvas.current_color;
                if (canvas.tool == TOOL_ERASER) {
                    draw_color = canvas.bg_color;
                }

                if (is_shape_tool(canvas.tool)) {
                    // Primo tocco: salva punto iniziale
                    if (input.front_just_pressed) {
                        canvas_save_undo(&canvas);
                        canvas.shape_start_x = tx;
                        canvas.shape_start_y = ty;
                        canvas.shape_drawing = 1;
                    }
                    // Il preview viene disegnato nel render
                } else {
                    // Tool continui (pencil, eraser, spray)
                    if (input.front_just_pressed) {
                        canvas_save_undo(&canvas);
                        if (canvas.tool == TOOL_SPRAY) {
                            canvas_draw_spray(&canvas, tx, ty, canvas.brush_size * 3, draw_color);
                        } else {
                            canvas_draw_brush(&canvas, tx, ty, canvas.brush_size, draw_color);
                        }
                    } else {
                        // Disegno continuo con interpolazione
                        if (canvas.tool == TOOL_SPRAY) {
                            canvas_draw_spray(&canvas, tx, ty, canvas.brush_size * 3, draw_color);
                        } else {
                            canvas_draw_line_brush(&canvas, input.front_prev_x, input.front_prev_y,
                                                   tx, ty, canvas.brush_size, draw_color);
                        }
                    }
                }
            }
        }

        // Rilascio touch → finalizza shape
        if (input.front_just_released && canvas.shape_drawing) {
            int tx = input.front_prev_x;
            int ty = input.front_prev_y;
            unsigned int draw_color = canvas.current_color;

            switch (canvas.tool) {
                case TOOL_LINE:
                    canvas_draw_line(&canvas, canvas.shape_start_x, canvas.shape_start_y,
                                     tx, ty, canvas.brush_size, draw_color);
                    break;
                case TOOL_RECT:
                    canvas_draw_rect(&canvas, canvas.shape_start_x, canvas.shape_start_y,
                                     tx, ty, draw_color);
                    break;
                case TOOL_FILL_RECT:
                    canvas_draw_filled_rect(&canvas, canvas.shape_start_x, canvas.shape_start_y,
                                            tx, ty, draw_color);
                    break;
                case TOOL_CIRCLE: {
                    int dx = tx - canvas.shape_start_x;
                    int dy = ty - canvas.shape_start_y;
                    int r = (int)sqrtf((float)(dx * dx + dy * dy));
                    canvas_draw_circle(&canvas, canvas.shape_start_x, canvas.shape_start_y, r, draw_color);
                    break;
                }
                case TOOL_FILL_CIRCLE: {
                    int dx = tx - canvas.shape_start_x;
                    int dy = ty - canvas.shape_start_y;
                    int r = (int)sqrtf((float)(dx * dx + dy * dy));
                    canvas_draw_filled_circle(&canvas, canvas.shape_start_x, canvas.shape_start_y, r, draw_color);
                    break;
                }
                default:
                    break;
            }
            canvas.shape_drawing = 0;
        }

        // Aggiorna UI timer
        ui_update(&ui);

        // === RENDERING ===
        vita2d_start_drawing();
        vita2d_clear_screen();

        // Canvas
        canvas_update_texture(&canvas);
        canvas_render(&canvas);

        // Preview shape in corso
        if (canvas.shape_drawing && input.front_touching) {
            ui_render_shape_preview(&canvas, input.front_x, input.front_y);
        }

        // Cursore touch
        if (input.front_touching) {
            ui_render_cursor(input.front_x, input.front_y, canvas.brush_size, canvas.current_color);
        }

        // UI
        ui_render_toolbar(&ui, &canvas, &palette);
        ui_render_palette(&ui, &palette);

        vita2d_end_drawing();
        vita2d_swap_buffers();
        sceDisplayWaitVblankStart();
    }

    // Cleanup
    canvas_destroy(&canvas);
    vita2d_fini();
    sceKernelExitProcess(0);
    return 0;
}
