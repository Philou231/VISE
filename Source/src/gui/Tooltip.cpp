#include "Tooltip.h"

#include "misc/Utilities.h"


using namespace std;


#define PADDING_X 5
#define PADDING_Y 5


Tooltip::Tooltip(SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, Anchorage anchor) :
    Widget(renderer,parent,renderLevel,x,y,1,1),
    m_label(renderer,this,renderLevel,0,0,0,0),
    m_position({x,y}),
    m_anchor(anchor),
    m_delayShown(3000),
    m_lastTimeShown(0)
{
    m_visible = false;
    m_colorBG = Utilities::colorRGBA(0,0,0,SDL_ALPHA_TRANSPARENT);
    m_label.setColor(Utilities::colorRGB(255,255,255),Utilities::colorRGB(0,0,0));
}

void Tooltip::setColor(Uint32 colorFG, Uint32 colorBG)
{
    m_label.setColor(colorFG,colorBG);
}

string Tooltip::getText()
{
    return m_label.getText();
}

void Tooltip::setText(string text, TTF_Font* font/*=NULL*/)
{
    m_label.setText(text, font);

    SDL_Rect textSize = m_label.getTextSize();
    textSize.w += PADDING_X*2;
    textSize.h += PADDING_X*2;

    Tooltip::recomputeAbsRect();

    Widget::needRedraw();
    for(size_t i=0;i<m_children.size();i++)
    {
        m_children[i]->recomputeAbsRect();
    }
}

void Tooltip::setPosition(SDL_Rect position, Anchorage anchor)
{
    m_rectAbs.x += position.x-m_position.x;
    m_rectAbs.y += position.y-m_position.y;

    m_position = position;
    m_anchor = anchor;

    Tooltip::recomputeAbsRect();

    for(size_t i=0;i<m_children.size();i++)
    {
        m_children[i]->recomputeAbsRect();
    }
}

void Tooltip::show()
{
    Widget::show();
    m_lastTimeShown = SDL_GetTicks();
}

void Tooltip::setDelayMs(unsigned long delayShown)
{
    m_delayShown = delayShown;
}

void Tooltip::manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard)
{
    if(m_delayShown!=0 && SDL_GetTicks() > m_lastTimeShown+m_delayShown)
        Widget::hide();
}

void Tooltip::recomputeAbsRect(SDL_Rect* windowRect)
{
    SDL_Rect parentRect = m_parent->getAbsRect();
    SDL_Rect position = m_position;
    if(position.x<0)
        position.x += parentRect.w;
    if(position.y<0)
        position.y += parentRect.h;

    SDL_Rect textSize = m_label.getTextSize();
    textSize.w += PADDING_X*2;
    textSize.h += PADDING_X*2;

    m_rectAbs = Widget::computeAnchorage(position.x, position.y, textSize.w, textSize.h, m_anchor);
}


Tooltip::~Tooltip()
{

}
