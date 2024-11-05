#include "Widget.h"

#include "misc/Utilities.h"


Widget* Widget::m_cursorCapturedByWidget=NULL;


Widget::Widget(SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, int w, int h) :
    m_renderer(renderer),
    m_parent(parent),
    m_maxRenderLevel(0),
    m_renderLevel(renderLevel),
    m_visible(true),
    m_surface(NULL),
    m_texture(NULL),
    m_rectRel({x,y,w,h}),
    m_needRedraw(false),
    m_needGeneralDraw(false)
{
    recomputeAbsRect();
    m_surface = SDL_CreateRGBSurfaceWithFormat(0,m_rectAbs.w,m_rectAbs.h,32,SDL_PIXELFORMAT_BGRA32);
    m_colorBG = Utilities::colorRGB(200,200,200);

    Widget::setMaxRenderLevel(m_renderLevel);

    if(parent!=NULL)
        parent->addToChildren(this);
}

void Widget::addToChildren(Widget* widget)
{
    m_children.push_back(widget);
}

void Widget::needGeneralDraw()
{
    if(m_parent==NULL)//This is the root widget
        m_needGeneralDraw=true;
    else
        m_parent->needGeneralDraw();
}

void Widget::setMaxRenderLevel(uint8_t renderLevel)
{
    if(m_parent!=NULL)
        m_parent->setMaxRenderLevel(renderLevel);
    else //root widget
    {
        if(renderLevel > m_maxRenderLevel)
            m_maxRenderLevel = renderLevel;
    }
}

void Widget::draw(SDL_Surface* surface/*=NULL*/, uint8_t renderLevel/*=0*/) //TODO: Change this function so that everyone uses textures and SDL_RenderCopy instead of blitting on a surface
{
    if(m_needRedraw)
    {
        redraw();
        m_needRedraw=false;
    }

    if(m_parent==NULL)//This is the root widget
    {
        SDL_RenderClear(m_renderer);

        for(uint16_t l=0;l<=m_maxRenderLevel;l++)
        {
            if(l==0)
                SDL_FillRect(m_surface,NULL,m_colorBG);
            else
                SDL_FillRect(m_surface,NULL,Utilities::colorRGBA(0,0,0,SDL_ALPHA_TRANSPARENT));

            for(size_t i=0;i<m_children.size();i++)
            {
                if(m_children[i]->visible())
                    m_children[i]->draw(m_surface,l);
            }

            if(m_texture!=NULL)
                SDL_DestroyTexture(m_texture);

            m_texture = SDL_CreateTextureFromSurface(m_renderer,m_surface);

            SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
        }
        SDL_RenderPresent(m_renderer);
    }
    else
    {
        if(renderLevel == m_renderLevel)
            SDL_BlitSurface(m_surface,NULL,surface,&m_rectAbs);

        for(size_t i=0;i<m_children.size();i++)
        {
            if(m_children[i]->visible())
                m_children[i]->draw(surface,renderLevel);//All widgets draw on the root widget's surface
        }
    }
}

bool Widget::visible()
{
    return m_visible;
}

void Widget::visible(bool visible)
{
    if(m_visible!=visible)
        Widget::needGeneralDraw();

    m_visible = visible;
}

void Widget::show()
{
    if(m_visible!=true)
        Widget::needGeneralDraw();

    m_visible = true;
}

void Widget::hide()
{
    if(m_visible!=false)
        Widget::needGeneralDraw();

    m_visible = false;
}

void Widget::setRect(int x, int y, int w, int h, SDL_Rect* windowRect/*=NULL*/)
{
    m_rectRel.x=x;
    m_rectRel.y=y;
    m_rectRel.w=w;
    m_rectRel.h=h;

    recomputeAbsRect(windowRect);
}

SDL_Rect Widget::getRect()
{
    return m_rectRel;
}

void Widget::setRect(SDL_Rect rect, SDL_Rect* windowRect/*=NULL*/)
{
    setRect(rect.x, rect.y, rect.w, rect.h, windowRect);
}

SDL_Rect Widget::getAbsRect()
{
    return m_rectAbs;
}

void Widget::recomputeAbsRect(SDL_Rect* windowRect/*=NULL*/)
{
    SDL_Rect newRectAbs;

    if(m_parent!=NULL)
    {
        SDL_Rect parentRect = m_parent->getAbsRect();
        newRectAbs = m_rectRel;

        if(m_rectRel.x<0)
            newRectAbs.x = parentRect.x+parentRect.w+m_rectRel.x;
        else
            newRectAbs.x = parentRect.x+m_rectRel.x;
        if(m_rectRel.y<0)
            newRectAbs.y = parentRect.h+m_rectRel.y;
        else
            newRectAbs.y = parentRect.y+m_rectRel.y;
        if(m_rectRel.w<=0)
            newRectAbs.w = parentRect.w+m_rectRel.w;
        if(m_rectRel.h<=0)
            newRectAbs.h = parentRect.h+m_rectRel.h;
    }
    else
    {
        newRectAbs = m_rectRel;
    }

    if(newRectAbs.w != m_rectAbs.w || newRectAbs.h != m_rectAbs.h)
        Widget::needRedraw();

    m_rectAbs = newRectAbs;


    for(size_t i=0;i<m_children.size();i++)
    {
        m_children[i]->recomputeAbsRect(windowRect);
    }
}

void Widget::needRedraw()
{
    m_needRedraw=true;
    needGeneralDraw();
}

void Widget::redraw()
{
    SDL_FreeSurface(m_surface);
    m_surface = SDL_CreateRGBSurfaceWithFormat(0,m_rectAbs.w,m_rectAbs.h,32,SDL_PIXELFORMAT_BGRA32);
    SDL_FillRect(m_surface,NULL,m_colorBG);
}

void Widget::manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard)
{
    if(m_parent==NULL)
    {
        switch(event.type)
        {
            case SDL_WINDOWEVENT:
                switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_EXPOSED:
                        Widget::needGeneralDraw();
                        break;
                }
                break;
        }
    }

    for(size_t i=0;i<m_children.size();i++)
    {
        if(m_children[i]->visible())
            m_children[i]->manageEvent(event, mouseAndKeyboard);
    }
}

void Widget::manageDraw()
{
    if(m_needGeneralDraw)
    {
        Widget::draw();
        m_needGeneralDraw=false;
    }
}

void Widget::captureCursor(Widget* captured)
{
    m_cursorCapturedByWidget = captured;
}

bool Widget::cursorIn(SDL_Rect cursorPos, SDL_Rect* rect/*=NULL*/)
{
    if(m_cursorCapturedByWidget!=NULL && m_cursorCapturedByWidget!=this) //Cursor cannot interact when captured
        return false;

    SDL_Rect rectBoundary;
    if(rect==NULL)
        rectBoundary = m_rectAbs;
    else
        rectBoundary = *rect;

    if(cursorPos.x>=rectBoundary.x && cursorPos.x<rectBoundary.x+rectBoundary.w &&
       cursorPos.y>=rectBoundary.y && cursorPos.y<rectBoundary.y+rectBoundary.h)
        return true;
    else
        return false;
}

bool Widget::rectTouchingIn(SDL_Rect rectPos, SDL_Rect* rectBoundaryGiven/*=NULL*/)
{
    SDL_Rect rectBoundary;
    if(rectBoundaryGiven==NULL)
        rectBoundary = m_rectAbs;
    else
        rectBoundary = *rectBoundaryGiven;

    if(rectPos.x+rectPos.w>rectBoundary.x && rectPos.x<rectBoundary.x+rectBoundary.w &&
       rectPos.y+rectPos.h>rectBoundary.y && rectPos.y<rectBoundary.y+rectBoundary.h)
        return true;
    else
        return false;
}

TouchSide Widget::rectTouchingOut(SDL_Rect rectPos, SDL_Rect* rectBoundaryGiven/*=NULL*/)
{
    SDL_Rect rectBoundary;
    if(rectBoundaryGiven==NULL)
        rectBoundary = m_rectAbs;
    else
        rectBoundary = *rectBoundaryGiven;

    int touchSide=TOUCH_NONE;

    if(rectPos.x+rectPos.w>rectBoundary.x+rectBoundary.w)
        touchSide |= TOUCH_RIGHT;
    if(rectPos.x<rectBoundary.x)
        touchSide |= TOUCH_LEFT;

    if(rectPos.y+rectPos.h>rectBoundary.y+rectBoundary.h)
        touchSide |= TOUCH_BOTTOM;
    else if(rectPos.y<rectBoundary.y)
        touchSide |= TOUCH_TOP;

    return (TouchSide)touchSide;
}

SDL_Rect Widget::computeAnchorage(int anchorX, int anchorY, int widgetW, int widgetH, Anchorage anchor)
{
    SDL_Rect dstRect;
    dstRect.x = anchorX;
    dstRect.y = anchorY;
    dstRect.w = widgetW;
    dstRect.h = widgetH;

    switch(anchor)
    {
        case TOP_LEFT:
            dstRect.x -= 0;
            dstRect.y -= 0;
            break;
        case TOP_CENTER:
            dstRect.x -= widgetW/2;
            dstRect.y -= 0;
            break;
        case TOP_RIGHT:
            dstRect.x -= widgetW;
            dstRect.y -= 0;
            break;

        case CENTER_LEFT:
            dstRect.x -= 0;
            dstRect.y -= widgetH/2;
            break;
        case CENTERED:
            dstRect.x -= widgetW/2;
            dstRect.y -= widgetH/2;
            break;
        case CENTER_RIGHT:
            dstRect.x -= widgetW;
            dstRect.y -= widgetH/2;
            break;

        case BOTTOM_LEFT:
            dstRect.x -= 0;
            dstRect.y -= widgetH;
            break;
        case BOTTOM_CENTER:
            dstRect.x -= widgetW/2;
            dstRect.y -= widgetH;
            break;
        case BOTTOM_RIGHT:
            dstRect.x -= widgetW;
            dstRect.y -= widgetH;
            break;

        default:
            break;
    }

    return dstRect;
}

Uint32 Widget::getEventTimestamp(SDL_Event event)
{
    switch(event.type)
    {
        case SDL_AUDIODEVICEADDED:
        case SDL_AUDIODEVICEREMOVED:
            return event.adevice.timestamp;

        case SDL_CONTROLLERAXISMOTION:
            return event.caxis.timestamp;

        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            return event.cbutton.timestamp;

        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
        case SDL_CONTROLLERDEVICEREMAPPED:
            return event.cdevice.timestamp;

        case SDL_DOLLARGESTURE:
        case SDL_DOLLARRECORD:
            return event.dgesture.timestamp;

        case SDL_DROPFILE:
        case SDL_DROPTEXT:
        case SDL_DROPBEGIN:
        case SDL_DROPCOMPLETE:
            return event.drop.timestamp;

        case SDL_FINGERMOTION:
        case SDL_FINGERDOWN:
        case SDL_FINGERUP:
            return event.tfinger.timestamp;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
            return event.key.timestamp;

        case SDL_JOYAXISMOTION:
            return event.jaxis.timestamp;

        case SDL_JOYBALLMOTION:
            return event.jball.timestamp;

        case SDL_JOYHATMOTION:
            return event.jhat.timestamp;

        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
            return event.jbutton.timestamp;

        case SDL_JOYDEVICEADDED:
        case SDL_JOYDEVICEREMOVED:
            return event.jdevice.timestamp;

        case SDL_MOUSEMOTION:
            return event.motion.timestamp;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            return event.button.timestamp;

        case SDL_MOUSEWHEEL:
            return event.wheel.timestamp;

        case SDL_MULTIGESTURE:
            return event.mgesture.timestamp;

        case SDL_QUIT:
            return event.quit.timestamp;

        case SDL_SYSWMEVENT:
            return event.syswm.timestamp;

        case SDL_TEXTEDITING:
            return event.edit.timestamp;

        case SDL_TEXTINPUT:
            return event.text.timestamp;

        case SDL_USEREVENT:
            return event.user.timestamp;

        case SDL_WINDOWEVENT:
            return event.window.timestamp;

        default:
            return 0;
    }
}

int Widget::isEventForMyWindow(SDL_Event event, Uint32 windowID)
{
    switch(event.type)
    {
        case SDL_DROPFILE:
        case SDL_DROPTEXT:
        case SDL_DROPBEGIN:
        case SDL_DROPCOMPLETE:
            return event.drop.windowID == windowID;

        case SDL_FINGERMOTION:
        case SDL_FINGERDOWN:
        case SDL_FINGERUP:
            return event.tfinger.windowID == windowID;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
            return event.key.windowID == windowID;

        case SDL_MOUSEMOTION:
            return event.motion.windowID == windowID;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            return event.button.windowID == windowID;

        case SDL_MOUSEWHEEL:
            return event.wheel.windowID == windowID;

        case SDL_TEXTEDITING:
            return event.edit.windowID == windowID;

        case SDL_TEXTINPUT:
            return event.text.windowID == windowID;

        case SDL_USEREVENT:
            return event.user.windowID == windowID;

        case SDL_WINDOWEVENT:
            return event.window.windowID == windowID;

        case SDL_AUDIODEVICEADDED:
        case SDL_AUDIODEVICEREMOVED:
        case SDL_CONTROLLERAXISMOTION:
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
        case SDL_CONTROLLERDEVICEREMAPPED:
        case SDL_DOLLARGESTURE:
        case SDL_DOLLARRECORD:
        case SDL_JOYAXISMOTION:
        case SDL_JOYBALLMOTION:
        case SDL_JOYHATMOTION:
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
        case SDL_JOYDEVICEADDED:
        case SDL_JOYDEVICEREMOVED:
        case SDL_MULTIGESTURE:
        case SDL_QUIT:
        case SDL_SYSWMEVENT:
        default:
            return -1; //Undefined
    }
}

int Widget::waitForMyEvent(SDL_Event* event, Uint32 windowID, bool windowInFocus)
{
    unsigned int timeout=100;

    while(true)
    {
        SDL_PumpEvents();
        int returnedValue = SDL_PeepEvents(event,1,SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
        if(returnedValue==0) //No events
        {
            SDL_Delay(10);
            continue;
        }
        else if(returnedValue==-1) //Error
        {
            cout <<"Error while reading events: " <<SDL_GetError() <<endl;
            return 0;
        }

        switch(Widget::isEventForMyWindow(*event,windowID))
        {
            case 0: //Not our event
                if(event->type==SDL_WINDOWEVENT && event->window.event==SDL_WINDOWEVENT_RESIZED)
                    timeout=5000;//Timestamp is set at the beginning of resize instead of at the end...

                if(SDL_GetTicks() - Widget::getEventTimestamp(*event) > timeout)
                    SDL_PollEvent(event); //The message was not handled by its recipient so we destroy the message
                SDL_Delay(10);
                continue;

            case 1: //Our event
                break;

            case -1: //Event is not associated with a window
                if(windowInFocus || SDL_GetTicks() - Widget::getEventTimestamp(*event) > timeout)
                {
                    break; //Our window is in focus or the message was not handled in time by the window in focus, we act upon the message
                }
                else
                {
                    SDL_Delay(10);
                    continue; //Not in focus, don't read
                }

        }

        break;
    }

    SDL_PollEvent(event);
    return 1;
}

Widget::~Widget()
{
    if(m_texture!=NULL)
        SDL_DestroyTexture(m_texture);
    if(m_surface!=NULL)
        SDL_FreeSurface(m_surface);
}
