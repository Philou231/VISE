#include "Button.h"

#include <SDL_image.h>
#include "misc/Utilities.h"


using namespace std;


Button::Button(SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, int w, int h) :
    Widget(renderer,parent,renderLevel,x,y,w,h),
    m_callbackFunction(NULL),
    m_callbackObjPtr(NULL),
    m_callbackArguments(NULL),
    m_label(renderer,this,renderLevel,0,0,0,0),
    m_labelPressed(renderer,this,renderLevel,0,0,0,0),
    m_clickedIn(false),
    m_pressable(false),
    m_labelPressedInitialized(false),
    m_pressedIn(false)
{
    m_labelPressed.visible(false);

    m_colorBGIdle = Utilities::colorRGB(50,50,50);
    m_colorBGOver = Utilities::colorRGB(50,0,0);
    m_colorBGPressed = Utilities::colorRGB(100,0,0);
    m_colorBGPressedInIdle = Utilities::colorRGB(200,0,0);
    m_colorBGPressedInIdleOver = Utilities::colorRGB(150,0,0);
    m_colorText = Utilities::colorRGB(255,255,255);
    m_colorTextPressed = m_colorText;

    m_colorBG = m_colorBGIdle;
    SDL_FillRect(m_surface,NULL,m_colorBG);
}

void Button::setIcon(string path)
{
    m_label.setImage(path);
}

void Button::setIconPressed(string path)
{
    m_labelPressedInitialized=true;
    m_labelPressed.setImage(path);
}

void Button::setIconColorMod(Uint8 r, Uint8 g, Uint8 b)
{
    m_label.setImageColorMod(r,g,b);
}

void Button::setIconPressedColorMod(Uint8 r, Uint8 g, Uint8 b)
{
    m_labelPressedInitialized=true;
    m_labelPressed.setImageColorMod(r,g,b);
}

string Button::getText()
{
    return m_label.getText();
}

void Button::setText(string text, TTF_Font* font/*=NULL*/)
{
    m_label.setText(text, font);
}

void Button::setTextPressed(string text, TTF_Font* font/*=NULL*/)
{
    m_labelPressedInitialized=true;
    m_labelPressed.setText(text, font);
}

void Button::setTextColor(Uint8 r, Uint8 g, Uint8 b)
{
    m_colorText = Utilities::colorRGB(r,g,b);
    m_label.setColor(m_colorText, m_colorBG);
}

void Button::setTextColorPressed(Uint8 r, Uint8 g, Uint8 b)
{
    m_labelPressedInitialized=true;

    m_colorTextPressed = Utilities::colorRGB(r,g,b);
    m_labelPressed.setColor(m_colorTextPressed, m_colorBG);
}

void Button::setColorBGIdle(Uint8 r, Uint8 g, Uint8 b)
{
    m_colorBGIdle = Utilities::colorRGB(r,g,b);
}

void Button::setColorBGOver(Uint8 r, Uint8 g, Uint8 b)
{
    m_colorBGOver = Utilities::colorRGB(r,g,b);
}

void Button::setColorBGPressed(Uint8 r, Uint8 g, Uint8 b)
{
    m_colorBGPressed = Utilities::colorRGB(r,g,b);
}

void Button::setColorBGPressedInIdle(Uint8 r, Uint8 g, Uint8 b)
{
    m_colorBGPressedInIdle = Utilities::colorRGB(r,g,b);
}

void Button::setColorBGPressedInOver(Uint8 r, Uint8 g, Uint8 b)
{
    m_colorBGPressedInIdleOver = Utilities::colorRGB(r,g,b);
}

void Button::recomposite()
{
    if(m_pressedIn)
    {
        if(m_colorBG==m_colorBGIdle)
            m_colorBG = m_colorBGPressedInIdle;
        else if(m_colorBG==m_colorBGOver)
            m_colorBG = m_colorBGPressedInIdleOver;

        if(m_labelPressedInitialized)
        {
            m_label.visible(false);
            m_labelPressed.visible(true);

            m_labelPressed.setColor(m_colorText, m_colorBG);
        }
        else
        {
            m_label.setColor(m_colorText, m_colorBG);
        }
    }
    else
    {
        if(m_colorBG==m_colorBGPressedInIdle)
            m_colorBG = m_colorBGIdle;
        else if(m_colorBG==m_colorBGPressedInIdleOver)
            m_colorBG = m_colorBGOver;

        m_label.visible(true);
        m_labelPressed.visible(false);

        m_label.setColor(m_colorText, m_colorBG);
    }

    SDL_FillRect(m_surface,NULL,m_colorBG);

    Widget::needGeneralDraw();
}

void Button::pressable(bool isPressable/*=true*/)
{
    m_pressable=isPressable;
}

void Button::toggle()
{
    Button::toggleNoCallback();
    Button::executeAction();
}

void Button::toggleNoCallback()
{
    m_pressedIn=!m_pressedIn;
    Widget::needRedraw();
}

void Button::press()
{
    m_pressedIn=true;
    Widget::needRedraw();
}

void Button::depress()
{
    m_pressedIn=false;
    Widget::needRedraw();
}

bool Button::pressed()
{
    return m_pressedIn;
}

void Button::redraw()
{
    Widget::redraw(); //Changes size if needed
    Button::recomposite(); //Redraws the button
}

void Button::setCallback(std::function<int (void*,void*)>* callbackFunction, void* objectPointer, void* arguments)
{
    m_callbackFunction = callbackFunction;
    m_callbackObjPtr = objectPointer;
    m_callbackArguments = arguments;
}

void Button::manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard)
{
    switch(event.type)
    {
        case SDL_MOUSEMOTION:
            if(cursorIn(mouseAndKeyboard.mousePos))
            {
                if(m_clickedIn)
                {
                    if(m_colorBG!=m_colorBGPressed)
                    {
                        m_colorBG = m_colorBGPressed;
                        Widget::needRedraw();
                    }
                }
                else
                {
                    if((m_pressedIn && m_colorBG!=m_colorBGPressedInIdleOver) || (!m_pressedIn && m_colorBG!=m_colorBGOver))
                    {
                        m_colorBG = m_colorBGOver;
                        Widget::needRedraw();
                    }
                }
            }
            else
            {
                if((m_pressedIn && m_colorBG!=m_colorBGPressedInIdle) || (!m_pressedIn && m_colorBG!=m_colorBGIdle))
                {
                    m_colorBG = m_colorBGIdle;
                    Widget::needRedraw();
                }
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            if(cursorIn(mouseAndKeyboard.mousePos))
            {
                m_clickedIn=true;
                m_colorBG = m_colorBGPressed;
                Widget::needRedraw();
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if(cursorIn(mouseAndKeyboard.mousePos))
            {
                if(m_clickedIn)
                {
                    if(m_pressable)
                        Button::toggle();
                    else
                        Button::executeAction();
                }

                m_colorBG = m_colorBGOver;
                Widget::needRedraw();
            }
            m_clickedIn=false;
            break;
    }

    Widget::manageEvent(event, mouseAndKeyboard);
}

int Button::executeAction()
{
    if(m_callbackFunction!=NULL)
        return (*m_callbackFunction)(m_callbackObjPtr,m_callbackArguments);
    return -1;
}

Button::~Button()
{

}
