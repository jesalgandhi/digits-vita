#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>

#include <vita2d.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Structure for game state
typedef struct {
    int target;
    int digits[6];
} DigitsGame;

// Initialize game state (set a target and generate some random digits)
void init_game(DigitsGame *game) {
    // For demonstration, set a fixed target
    game->target = 100;
    srand(time(NULL));
    for (int i = 0; i < 6; i++) {
         game->digits[i] = (rand() % 9) + 1;  // random digit from 1 to 9
    }
}

// Update game state based on input (for now, pressing CROSS resets the game)
void update_game(DigitsGame *game, SceCtrlData ctrl) {
    if (ctrl.buttons & SCE_CTRL_CROSS) {  // CROSS button to reset game
         init_game(game);
    }
}

// Render the game state on-screen
void render_game(DigitsGame *game, vita2d_pvf *pvf) {
    char buffer[64];

    // Draw target number
    sprintf(buffer, "Target: %d", game->target);
    vita2d_pvf_draw_text(pvf, 20, 50, RGBA8(255, 255, 255, 255), 1.0f, buffer);

    // Draw digits available
    for (int i = 0; i < 6; i++) {
        sprintf(buffer, "Digit %d: %d", i+1, game->digits[i]);
        vita2d_pvf_draw_text(pvf, 20, 100 + (i * 30), RGBA8(200, 200, 200, 255), 1.0f, buffer);
    }
}

int main(void) {
    DigitsGame game;
    SceCtrlData ctrl;
    vita2d_pvf *pvf;

    vita2d_init();

    pvf = vita2d_load_default_pvf();

    init_game(&game);

    while (1) {
        sceCtrlPeekBufferPositive(0, &ctrl, 1);

        if (ctrl.buttons & SCE_CTRL_START) break;


        vita2d_start_drawing();
        vita2d_clear_screen();

        update_game(&game, ctrl);
        render_game(&game, pvf);

        vita2d_end_drawing();
        vita2d_swap_buffers();
    }
    
    vita2d_fini();
    vita2d_free_pvf(pvf);

    sceKernelExitProcess(0);
    return 0;
}
