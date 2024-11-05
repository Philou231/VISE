#include "WidgetBar.h"

#include "misc/Utilities.h"



WidgetBar::WidgetBar(SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, int w, int h) :
    Widget(renderer,parent,renderLevel,x,y,w,h)
{
    m_colorBG = Utilities::colorRGB(127,127,127);
    SDL_FillRect(m_surface,NULL,m_colorBG);
}

WidgetBar::~WidgetBar()
{

}
