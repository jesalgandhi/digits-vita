#include "rendering.h"
#include <stdio.h>

// Draw enhanced Frutiger Aero style reflections on a bubble
void draw_frutiger_reflections(float x, float y, float radius, int baseColor) {
    unsigned char r = (baseColor >> 24) & 0xFF;
    unsigned char g = (baseColor >> 16) & 0xFF;
    unsigned char b = (baseColor >> 8) & 0xFF;
    unsigned char a = baseColor & 0xFF;
    
    // Primary highlight - top-left quadrant (brighter)
    vita2d_draw_fill_circle(
        x - radius * 0.3f, 
        y - radius * 0.3f, 
        radius * 0.5f, 
        RGBA8(255, 255, 255, 80)
    );
    
    // Secondary highlight - small bright spot
    vita2d_draw_fill_circle(
        x - radius * 0.15f, 
        y - radius * 0.15f, 
        radius * 0.2f, 
        RGBA8(255, 255, 255, 120)
    );
    
    // Tertiary highlight - tiny bright spot
    vita2d_draw_fill_circle(
        x - radius * 0.25f, 
        y - radius * 0.25f, 
        radius * 0.06f, 
        RGBA8(255, 255, 255, 180)
    );
    
    // Bottom-right shadow
    vita2d_draw_fill_circle(
        x + radius * 0.4f, 
        y + radius * 0.4f, 
        radius * 0.4f, 
        RGBA8(0, 0, 0, 20)
    );
}

// Draw a selection ring around a bubble (Frutiger Aero style)
void draw_selection_ring(float x, float y, float radius) {
    // Outer glow with gradient
    for (int i = 5; i > 0; i--) {
        float alpha = 40 - i * 8;
        if (alpha < 0) alpha = 0;
        
        vita2d_draw_fill_circle(
            x, y, radius + i,
            RGBA8(255, 255, 255, (unsigned char)alpha)
        );
    }
    
    // Inner bright ring
    vita2d_draw_fill_circle(
        x, y, radius + 2.5f,
        RGBA8(255, 255, 255, 160)
    );
}

// Draw a bubble with enhanced Frutiger Aero glass effect
void draw_bubble(DigitsGame *game, Bubble *bubble, vita2d_pvf *pvf, int color) {
    if (!bubble->active) return;
    
    float radius = (bubble->type == BUBBLE_OP) ? OP_BUBBLE_RADIUS : BUBBLE_RADIUS;
    radius *= bubble->scale;
    
    // Select correct color based on bubble type
    int bubbleColor;
    if (bubble->type == BUBBLE_OP) {
        bubbleColor = game->opColor;
    } else if (bubble->type == BUBBLE_BUTTON) {
        // Button colors
        switch (bubble->value) {
            case BTN_NEXT_ROUND:
                bubbleColor = RGBA8(100, 220, 100, 220);  // Green
                break;
            case BTN_PREV_ROUND:
                bubbleColor = RGBA8(220, 180, 100, 220);  // Amber
                break;
            case BTN_SUBMIT:
                bubbleColor = RGBA8(100, 180, 255, 220);  // Blue
                break;
            default:
                bubbleColor = RGBA8(200, 200, 200, 220);  // Gray
        }
    } else {
        bubbleColor = game->colors[bubble->color];
    }
    
    // Draw selection ring if this bubble is selected
    if (bubble->selected) {
        draw_selection_ring(bubble->x, bubble->y, radius);
    }
    
    // Draw bubble background (glass effect)
    vita2d_draw_fill_circle(bubble->x, bubble->y, radius, bubbleColor);
    
    // Draw Frutiger Aero style reflections
    draw_frutiger_reflections(bubble->x, bubble->y, radius, bubbleColor);
    
    // Draw bubble content
    char buffer[16];
    
    switch (bubble->type) {
        case BUBBLE_DIGIT:
        case BUBBLE_RESULT:
            sprintf(buffer, "%d", bubble->value);
            vita2d_pvf_draw_text(pvf, 
                                bubble->x - vita2d_pvf_text_width(pvf, 1.2f, buffer) / 2.0f, 
                                bubble->y + vita2d_pvf_text_height(pvf, 1.2f, buffer) / 2.0f, 
                                RGBA8(0, 0, 0, 255), 1.2f, buffer);
            break;
            
        case BUBBLE_OP:
            switch (bubble->value) {
                case OP_ADD:
                    vita2d_pvf_draw_text(pvf, bubble->x - 10, bubble->y + 15, RGBA8(0, 0, 0, 255), 1.5f, "+");
                    break;
                case OP_SUB:
                    vita2d_pvf_draw_text(pvf, bubble->x - 10, bubble->y + 15, RGBA8(0, 0, 0, 255), 1.5f, "-");
                    break;
                case OP_MUL:
                    vita2d_pvf_draw_text(pvf, bubble->x - 10, bubble->y + 15, RGBA8(0, 0, 0, 255), 1.5f, "×");
                    break;
                case OP_DIV:
                    vita2d_pvf_draw_text(pvf, bubble->x - 10, bubble->y + 15, RGBA8(0, 0, 0, 255), 1.5f, "÷");
                    break;
            }
            break;
            
        case BUBBLE_BUTTON:
            switch (bubble->value) {
                case BTN_NEXT_ROUND:
                    vita2d_pvf_draw_text(pvf, bubble->x - 35, bubble->y + 15, RGBA8(0, 0, 0, 255), 1.0f, "Next Round");
                    break;
                case BTN_PREV_ROUND:
                    vita2d_pvf_draw_text(pvf, bubble->x - 40, bubble->y + 15, RGBA8(0, 0, 0, 255), 1.0f, "Prev Round");
                    break;
                case BTN_SUBMIT:
                    vita2d_pvf_draw_text(pvf, bubble->x - 30, bubble->y + 15, RGBA8(0, 0, 0, 255), 1.0f, "Submit");
                    break;
            }
            break;
    }
}

// Calculate score based on how close user got to targets
int calculate_final_score(DigitsGame *game) {
    int totalScore = 0;
    int maxPossibleScore = 1000;
    
    for (int i = 0; i < game->totalRounds; i++) {
        Round *round = &game->rounds[i];
        
        if (round->completed) {
            int difference = abs(round->target - round->userResult);
            int roundScore = 0;
            
            // Perfect match
            if (difference == 0) {
                roundScore = 200;
            } 
            // Close
            else if (difference <= 5) {
                roundScore = 150;
            }
            // Pretty good
            else if (difference <= 20) {
                roundScore = 100;
            }
            // Decent
            else if (difference <= 50) {
                roundScore = 50;
            }
            // Way off
            else {
                roundScore = 10;
            }
            
            totalScore += roundScore;
        }
    }
    
    return totalScore;
}

// Render final game score screen
void render_final_score(DigitsGame *game, vita2d_pvf *pvf) {
    char buffer[128];
    float centerX = SCREEN_WIDTH / 2;
    
    // Draw semi-transparent overlay
    vita2d_draw_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, RGBA8(0, 0, 40, 200));
    
    // Draw title
    vita2d_pvf_draw_text(pvf, centerX - 140, 100, RGBA8(255, 255, 100, 255), 2.0f, "FINAL SCORE");
    
    // Calculate and display score
    int finalScore = calculate_final_score(game);
    sprintf(buffer, "%d points", finalScore);
    vita2d_pvf_draw_text(pvf, centerX - 100, 180, RGBA8(255, 255, 255, 255), 2.0f, buffer);
    
    // Draw round results
    vita2d_pvf_draw_text(pvf, centerX - 200, 240, RGBA8(200, 200, 255, 255), 1.3f, "Round Results:");
    
    for (int i = 0; i < game->totalRounds; i++) {
        Round *round = &game->rounds[i];
        
        if (round->completed) {
            sprintf(buffer, "Round %d: Target %d, Your Result %d", 
                    i+1, round->target, round->userResult);
        } else {
            sprintf(buffer, "Round %d: Target %d, Not Completed", 
                    i+1, round->target);
        }
        
        vita2d_pvf_draw_text(pvf, centerX - 220, 280 + (i * 40), 
                          RGBA8(255, 255, 255, 255), 1.0f, buffer);
    }
    
    // Draw restart instruction
    vita2d_pvf_draw_text(pvf, centerX - 180, SCREEN_HEIGHT - 50, 
                      RGBA8(200, 200, 200, 255), 1.0f, "Press START to play again");
}

// Render the game state on-screen
void render_game(DigitsGame *game, vita2d_pvf *pvf) {
    char buffer[64];
    
    // If game is completed, render the final score screen instead
    if (game->gameCompleted) {
        render_final_score(game, pvf);
        return;
    }
    
    // Draw background gradient (Frutiger Aero-style)
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        float t = (float)y / SCREEN_HEIGHT;
        int color = RGBA8(
            20 + (int)(20 * t), 
            30 + (int)(40 * t), 
            50 + (int)(100 * t), 
            255
        );
        vita2d_draw_line(0, y, SCREEN_WIDTH, y, color);
    }
    
    // Draw light streaks (Aero effect) - more subtle and varied
    for (int i = 0; i < 8; i++) {
        float x = 100 + i * 120;
        float opacity = 5 + (i % 3) * 5;
        float width = 150 + (i % 5) * 50;
        
        vita2d_draw_line(x, 0, x + width, SCREEN_HEIGHT, 
                      RGBA8(255, 255, 255, (unsigned char)opacity));
    }
    
    // Draw game title
    vita2d_pvf_draw_text(pvf, 20, 40, RGBA8(255, 255, 255, 255), 1.8f, "DIGITS");
    
    // Draw current round info
    sprintf(buffer, "Round %d/%d", game->currentRound + 1, game->totalRounds);
    vita2d_pvf_draw_text(pvf, SCREEN_WIDTH/2 - 50, 40, 
                      RGBA8(200, 200, 255, 255), 1.3f, buffer);
    
    // Draw target number
    sprintf(buffer, "Target: %d", game->rounds[game->currentRound].target);
    vita2d_pvf_draw_text(pvf, SCREEN_WIDTH - 200, 40, 
                      RGBA8(255, 255, 255, 255), 1.3f, buffer);
    
    // Draw instructions
    if (!game->gameWon) {
        vita2d_pvf_draw_text(pvf, 20, SCREEN_HEIGHT - 20, 
                          RGBA8(200, 200, 200, 200), 0.7f, 
                          "Combine numbers with operations to reach the target. Flick to move. Press START to reset.");
    }
    
    // Draw bubbles from back to front
    for (int i = 0; i < game->bubbleCount; i++) {
        if (i != game->draggingBubble && game->bubbles[i].active) {
            int color = (game->bubbles[i].type == BUBBLE_OP) ? 
                      game->opColor : game->colors[game->bubbles[i].color];
            draw_bubble(game, &game->bubbles[i], pvf, color);
        }
    }
    
    // Draw dragged bubble last (on top)
    if (game->draggingBubble >= 0) {
        int color = (game->bubbles[game->draggingBubble].type == BUBBLE_OP) ? 
                  game->opColor : game->colors[game->bubbles[game->draggingBubble].color];
        draw_bubble(game, &game->bubbles[game->draggingBubble], pvf, color);
    }
    
    // Draw win message if target reached
    if (game->gameWon) {
        // Draw semi-transparent overlay
        vita2d_draw_rectangle(0, SCREEN_HEIGHT/2 - 50, SCREEN_WIDTH, 100, 
                           RGBA8(0, 0, 0, 180));
        
        // Draw congratulation message
        vita2d_pvf_draw_text(pvf, SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 + 15, 
                          RGBA8(255, 255, 100, 255), 1.5f, "TARGET REACHED!");
        
        // Draw instruction to continue
        vita2d_pvf_draw_text(pvf, SCREEN_WIDTH/2 - 220, SCREEN_HEIGHT/2 + 50, 
                          RGBA8(200, 200, 200, 255), 0.8f, 
                          "Press Next Round or use all remaining numbers");
    }
} 