#include "game_state.h"
#include <stdio.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Initialize the bubble colors with Frutiger Aero style colors
void init_colors(DigitsGame *game) {
    // Frutiger Aero-inspired colors (pastel with glass-like appearance)
    game->colors[0] = RGBA8(120, 180, 255, 220);  // Blue
    game->colors[1] = RGBA8(255, 150, 150, 220);  // Red
    game->colors[2] = RGBA8(150, 255, 150, 220);  // Green
    game->colors[3] = RGBA8(255, 255, 150, 220);  // Yellow
    game->colors[4] = RGBA8(210, 150, 255, 220);  // Purple
    game->colors[5] = RGBA8(255, 200, 150, 220);  // Orange
    
    // Operation bubbles color (light blue)
    game->opColor = RGBA8(100, 170, 255, 220);
}

// Initialize a new bubble
void init_bubble(DigitsGame *game, int index, int type, int value, float x, float y) {
    game->bubbles[index].type = type;
    game->bubbles[index].value = value;
    game->bubbles[index].x = x;
    game->bubbles[index].y = y;
    game->bubbles[index].dx = 0.0f;
    game->bubbles[index].dy = 0.0f;
    game->bubbles[index].friction = 0.95f;
    game->bubbles[index].active = 1;
    game->bubbles[index].selected = 0;
    game->bubbles[index].scale = 0.5f;  // Start small for pop-in animation
    
    // Set appropriate color based on type
    if (type == BUBBLE_OP) {
        game->bubbles[index].color = -1; // Special flag for operation bubbles
    } else if (type == BUBBLE_BUTTON) {
        game->bubbles[index].color = -2; // Special flag for button bubbles
    } else {
        game->bubbles[index].color = rand() % 6;
    }
}

// Check if we can reach target with given digits
int can_reach_target(int digits[], int n, int target) {
    // Simple recursive function to check if target can be reached
    // Note: This is a simplified algorithm and may not catch all possible solutions
    
    if (n <= 0) return 0;
    
    // If one of the digits is the target, we can reach it
    for (int i = 0; i < n; i++) {
        if (digits[i] == target) return 1;
    }
    
    // Try all operations between pairs of digits
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) continue;
            
            // Create a new array without the used digits
            int newDigits[MAX_DIGITS_PER_ROUND];
            int newN = 0;
            
            for (int k = 0; k < n; k++) {
                if (k != i && k != j) {
                    newDigits[newN++] = digits[k];
                }
            }
            
            // Try addition
            newDigits[newN] = digits[i] + digits[j];
            if (can_reach_target(newDigits, newN + 1, target)) return 1;
            
            // Try subtraction (only if result is positive)
            if (digits[i] > digits[j]) {
                newDigits[newN] = digits[i] - digits[j];
                if (can_reach_target(newDigits, newN + 1, target)) return 1;
            }
            
            // Try multiplication
            newDigits[newN] = digits[i] * digits[j];
            if (can_reach_target(newDigits, newN + 1, target)) return 1;
            
            // Try division (only if divisible evenly)
            if (digits[j] != 0 && digits[i] % digits[j] == 0) {
                newDigits[newN] = digits[i] / digits[j];
                if (can_reach_target(newDigits, newN + 1, target)) return 1;
            }
        }
    }
    
    return 0;
}

// Generate digits that can reach the target
void generate_solvable_digits(int target, int digits[], int n) {
    // Generate meaningful digits related to the target
    srand(time(NULL));
    
    // First, initialize with some random reasonable digits
    for (int i = 0; i < n; i++) {
        // Choose from possible useful digits (1-10, 25, 50, 75, 100)
        int possibleDigits[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 25, 50, 75, 100};
        int numPossible = sizeof(possibleDigits) / sizeof(possibleDigits[0]);
        
        digits[i] = possibleDigits[rand() % numPossible];
    }
    
    // Ensure at least two digits can combine to get closer to target
    if (target > 100) {
        digits[0] = target / (2 + (rand() % 3));
        digits[1] = target - digits[0];
    } else {
        // For smaller targets, ensure we have a combination of small numbers
        digits[0] = 1 + rand() % 9;
        digits[1] = 1 + rand() % 9;
    }
    
    // Verify our digits can reach the target; if not, try again
    int attempts = 0;
    while (!can_reach_target(digits, n, target) && attempts < 10) {
        // Adjust digits to make it more likely to reach target
        digits[rand() % n] = 1 + rand() % 10;
        attempts++;
    }
}

// Initialize a new round
void init_round(DigitsGame *game, int roundIndex) {
    if (roundIndex < 0 || roundIndex >= MAX_ROUNDS) return;
    
    Round *round = &game->rounds[roundIndex];
    
    // Generate a target based on round difficulty
    int targetMin = 30 + (roundIndex * 50);
    int targetMax = 100 + (roundIndex * 100);
    round->target = targetMin + (rand() % (targetMax - targetMin + 1));
    
    // Set digit count
    round->digitCount = 6;
    
    // Generate digits that can reach the target
    generate_solvable_digits(round->target, round->digits, round->digitCount);
    
    round->completed = 0;
    round->userResult = 0;
    
    // If this is the current round, initialize bubbles
    if (roundIndex == game->currentRound) {
        // Clear existing bubbles
        game->bubbleCount = 0;
        game->selectedOp = OP_NONE;
        game->selectedBubble1 = -1;
        game->selectedBubble2 = -1;
        game->draggingBubble = -1;
        game->gameWon = 0;
        
        // Initialize digit bubbles in a circle at the center of the screen
        float centerX = SCREEN_WIDTH / 2;
        float centerY = SCREEN_HEIGHT / 2 - 30;
        float radius = 150;
        
        for (int i = 0; i < round->digitCount; i++) {
            float angle = i * (2.0f * M_PI / round->digitCount);
            float x = centerX + radius * cos(angle);
            float y = centerY + radius * sin(angle);
            
            init_bubble(game, i, BUBBLE_DIGIT, round->digits[i], x, y);
            game->bubbleCount++;
        }
        
        // Initialize operation bubbles vertically on the left side
        float opY = 150;
        float opSpacing = 100;
        float opX = 80;
        
        // Addition
        init_bubble(game, game->bubbleCount++, BUBBLE_OP, OP_ADD, opX, opY);
        // Subtraction
        init_bubble(game, game->bubbleCount++, BUBBLE_OP, OP_SUB, opX, opY + opSpacing);
        // Multiplication
        init_bubble(game, game->bubbleCount++, BUBBLE_OP, OP_MUL, opX, opY + opSpacing * 2);
        // Division
        init_bubble(game, game->bubbleCount++, BUBBLE_OP, OP_DIV, opX, opY + opSpacing * 3);
        
        // Add navigation buttons
        float btnY = SCREEN_HEIGHT - 50;
        
        // Previous round button
        if (roundIndex > 0) {
            init_bubble(game, game->bubbleCount++, BUBBLE_BUTTON, BTN_PREV_ROUND, 150, btnY);
        }
        
        // Next round button
        if (roundIndex < game->totalRounds - 1) {
            init_bubble(game, game->bubbleCount++, BUBBLE_BUTTON, BTN_NEXT_ROUND, SCREEN_WIDTH - 150, btnY);
        } else {
            // Submit button on last round
            init_bubble(game, game->bubbleCount++, BUBBLE_BUTTON, BTN_SUBMIT, SCREEN_WIDTH - 150, btnY);
        }
    }
}

// Initialize game state
void init_game(DigitsGame *game) {
    // Initialize game state
    game->totalRounds = MAX_ROUNDS;
    game->currentRound = 0;
    game->bubbleCount = 0;
    game->selectedOp = OP_NONE;
    game->selectedBubble1 = -1;
    game->selectedBubble2 = -1;
    game->draggingBubble = -1;
    game->flicking = 0;
    game->flickVelocityX = 0.0f;
    game->flickVelocityY = 0.0f;
    game->lastTouchX = 0.0f;
    game->lastTouchY = 0.0f;
    game->gameCompleted = 0;
    game->touchTimer = 0.0f;
    game->lastTouchTime = 0.0f;
    game->gameWon = 0;
    
    // Initialize colors
    init_colors(game);
    
    // Seed random
    srand(time(NULL));
    
    // Initialize all rounds
    for (int i = 0; i < game->totalRounds; i++) {
        game->rounds[i].completed = 0;
        game->rounds[i].userResult = 0;
        init_round(game, i);
    }
}

// Check if a point is inside a bubble
int is_point_in_bubble(float px, float py, Bubble *bubble) {
    float distance = sqrtf(powf(bubble->x - px, 2) + powf(bubble->y - py, 2));
    float radius = (bubble->type == BUBBLE_OP) ? OP_BUBBLE_RADIUS : BUBBLE_RADIUS;
    return distance < radius * bubble->scale;
}

// Apply an operation between two bubbles and create a result bubble
void apply_operation(DigitsGame *game, int bubble1Index, int bubble2Index) {
    Bubble *b1 = &game->bubbles[bubble1Index];
    Bubble *b2 = &game->bubbles[bubble2Index];
    int result = 0;
    int valid = 0;
    
    // Calculate result based on the selected operation
    switch (game->selectedOp) {
        case OP_ADD:
            result = b1->value + b2->value;
            valid = 1;
            break;
        case OP_SUB:
            result = b1->value - b2->value;
            valid = (result > 0);  // Only allow positive results
            break;
        case OP_MUL:
            result = b1->value * b2->value;
            valid = 1;
            break;
        case OP_DIV:
            // Only allow division if it results in a whole number
            if (b2->value != 0 && b1->value % b2->value == 0) {
                result = b1->value / b2->value;
                valid = 1;
            }
            break;
    }
    
    if (valid) {
        // Deactivate the used bubbles
        b1->active = 0;
        b2->active = 0;
        
        // Create a new result bubble at the midpoint between the two bubbles
        float newX = (b1->x + b2->x) / 2.0f;
        float newY = (b1->y + b2->y) / 2.0f;
        
        // Find an available slot for the new bubble
        int newIndex = -1;
        for (int i = 0; i < MAX_BUBBLES; i++) {
            if (!game->bubbles[i].active) {
                newIndex = i;
                break;
            }
        }
        
        // If no slots available, create a new one
        if (newIndex == -1 && game->bubbleCount < MAX_BUBBLES - 1) {
            newIndex = game->bubbleCount;
            game->bubbleCount++;
        }
        
        if (newIndex != -1) {
            init_bubble(game, newIndex, BUBBLE_RESULT, result, newX, newY);
            
            // Check if we've reached the target
            if (result == game->rounds[game->currentRound].target) {
                game->gameWon = 1;
                game->rounds[game->currentRound].completed = 1;
                game->rounds[game->currentRound].userResult = result;
            }
        }
    }
    
    // Reset selection state
    game->selectedBubble1 = -1;
    game->selectedBubble2 = -1;
}

// Handle going to the next round
void go_to_next_round(DigitsGame *game) {
    // Store current round's result
    int activeDigitFound = 0;
    int lastResult = 0;
    
    // Find the last active digit (result)
    for (int i = 0; i < game->bubbleCount; i++) {
        if (game->bubbles[i].active && 
            (game->bubbles[i].type == BUBBLE_DIGIT || 
             game->bubbles[i].type == BUBBLE_RESULT)) {
            activeDigitFound = 1;
            lastResult = game->bubbles[i].value;
        }
    }
    
    // Mark current round as completed
    if (activeDigitFound) {
        game->rounds[game->currentRound].completed = 1;
        game->rounds[game->currentRound].userResult = lastResult;
    }
    
    // Move to next round
    if (game->currentRound < game->totalRounds - 1) {
        game->currentRound++;
        init_round(game, game->currentRound);
    }
}

// Handle going to the previous round
void go_to_prev_round(DigitsGame *game) {
    // Store current round's result
    int activeDigitFound = 0;
    int lastResult = 0;
    
    // Find the last active digit (result)
    for (int i = 0; i < game->bubbleCount; i++) {
        if (game->bubbles[i].active && 
            (game->bubbles[i].type == BUBBLE_DIGIT || 
             game->bubbles[i].type == BUBBLE_RESULT)) {
            activeDigitFound = 1;
            lastResult = game->bubbles[i].value;
        }
    }
    
    // Mark current round as completed
    if (activeDigitFound) {
        game->rounds[game->currentRound].completed = 1;
        game->rounds[game->currentRound].userResult = lastResult;
    }
    
    // Move to previous round
    if (game->currentRound > 0) {
        game->currentRound--;
        init_round(game, game->currentRound);
    }
}

// Submit the final results and calculate score
void submit_game(DigitsGame *game) {
    // Store current round's result
    int activeDigitFound = 0;
    int lastResult = 0;
    
    // Find the last active digit (result)
    for (int i = 0; i < game->bubbleCount; i++) {
        if (game->bubbles[i].active && 
            (game->bubbles[i].type == BUBBLE_DIGIT || 
             game->bubbles[i].type == BUBBLE_RESULT)) {
            activeDigitFound = 1;
            lastResult = game->bubbles[i].value;
        }
    }
    
    // Mark current round as completed
    if (activeDigitFound) {
        game->rounds[game->currentRound].completed = 1;
        game->rounds[game->currentRound].userResult = lastResult;
    }
    
    // Set game completed flag
    game->gameCompleted = 1;
} 