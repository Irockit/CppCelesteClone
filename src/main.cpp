#include "engine_lib.h"
#include "input.h"
#include "game.h"
//#include "game.cpp"
#define APIENTRY
#define GL_GLEXT_PROTOTYPES
#include <glcorearb.h>

static KeyCodeID KeyCodeLookupTable[KEY_COUNT];

#include "platform.h"
#ifdef _WIN32
#include "win32_platform.cpp"
#endif

#include "gl_renderer.cpp"

//#####################################################################################################################################
//                                                  Game DLL Stuff
//#####################################################################################################################################
typedef decltype(update_game) update_game_type;
static update_game_type* update_game_ptr;


//#####################################################################################################################################
//                                                  Cross Platform Functions
//#####################################################################################################################################
void reload_game_dll(BumpAllocator* transientStorage);

int main(){
    BumpAllocator transientStorage = make_bump_allocator(MB(50));
    BumpAllocator persistentStorage = make_bump_allocator(MB(50));

    gameState = (GameState*)bump_alloc(&persistentStorage, sizeof(GameState));
    SM_ASSERT_GUARD(gameState, -1, "Failed to allocate GameState");
    input = (Input*)bump_alloc(&persistentStorage, sizeof(Input));
    SM_ASSERT_GUARD(input, -1, "Failed to allocate Input");
    renderData = (RenderData*)bump_alloc(&persistentStorage, sizeof(RenderData));
    SM_ASSERT_GUARD(renderData, -1, "Failed to allocate RenderData");

    platform_fill_keycode_lookup_table();
    platform_create_window(1280, 640, "Game");

    gl_init(&transientStorage);
    while (running){
        reload_game_dll(&transientStorage);
        platform_update_window();
        update_game(gameState, renderData, input);
        gl_render();
        platform_swap_buffers();

        transientStorage.used = 0;
    }
    return 0;
}

void update_game(GameState* gameStateIn, RenderData* renderDataIn, Input* inputIn){
    update_game_ptr(gameStateIn, renderDataIn, inputIn);
}

void reload_game_dll(BumpAllocator* transientStorage){
    static void* gameDLL;
    static long long lastEditTimestampGameDLL;
    long long currentTimestampGameDLL = get_timestamp("game.dll");
    if(currentTimestampGameDLL > lastEditTimestampGameDLL){
        if(gameDLL){
            bool freeResult = platform_free_dynamic_library(gameDLL);
            SM_ASSERT(freeResult, "Failed to free game.dll");
            gameDLL = nullptr;
            SM_TRACE("Freed game.dll");
        }
        while(!copy_file("game.dll", "game_load.dll", transientStorage)){
            Sleep(10);
        }
        SM_TRACE("Copied game.dll into game_load.dll");

        gameDLL = platform_load_dynamic_library("game_load.dll");
        SM_ASSERT(gameDLL, "Failed to load game.dll");

        update_game_ptr = (update_game_type*)platform_load_dynamic_function(gameDLL, "update_game");
        SM_ASSERT(update_game_ptr, "Failed to load update_game function");
        lastEditTimestampGameDLL = currentTimestampGameDLL;
    }
}



