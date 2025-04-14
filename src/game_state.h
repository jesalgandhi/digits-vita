#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <psp2/touch.h>
#include <psp2/ctrl.h>
#include <vita2d.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#define MAX_BUBBLES 20
#define MAX_ROUNDS 5
#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544
#define BUBBLE_RADIUS 45
#define OP_BUBBLE_RADIUS 40
#define MAX_DIGITS_PER_ROUND 6

// Bubble types
#define BUBBLE_DIGIT 0
#define BUBBLE_RESULT 1
#define BUBBLE_OP 2
#define BUBBLE_BUTTON 3

// Operation types
#define OP_NONE 0
#define OP_ADD 1
#define OP_SUB 2
#define OP_MUL 3
#define OP_DIV 4

// Button types
#define BTN_NEXT_ROUND 0
#define BTN_PREV_ROUND 1
#define BTN_SUBMIT 2

// Bubble structure
typedef struct {
    int type;           // Type of bubble (digit, result, operation, button)
    int value;          // Value for digit/result, operation type for op, button type for button
    float x, y;         // Position
    float dx, dy;       // Velocity for movement
    float friction;     // Friction to slow down movement
    int active;         // Whether bubble is active/visible
    int selected;       // Whether bubble is currently selected
    float scale;        // Scale factor for animation
    int color;          // Color index for visual variety
} Bubble;

// Round structure
typedef struct {
    int target;                         // Target number to reach
    int digits[MAX_DIGITS_PER_ROUND];   // Original digits for this round
    int digitCount;                     // Count of digits for this round
    int completed;                      // Whether round was completed
    int userResult;                     // User's final result
} Round;

// Structure for game state
typedef struct {
    Round rounds[MAX_ROUNDS];           // Game rounds
    int currentRound;                   // Current round index (0-based)
    int totalRounds;                    // Total number of rounds
    Bubble bubbles[MAX_BUBBLES];        // All bubbles in play
    int bubbleCount;                    // Current count of bubbles
    int selectedOp;                     // Currently selected operation
    int selectedBubble1;                // First selected bubble for operation
    int selectedBubble2;                // Second selected bubble for operation
    int draggingBubble;                 // Index of currently dragged bubble
    int flicking;                       // Flag if we're flicking a bubble
    float flickVelocityX, flickVelocityY; // Flick velocity components
    float lastTouchX, lastTouchY;       // Last touch position for flick calculation
    float dragOffsetX, dragOffsetY;     // Offset for dragging
    int gameCompleted;                  // Flag to indicate if all rounds completed
    int colors[6];                      // Array of bubble colors
    int opColor;                        // Color for operation bubbles
    float touchTimer;                   // Timer to detect taps vs drags
    float lastTouchTime;                // Time of last touch
    int gameWon;                        // Flag if current round is won
} DigitsGame;

// Function declarations
void init_colors(DigitsGame *game);
void init_bubble(DigitsGame *game, int index, int type, int value, float x, float y);
void init_game(DigitsGame *game);
void init_round(DigitsGame *game, int roundIndex);
int is_point_in_bubble(float px, float py, Bubble *bubble);
void apply_operation(DigitsGame *game, int bubble1Index, int bubble2Index);
void update_game(DigitsGame *game, SceTouchData touch, SceTouchData prevTouch);
void draw_bubble(DigitsGame *game, Bubble *bubble, vita2d_pvf *pvf, int color);
void render_game(DigitsGame *game, vita2d_pvf *pvf);
int can_reach_target(int digits[], int n, int target);
void generate_solvable_digits(int target, int digits[], int n);

#endif // GAME_STATE_H 