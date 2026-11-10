#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "Game.hpp"
#include <string>

namespace { int smokeFrames{}; }
SDL_AppResult SDL_AppInit(void** state,int argc,char** argv){
  for(int i=1;i<argc;i++)if(std::string(argv[i])=="--smoke-test")smokeFrames=120;
  auto* game=new Game;std::string error;if(!game->init(error)){SDL_Log("SEWERLINE: %s",error.c_str());delete game;return SDL_APP_FAILURE;}*state=game;if(smokeFrames){SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.key=SDLK_RETURN;e.key.scancode=SDL_SCANCODE_RETURN;game->event(e);}return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppEvent(void* state,SDL_Event* event){return static_cast<Game*>(state)->event(*event)?SDL_APP_CONTINUE:SDL_APP_SUCCESS;}
SDL_AppResult SDL_AppIterate(void* state){if(!static_cast<Game*>(state)->tick())return SDL_APP_SUCCESS;if(smokeFrames&&!--smokeFrames)return SDL_APP_SUCCESS;return SDL_APP_CONTINUE;}
void SDL_AppQuit(void* state,SDL_AppResult){if(state){static_cast<Game*>(state)->shutdown();delete static_cast<Game*>(state);}}
