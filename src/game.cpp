#include "game.h"
#include "engine_lib.h"
#include "assets.h"
#include "input.h"
#include "render_interface.h"

//#####################################################################################################################################
//                                                  Game Constants
//#####################################################################################################################################
static KeyMap maps[] = {{MOVE_UP,    KEY_W}, {MOVE_UP,    KEY_UP},                     //MoveUp
                        {MOVE_LEFT,  KEY_A}, {MOVE_LEFT,  KEY_LEFT},                   //MoveLeft
                        {MOVE_DOWN,  KEY_S}, {MOVE_DOWN,  KEY_DOWN},                   //MoveDown
                        {MOVE_RIGHT, KEY_D}, {MOVE_RIGHT, KEY_RIGHT},                  //MoveRight
                        {MOUSE_LEFT, KEY_MOUSE_LEFT}, {MOUSE_RIGHT, KEY_MOUSE_RIGHT}}; //MouseClicks
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

Tile* get_tile(int x, int y){
    Tile* tile = nullptr;
    if(x >=0 && x < WORLD_GRID.x && y >=0 && y < WORLD_GRID.y){
        tile = &gameState->worldGrid[x][y];
    }
    return tile;
}

Tile* get_tile(IVec2 worldPos){ return get_tile(worldPos.x / TILESIZE, worldPos.y / TILESIZE);}



int get_neigbour_mask(int x, int y, Tile* tile, int* neighbourOffsets){
    int mask = 0;
    int neighbourCount = 0;
    int extendedNeighbourCount = 0;
    int emptyNeighbourSlot = 0;
    
    for(int n = 0; n < 12; n++){
        Tile* neighbour = get_tile(x+neighbourOffsets[n*2], y + neighbourOffsets[n * 2 + 1]);
        if(!neighbour || neighbour->isVisible){
            mask |= BIT(n);
            if(n < 8) neighbourCount++;
            else extendedNeighbourCount++;
        } else if (n < 8) emptyNeighbourSlot = n;
    }

    if(neighbourCount == 7 && emptyNeighbourSlot >= 4) mask = 16 + (emptyNeighbourSlot - 4);
    else if(neighbourCount == 8 && extendedNeighbourCount == 4) mask = 20;
    else mask &= 0b1111;
    return mask;
}

void set_neigbour_masks(){
    int neighbourOffsets[24] = {  0,-1,   -1, 0,    1, 0,    0, 1,
                                 -1,-1,    1,-1,   -1, 1,    1, 1,
                                  0,-2,   -2, 0,    2, 0,    0, 2};
    for(int y = 0; y < WORLD_GRID.y; y++){
        for(int x = 0; x < WORLD_GRID.x; x++){
            Tile* tile = get_tile(x, y);
            if(!tile->isVisible)continue;
            tile->neigbourMask = get_neigbour_mask(x, y, tile, neighbourOffsets);
            
            Transform transform ={};
            transform.pos = {x * (float)TILESIZE, y * (float)TILESIZE};
            transform.size = {8,8};
            transform.spriteSize = {8, 8};
            transform.atlasOffset = gameState->tileCoords[tile->neigbourMask];
            draw_quad(transform);
        }
    }
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

        gameState->MapKeys(maps, sizeof(maps)/sizeof(maps[0]));
        
        renderData->gameCamera.position = {160, -90};
        {
            IVec2 tilesPosition = {48, 0};
            for(int y = 0; y < 5; y++){
                for(int x = 0; x < 4; x++){
                    gameState->tileCoords.add({tilesPosition.x + x * 8, tilesPosition.y + y * 8});
                }
            }
            gameState->tileCoords.add({tilesPosition.x, tilesPosition.y+5*8});
        }
    }

    if(is_down(MOUSE_LEFT) || is_down(MOUSE_RIGHT)){
        if(is_down(MOUSE_LEFT)){
            Tile* tile = get_tile(input->mousePosWorld);
            if(tile) tile->isVisible = true;
        }
        if(is_down(MOUSE_RIGHT)){
            Tile* tile = get_tile(input->mousePosWorld);
            if(tile) tile->isVisible = false;
        }
        set_neigbour_masks();
    }else{         
        for(int y = 0; y < WORLD_GRID.y; y++){
            for(int x = 0; x < WORLD_GRID.x; x++){
                Tile* tile = get_tile(x, y);
                if(!tile->isVisible) continue;

                Transform transform ={};
                transform.pos = {x * (float)TILESIZE, y * (float)TILESIZE};
                transform.size = {8,8};
                transform.spriteSize = {8, 8};
                transform.atlasOffset = gameState->tileCoords[tile->neigbourMask];
                draw_quad(transform);
            }
        }
    }
    
    draw_sprite(SPRITE_DICE, gameState->playerPos);
    if(is_down(MOVE_LEFT)) gameState->playerPos.x -= 1;
    if(is_down(MOVE_RIGHT)) gameState->playerPos.x += 1;
    if(is_down(MOVE_UP)) gameState->playerPos.y -= 1;
    if(is_down(MOVE_DOWN)) gameState->playerPos.y += 1;

}

