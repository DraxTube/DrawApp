#ifndef INPUT_H
#define INPUT_H

#include <psp2/ctrl.h>
#include <psp2/touch.h>

#define TOUCH_FRONT  0
#define TOUCH_BACK   1

typedef struct {
    // Touch frontale
    int front_touching;
    int front_x;
    int front_y;
    int front_prev_x;
    int front_prev_y;
    int front_just_pressed;
    int front_just_released;

    // Touch posteriore
    int back_touching;
    int back_x;
    int back_y;

    // Pulsanti
    SceCtrlData pad;
    SceCtrlData pad_prev;
    unsigned int pressed;   // appena premuti
    unsigned int held;      // tenuti
    unsigned int released;  // appena rilasciati

    // Analog sticks
    int lx, ly, rx, ry;
} InputState;

void input_init(void);
void input_update(InputState *state);
int  input_button_pressed(const InputState *state, unsigned int button);
int  input_button_held(const InputState *state, unsigned int button);
int  input_button_released(const InputState *state, unsigned int button);

#endif
