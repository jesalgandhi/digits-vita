#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/display.h>

#include <string.h>

#include <vita2d.h>

#include "game_state.h"
#include "rendering.h"
#include "input.h"

int main(void) {
    DigitsGame game;
    SceTouchData touch, prevTouch;
    SceCtrlData pad, prevPad;
    vita2d_pvf *pvf;

    // Initialize PS Vita systems
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITAL);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
    
    vita2d_init();
    vita2d_set_clear_color(RGBA8(20, 30, 50, 255));

    pvf = vita2d_load_default_pvf();

    // Initialize game state
    init_game(&game);
    
    // Clear input state
    memset(&prevTouch, 0, sizeof(SceTouchData));
    memset(&prevPad, 0, sizeof(SceCtrlData));

    while (1) {
        // Update touch input
        memcpy(&prevTouch, &touch, sizeof(SceTouchData));
        sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);
        
        // Update pad input
        memcpy(&prevPad, &pad, sizeof(SceCtrlData));
        sceCtrlPeekBufferPositive(0, &pad, 1);
        
        // Check for exit condition
        if ((pad.buttons & SCE_CTRL_START) && (pad.buttons & SCE_CTRL_SELECT)) break;

        // Update and render game
        vita2d_start_drawing();
        vita2d_clear_screen();

        // Update game state
        update_game(&game, touch, prevTouch, pad);
        
        // Render the game
        render_game(&game, pvf);

        vita2d_end_drawing();
        vita2d_swap_buffers();
        
        // Maintain frame rate
        sceDisplayWaitVblankStart();
    }
    
    vita2d_fini();
    vita2d_free_pvf(pvf);

    sceKernelExitProcess(0);
    return 0;
}
