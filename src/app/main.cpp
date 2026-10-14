// Vera - 3D pizza-delivery driving game.
// Build: see Makefile.macos / Makefile
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "VeraView.h"

#include <iostream>

using namespace std;

void mainLoop(CVeraView *Vera) {
  Vera->OnDraw();

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    SDL_Point point;
    if (event.type == SDL_KEYDOWN) {
      Vera->OnKeyDown(event.key.keysym.sym, 0, 0);
      if (event.key.keysym.sym == SDLK_RETURN ||
          event.key.keysym.sym == SDLK_BACKSPACE) {
        Vera->OnChar(event.key.keysym.sym, 0, 0);
      }
    } else if (event.type == SDL_KEYUP) {
      Vera->OnKeyUp(event.key.keysym.sym, 0, 0);
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
      if (event.button.button == 1) {
        point.x = event.motion.x;
        point.y = event.motion.y;
        Vera->OnLButtonDown(point);
      }
    } else if (event.type == SDL_MOUSEBUTTONUP) {
      if (event.button.button == 1) {
        point.x = event.motion.x;
        point.y = event.motion.y;
        Vera->OnLButtonUp(point);
      }
    } else if (event.type == SDL_MOUSEMOTION) {
      point.x = event.motion.x;
      point.y = event.motion.y;
      Vera->OnMouseMove(point);
    } else if (event.type == SDL_FINGERDOWN) {
      Vera->OnFingerDown(event.tfinger.x, event.tfinger.y,
                         event.tfinger.fingerId);
    } else if (event.type == SDL_FINGERUP) {
      Vera->OnFingerUp(event.tfinger.x, event.tfinger.y,
                       event.tfinger.fingerId);
    } else if (event.type == SDL_QUIT) {
      Vera->setExit();
    } else if (event.type == SDL_TEXTINPUT) {
      // FIXME handle full text
      Vera->OnChar((int)event.text.text[0], 0, 0);
    }
  }
}

int main(int argc, char **argv) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    cout << "SDL init failed" << endl;
  }

  if (TTF_Init() == -1) {
    cout << "TTF_Init: " << TTF_GetError() << endl;
    return 1;
  }

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

  SDL_Window *window =
      SDL_CreateWindow("Vera", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                       0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
  if (window == NULL) {
    cout << "error creating window" << endl;
  }
  SDL_GLContext context = SDL_GL_CreateContext(window);

  CVeraView Vera;
  Vera.setWindow(window);
  SDL_GetWindowSize(window, &Vera.window_width, &Vera.window_height);
  Vera.OnCreate();

  SDL_GL_SetSwapInterval(0);
  while (!Vera.isExit()) {
    mainLoop(&Vera);
  }
  TTF_Quit();
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
