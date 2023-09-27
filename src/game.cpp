#include "game.h"
#include "engine_lib.h"
#include "assets.h"
#include "input.h"
#include "render_interface.h"

//#####################################################################################################################################
//                                                  Game Constants
//#####################################################################################################################################
constexpr int WORLD_WIDTH = 320;
constexpr int WORLD_HEIGHT = 180;
constexpr int TILESIZE = 8;
//#####################################################################################################################################
//                                                  Game Structs
//#####################################################################################################################################
//#####################################################################################################################################
//                                                  Game Functions
//#####################################################################################################################################
bool just_pressed(GameInputType type){
    KeyMapping mapping = gameState->keyMappings[type];
    for(int idx = 0; idx < mapping.keys.count; idx++){
        if(input->keys[mapping.keys[idx]].justPressed) { 
            return true;
        }
    }
    return false;
}

bool is_down(GameInputType type){
    KeyMapping mapping = gameState->keyMappings[type];
    for(int idx = 0; idx < mapping.keys.count; idx++){
        if(input->keys[mapping.keys[idx]].isDown) { 
            return true;
        }
    }
    return false;
}
//#####################################################################################################################################
//                                                  Game Functions(Exposed)
//#####################################################################################################################################
EXPORT_FN void update_game(GameState* gameStateIn, RenderData* renderDataIn, Input* inputIn){
    if(renderData != renderDataIn){
        gameState = gameStateIn;
        input = inputIn;
        renderData = renderDataIn;
    }

    if(!gameState->initialized){
        renderData->gameCamera.dimensions = {WORLD_WIDTH, WORLD_HEIGHT};
        gameState->initialized = true;

        {
            gameState->keyMappings[MOVE_UP].keys.add(KEY_W);
            gameState->keyMappings[MOVE_LEFT].keys.add(KEY_A);
            gameState->keyMappings[MOVE_DOWN].keys.add(KEY_S);
            gameState->keyMappings[MOVE_RIGHT].keys.add(KEY_D);
            gameState->keyMappings[MOVE_UP].keys.add(KEY_UP);
            gameState->keyMappings[MOVE_LEFT].keys.add(KEY_LEFT);
            gameState->keyMappings[MOVE_DOWN].keys.add(KEY_DOWN);
            gameState->keyMappings[MOVE_RIGHT].keys.add(KEY_RIGHT);
        }
    }

    
    draw_sprite(SPRITE_DICE, gameState->playerPos);
    
    if(is_down(MOVE_LEFT)) gameState->playerPos.x -= 1;
    if(is_down(MOVE_RIGHT)) gameState->playerPos.x += 1;
    if(is_down(MOVE_UP)) gameState->playerPos.y -= 1;
    if(is_down(MOVE_DOWN)) gameState->playerPos.y += 1;

}

