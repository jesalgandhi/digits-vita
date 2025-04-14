#ifndef RENDERING_H
#define RENDERING_H

#include "game_state.h"

// Function declarations for rendering
void draw_bubble(DigitsGame *game, Bubble *bubble, vita2d_pvf *pvf, int color);
void render_game(DigitsGame *game, vita2d_pvf *pvf);
void render_final_score(DigitsGame *game, vita2d_pvf *pvf);
void draw_frutiger_reflections(float x, float y, float radius, int baseColor);

#endif // RENDERING_H 