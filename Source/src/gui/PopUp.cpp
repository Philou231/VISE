#include "PopUp.h"
#include <iostream>
#include "../misc/Utilities.h"

using namespace std;


#define BUTTON_W 80
#define BUTTON_H 40


PopUp::PopUp(SDL_Window* windowParent, SDL_Rect rect, Anchorage anchor, bool modal, string title) :
    m_parentWindow(windowParent),
    m_window(SDL_CreateWindow(title.c_str(), 0, 0, rect.w, rect.h, SDL_WINDOW_HIDDEN)),
    m_renderer(SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED)),
    m_callbackButtonOk(NULL),
    m_rect(rect),
    m_anchor(anchor),
    m_modal(modal),
    m_beingPrompted(false),
    m_status(POPUP_IDLE),
    m_rootWidget(m_renderer,NULL,0,0,0,rect.w,rect.h),
    m_label(m_renderer,&m_rootWidget,0,0,0,rect.w,rect.h/2),
    m_buttonOk(m_renderer,&m_rootWidget,0,(rect.w-BUTTON_W)/2,rect.h*3/4-BUTTON_H/2,BUTTON_W,BUTTON_H)
{
    int parentWindowX, parentWindowY;
    SDL_GetWindowPosition(m_parentWindow, &parentWindowX, &parentWindowY);
    SDL_SetWindowPosition(m_window, parentWindowX+m_rect.x, parentWindowY+m_rect.y);
    recomputePosAndSize();

    m_callbackButtonOk = new function<int (void*,void*)> ([&](void* objPtr, void* arguments)
    {
        PopUp* popUpPtr = (PopUp*)objPtr;
        popUpPtr->m_status=POPUP_OK;
        return 0;
    });
    m_buttonOk.setCallback(m_callbackButtonOk,this,NULL);
    m_buttonOk.setText("Ok");


    m_cursorArrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
}

void PopUp::setTextAlignment(Alignment alignment)
{
    m_label.setAlignment(alignment);
}

void PopUp::setTextPadding(int padding)
{
    m_label.setPadding(padding);
}

void PopUp::setText(string message, TTF_Font* font/*=NULL*/)
{
    m_label.setText(message, font);
    m_buttonOk.setText(m_buttonOk.getText(),font);
}

void PopUp::setTitle(string title)
{
    SDL_SetWindowTitle(m_window, title.c_str());
}

void PopUp::setButtonOkText(string text, TTF_Font* font/*=NULL*/)
{
    m_buttonOk.setText(text, font);
}

void PopUp::setButtonOkSpacingHeight(int height)
{
    SDL_Rect labelRect = m_label.getRect();
    labelRect.h = m_rect.h - height;
    m_label.setRect(labelRect);

    SDL_Rect buttonRect = m_buttonOk.getRect();
    buttonRect.y = -height/2-BUTTON_H/2;
    m_buttonOk.setRect(buttonRect);
}

void PopUp::setPositionAndSize(SDL_Rect rect, Anchorage anchor)
{
    m_rect = rect;
    m_anchor = anchor;

    PopUp::recomputePosAndSize();
}

PopUpStatus PopUp::prompt() //Show the window and take focus
{
    if(m_modal) //Start the loop in a thread to allow the main program to proceed
    {
        return PopUp::promptLoop();
    }
    else
    {
        if(m_beingPrompted) //Do not start another thread
        {
            SDL_RaiseWindow(m_window);
            return POPUP_IDLE;
        }

        SDL_Thread* popUpThread = SDL_CreateThread(PopUp::promptLoopCaller,"Prompt Loop",this);
        SDL_DetachThread(popUpThread);

        return POPUP_IDLE;
    }
}

int PopUp::promptLoopCaller(void* popUpVoidPointer)
{
    if(popUpVoidPointer==NULL)
        return -1;

    PopUp* popUpPointer = (PopUp*)popUpVoidPointer;
    popUpPointer->promptLoop();
    return 0;
}

PopUpStatus PopUp::promptLoop()
{
    m_beingPrompted = true;

    SDL_Cursor* previousCursor = SDL_GetCursor();
    SDL_SetCursor(m_cursorArrow);

    m_rootWidget.needGeneralDraw();
    SDL_ShowWindow(m_window);
    SDL_RaiseWindow(m_window);

    MouseAndKeyboardState mouseAndKeyboard;
    mouseAndKeyboard.mousePos.w=0;
    mouseAndKeyboard.mousePos.h=0;
    mouseAndKeyboard.keyboardState = SDL_GetKeyboardState(NULL);

    Uint32 windowID = SDL_GetWindowID(m_window);
    bool windowInFocus = true;

    //Event loop
    SDL_Event event;
    m_status = POPUP_IDLE;
    while(m_status == POPUP_IDLE && Widget::waitForMyEvent(&event, windowID, windowInFocus))
    {
        switch(event.type)
        {
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                mouseAndKeyboard.mouseState = SDL_GetMouseState(&mouseAndKeyboard.mousePos.x, &mouseAndKeyboard.mousePos.y);
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                mouseAndKeyboard.keyboardState = SDL_GetKeyboardState(NULL);
                break;

            case SDL_WINDOWEVENT:
                switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                    case SDL_WINDOWEVENT_TAKE_FOCUS:
                        windowInFocus = true;
                        break;

                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        windowInFocus = false;
                        break;
                }
                break;
        }
        manageEvent(event, mouseAndKeyboard);
        draw();
    }

    SDL_HideWindow(m_window);
    SDL_SetCursor(previousCursor);

    SDL_RaiseWindow(m_parentWindow);

    m_beingPrompted=false;

    return m_status;
}

void PopUp::manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard)
{
    switch(event.type)
    {
        case SDL_QUIT:
            m_status = POPUP_CANCEL;//Did not answer
            break;

        case SDL_WINDOWEVENT:
            switch(event.window.event)
            {
                case SDL_WINDOWEVENT_CLOSE:
                    m_status = POPUP_CANCEL;//Did not answer
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    if(m_modal && event.window.windowID!=SDL_GetWindowID(m_window))
                        SDL_RaiseWindow(m_window);//Simulate a modal window because SDL_SetWindowModalFor is "not supported"
                    break;
            }
            break;

        case SDL_KEYDOWN:
            switch(event.key.keysym.scancode)
            {
                case SDL_SCANCODE_ESCAPE:
                    m_status = POPUP_CANCEL;//Did not answer
                    break;

                case SDL_SCANCODE_RETURN:
                case SDL_SCANCODE_KP_ENTER:
                    m_status = POPUP_OK;
                    break;

                default:
                    break;
            }
            break;
    }

    m_rootWidget.manageEvent(event,mouseAndKeyboard);
}

void PopUp::draw()
{
    m_rootWidget.manageDraw();
}



void PopUp::recomputePosAndSize()
{
    int parentWindowX, parentWindowY;
    SDL_GetWindowPosition(m_parentWindow, &parentWindowX, &parentWindowY);

    int X=parentWindowX+m_rect.x, Y=parentWindowY+m_rect.y;
    const int W=m_rect.w, H=m_rect.h;

    SDL_Rect rectDst = Widget::computeAnchorage(X,Y,W,H,m_anchor);

    SDL_SetWindowPosition(m_window, rectDst.x, rectDst.y);
    SDL_SetWindowSize(m_window, W, H);

    SDL_Rect widgetRect={0,0,W,H};
    SDL_Rect windowRect={X,Y,W,H};
    m_rootWidget.setRect(widgetRect, &windowRect);
    widgetRect.h = H/2;
    m_label.setRect(widgetRect);
}


PopUp::~PopUp()
{
    SDL_FreeCursor(m_cursorArrow);
    delete m_callbackButtonOk;
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
}

