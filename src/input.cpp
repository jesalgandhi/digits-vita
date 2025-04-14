#include "input.h"
#include "rendering.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <algorithm>

// Time-based values for input
#define FLICK_THRESHOLD 5.0f
#define DRAG_THRESHOLD 3.0f
#define TOUCH_TIME_THRESHOLD 0.15f

// Update physics for all bubbles (movement, velocity, collisions)
void handle_bubble_physics(DigitsGame *game) {
    for (int i = 0; i < game->bubbleCount; i++) {
        if (game->bubbles[i].active) {
            // Update position based on velocity
            game->bubbles[i].x += game->bubbles[i].dx;
            game->bubbles[i].y += game->bubbles[i].dy;
            
            // Apply friction to slow down
            game->bubbles[i].dx *= game->bubbles[i].friction;
            game->bubbles[i].dy *= game->bubbles[i].friction;
            
            // Stop if velocity is very small
            if (fabs(game->bubbles[i].dx) < 0.1f) game->bubbles[i].dx = 0;
            if (fabs(game->bubbles[i].dy) < 0.1f) game->bubbles[i].dy = 0;
            
            // Boundary collisions (with bounce effect)
            if (game->bubbles[i].x - BUBBLE_RADIUS < 0) {
                game->bubbles[i].x = BUBBLE_RADIUS;
                game->bubbles[i].dx = -game->bubbles[i].dx * 0.8f;
            }
            
            if (game->bubbles[i].x + BUBBLE_RADIUS > SCREEN_WIDTH) {
                game->bubbles[i].x = SCREEN_WIDTH - BUBBLE_RADIUS;
                game->bubbles[i].dx = -game->bubbles[i].dx * 0.8f;
            }
            
            if (game->bubbles[i].y - BUBBLE_RADIUS < 0) {
                game->bubbles[i].y = BUBBLE_RADIUS;
                game->bubbles[i].dy = -game->bubbles[i].dy * 0.8f;
            }
            
            if (game->bubbles[i].y + BUBBLE_RADIUS > SCREEN_HEIGHT) {
                game->bubbles[i].y = SCREEN_HEIGHT - BUBBLE_RADIUS;
                game->bubbles[i].dy = -game->bubbles[i].dy * 0.8f;
            }
        }
    }
}

// Handle touch input for the game
void handle_touch_input(DigitsGame *game, SceTouchData touch, SceTouchData prevTouch) {
    if (game->gameCompleted) {
        // No input handling in game over state
        return;
    }

    if (game->rounds[game->currentRound].completed) {
        // Handle navigation buttons when round is completed
        if (touch.reportNum > 0) {
            float tx = touch.report[0].x / 2.0f;
            float ty = touch.report[0].y / 2.0f;
            
            // Check if next round button is pressed
            if (tx > SCREEN_WIDTH - 150 && tx < SCREEN_WIDTH - 50 &&
                ty > SCREEN_HEIGHT - 70 && ty < SCREEN_HEIGHT - 30) {
                go_to_next_round(game);
            }
            
            // Check if prev round button is pressed
            if (tx > 50 && tx < 150 &&
                ty > SCREEN_HEIGHT - 70 && ty < SCREEN_HEIGHT - 30) {
                go_to_prev_round(game);
            }
            
            // Check if submit button is pressed (last round)
            if (game->currentRound == game->totalRounds - 1 &&
                tx > SCREEN_WIDTH - 150 && tx < SCREEN_WIDTH - 50 &&
                ty > SCREEN_HEIGHT - 70 && ty < SCREEN_HEIGHT - 30) {
                submit_game(game);
            }
        }
        
        return;
    }

    // Regular gameplay touch handling
    if (touch.reportNum > 0) {
        float tx = touch.report[0].x / 2.0f;
        float ty = touch.report[0].y / 2.0f;
        
        // Check if touch is on operation bubbles
        for (int i = 0; i < game->bubbleCount; i++) {
            if (game->bubbles[i].active && game->bubbles[i].type == BUBBLE_OP) {
                float dx = tx - game->bubbles[i].x;
                float dy = ty - game->bubbles[i].y;
                float distance = sqrt(dx*dx + dy*dy);
                
                if (distance < OP_BUBBLE_RADIUS) {
                    // Select this operation
                    game->selectedOp = game->bubbles[i].value;
                    
                    // Mark this bubble as selected and deselect others
                    for (int j = 0; j < game->bubbleCount; j++) {
                        if (game->bubbles[j].type == BUBBLE_OP) {
                            game->bubbles[j].selected = (j == i);
                        }
                    }
                    return;
                }
            }
        }
        
        // Check for button presses
        for (int i = 0; i < game->bubbleCount; i++) {
            if (game->bubbles[i].active && game->bubbles[i].type == BUBBLE_BUTTON) {
                float dx = tx - game->bubbles[i].x;
                float dy = ty - game->bubbles[i].y;
                float distance = sqrt(dx*dx + dy*dy);
                
                if (distance < BUBBLE_RADIUS) {
                    // Handle button press
                    switch (game->bubbles[i].value) {
                        case BTN_NEXT_ROUND:
                            go_to_next_round(game);
                            return;
                        case BTN_PREV_ROUND:
                            go_to_prev_round(game);
                            return;
                        case BTN_SUBMIT:
                            submit_game(game);
                            return;
                    }
                }
            }
        }
        
        // Check digit/result bubbles
        for (int i = 0; i < game->bubbleCount; i++) {
            if (game->bubbles[i].active && 
                (game->bubbles[i].type == BUBBLE_DIGIT || game->bubbles[i].type == BUBBLE_RESULT)) {
                
                float dx = tx - game->bubbles[i].x;
                float dy = ty - game->bubbles[i].y;
                float distance = sqrt(dx*dx + dy*dy);
                
                if (distance < BUBBLE_RADIUS) {
                    // Bubble touched
                    
                    // If we have a selected bubble and operation, perform the operation
                    if (game->selectedBubble1 >= 0 && game->selectedOp != OP_NONE && 
                        game->selectedBubble1 != i) {
                        game->selectedBubble2 = i;
                        game->bubbles[i].selected = 1;
                        
                        // Apply the operation
                        apply_operation(game, game->selectedBubble1, game->selectedBubble2);
                        
                        // Reset operation selection
                        game->selectedOp = OP_NONE;
                        for (int j = 0; j < game->bubbleCount; j++) {
                            if (game->bubbles[j].type == BUBBLE_OP) {
                                game->bubbles[j].selected = 0;
                            }
                        }
                    } else {
                        // Select this bubble
                        game->selectedBubble1 = i;
                        game->bubbles[i].selected = 1;
                    }
                    return;
                }
            }
        }
        
        // If touch is not on any interactive element and we have a previous touch,
        // calculate flick velocity for bubble movement
        if (game->selectedBubble1 >= 0 && prevTouch.reportNum > 0) {
            float prev_tx = prevTouch.report[0].x / 2.0f;
            float prev_ty = prevTouch.report[0].y / 2.0f;
            
            // Calculate velocity based on touch movement
            float dx = tx - prev_tx;
            float dy = ty - prev_ty;
            
            // Apply velocity to the selected bubble
            game->bubbles[game->selectedBubble1].dx = dx * 0.2f;
            game->bubbles[game->selectedBubble1].dy = dy * 0.2f;
        }
        
        // If touch is not on any interactive element, deselect
        game->selectedBubble1 = -1;
    }
}

// Handle gamepad button input
void handle_button_input(DigitsGame *game, SceCtrlData pad) {
    // Handle button input for navigation and game control
    if (pad.buttons & SCE_CTRL_CIRCLE) {
        // Reset current selection
        game->selectedBubble1 = -1;
        game->selectedOp = OP_NONE;
    }
    
    if (pad.buttons & SCE_CTRL_TRIANGLE) {
        // Reset current round
        init_round(game, game->currentRound);
    }
    
    // Handle navigation between rounds if current round is completed
    if (game->rounds[game->currentRound].completed) {
        if (pad.buttons & SCE_CTRL_RIGHT) {
            go_to_next_round(game);
            sceKernelDelayThread(200000); // 0.2 second delay to prevent multiple triggers
        }
        
        if (pad.buttons & SCE_CTRL_LEFT) {
            go_to_prev_round(game);
            sceKernelDelayThread(200000); // 0.2 second delay to prevent multiple triggers
        }
    }
}

// Main update function combining all input handling
void update_game(DigitsGame *game, SceTouchData touch, SceTouchData prevTouch, SceCtrlData pad) {
    handle_bubble_physics(game);
    handle_touch_input(game, touch, prevTouch);
    handle_button_input(game, pad);
    
    // Check if the current round is completed
    if (!game->rounds[game->currentRound].completed && game->bubbleCount == 1 && 
        game->bubbles[0].value == game->rounds[game->currentRound].target) {
        // Round completed!
        game->rounds[game->currentRound].completed = 1;
        game->rounds[game->currentRound].userResult = game->bubbles[0].value;
        
        // Check if all rounds are completed
        bool all_completed = true;
        for (int i = 0; i < game->totalRounds; i++) {
            if (!game->rounds[i].completed) {
                all_completed = false;
                break;
            }
        }
        
        if (all_completed) {
            // All rounds completed - show game completion screen
            game->gameWon = 1;
        }
    }
}

// Round navigation functions are defined in game_state.cpp 