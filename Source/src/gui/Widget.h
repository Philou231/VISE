#ifndef WIDGET_HEADER
#define WIDGET_HEADER

#include <vector>
#include <SDL.h>


struct MouseAndKeyboardState
{
    SDL_Rect mousePos;
    Uint32 mouseState;
    const Uint8* keyboardState;
};

enum Anchorage{TOP_LEFT,TOP_CENTER,TOP_RIGHT,CENTER_LEFT,CENTERED,CENTER_RIGHT,BOTTOM_LEFT,BOTTOM_CENTER,BOTTOM_RIGHT};

enum TouchSide{TOUCH_NONE=0, TOUCH_TOP=1, TOUCH_BOTTOM=2, TOUCH_LEFT=4, TOUCH_RIGHT=8};


class Widget
{
    public:
        Widget(SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, int w, int h);
        void needGeneralDraw();
        void setMaxRenderLevel(uint8_t renderLevel);
        void addToChildren(Widget* widget);

        uint8_t getRenderLevel() {return m_renderLevel;}
        void setRenderLevel(uint8_t renderLevel) {m_renderLevel = renderLevel;}

        virtual void draw(SDL_Surface* surface=NULL, uint8_t renderLevel=0);

        bool visible();
        virtual void visible(bool visible);
        virtual void show();
        void hide();

        void setRect(int x, int y, int w, int h, SDL_Rect* windowRect=NULL); //relative
        void setRect(SDL_Rect rect, SDL_Rect* windowRect=NULL); //relative
        SDL_Rect getRect(); //relative
        SDL_Rect getAbsRect();
        virtual void recomputeAbsRect(SDL_Rect* windowRect=NULL);

        virtual void manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard);
        void manageDraw();
        static void captureCursor(Widget* captured);
        bool cursorIn(SDL_Rect cursorPos, SDL_Rect* rect=NULL);
        bool rectTouchingIn(SDL_Rect rectPos, SDL_Rect* rectBoundaryGiven=NULL);
        TouchSide rectTouchingOut(SDL_Rect rectPos, SDL_Rect* rectBoundaryGiven=NULL);
        static SDL_Rect computeAnchorage(int anchorX, int anchorY, int widgetW, int widgetH, Anchorage anchor);

        static Uint32 getEventTimestamp(SDL_Event event);
        static int isEventForMyWindow(SDL_Event event, Uint32 windowID);
        static int waitForMyEvent(SDL_Event* event, Uint32 windowID, bool windowInFocus);

        virtual ~Widget();
    protected:
        void needRedraw();
        virtual void redraw();

    protected:
        std::vector<Widget*> directRender;//These are the objects that needs to bee rendered last as they render themselves

        SDL_Renderer* m_renderer;
        Widget* m_parent;

        uint8_t m_maxRenderLevel;
        uint8_t m_renderLevel; //Determines the order in which elements gets rendered. Level 0 are all applied on the main surface, higher levels render themselves on top of that

        bool m_visible;

        std::vector<Widget*> m_children;
        SDL_Surface* m_surface;
        SDL_Texture* m_texture;


        SDL_Rect m_rectRel;
        SDL_Rect m_rectAbs;

        Uint32 m_colorBG;

        static Widget* m_cursorCapturedByWidget;

        bool m_needRedraw; //For individual widgets
        bool m_needGeneralDraw; //For all widgets at once
};

#endif // WIDGET_HEADER
