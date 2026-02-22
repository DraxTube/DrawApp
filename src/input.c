#include "input.h"
#include <string.h>
#include <stdlib.h>

void input_init(void) {
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);
    sceTouchEnableTouchForce(SCE_TOUCH_PORT_FRONT);
}

void input_update(InputState *state) {
    state->pad_prev = state->pad;
    int prev_front_touching = state->front_touching;

    memset(&state->pad, 0, sizeof(SceCtrlData));
    sceCtrlPeekBufferPositive(0, &state->pad, 1);

    state->pressed  = state->pad.buttons & ~state->pad_prev.buttons;
    state->held     = state->pad.buttons;
    state->released = ~state->pad.buttons & state->pad_prev.buttons;

    state->lx = (int)state->pad.lx - 128;
    state->ly = (int)state->pad.ly - 128;
    state->rx = (int)state->pad.rx - 128;
    state->ry = (int)state->pad.ry - 128;

    if (abs(state->lx) < 20) state->lx = 0;
    if (abs(state->ly) < 20) state->ly = 0;
    if (abs(state->rx) < 20) state->rx = 0;
    if (abs(state->ry) < 20) state->ry = 0;

    SceTouchData touch;
    sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);

    state->front_prev_x = state->front_x;
    state->front_prev_y = state->front_y;

    if (touch.reportNum > 0) {
        state->front_touching = 1;
        state->front_x = touch.report[0].x / 2;
        state->front_y = touch.report[0].y / 2;
    } else {
        state->front_touching = 0;
    }

    state->front_just_pressed  = (state->front_touching && !prev_front_touching);
    state->front_just_released = (!state->front_touching && prev_front_touching);

    SceTouchData back_touch;
    sceTouchPeek(SCE_TOUCH_PORT_BACK, &back_touch, 1);

    if (back_touch.reportNum > 0) {
        state->back_touching = 1;
        state->back_x = back_touch.report[0].x / 2;
        state->back_y = back_touch.report[0].y / 2;
    } else {
        state->back_touching = 0;
    }
}

int input_button_pressed(const InputState *state, unsigned int button) {
    return (state->pressed & button) != 0;
}

int input_button_held(const InputState *state, unsigned int button) {
    return (state->held & button) != 0;
}

int input_button_released(const InputState *state, unsigned int button) {
    return (state->released & button) != 0;
}
