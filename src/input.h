#ifndef INPUT_H
#define INPUT_H

#include "game_state.h"
#include <psp2/kernel/threadmgr.h> // For sceKernelDelayThread

// Forward declarations for round navigation functions
void go_to_next_round(DigitsGame *game);
void go_to_prev_round(DigitsGame *game);
void submit_game(DigitsGame *game);

// Function declarations for input handling
void update_game(DigitsGame *game, SceTouchData touch, SceTouchData prevTouch, SceCtrlData pad);
void handle_bubble_physics(DigitsGame *game);
void handle_touch_input(DigitsGame *game, SceTouchData touch, SceTouchData prevTouch);
void handle_button_input(DigitsGame *game, SceCtrlData pad);

#endif // INPUT_H 