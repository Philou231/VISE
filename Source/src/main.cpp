#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "interface/CommDDE.h"
#include "gui/Gui.h"



//TODO: Think about integrating ICU for perfect unicode handling (https://icu.unicode.org/)
//TODO: Change the Widget::draw function to work with everyone rendering themselves rather than blitting on a surface
//TODO: Add the possiblity to calibrate without a P&P in order to take measurements and add comments
//TODO: Implement the timers event to allow animations capabilities and tooltips to disapear
//TODO: Show a history of changes made
//TODO: Backspace to come back to the previous one (Complete history with Ctrl+Z and Ctrl+Y ?)
//TODO: Create a settings Pop-Up



int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);
    IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG);
    atexit(IMG_Quit);
    TTF_Init();
    atexit(TTF_Quit);


    SDL_Window* window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);  //Uses 80MB of memory
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"best");


    Gui myGUI(window,renderer); //The whole software is encapsulated inside the Gui class
    return myGUI.loop(argc, argv); //This is the main loop of the software
}
