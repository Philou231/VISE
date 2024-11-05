#ifndef TOOLTIP_HEADER
#define TOOLTIP_HEADER

#include <string>
#include <SDL_ttf.h>
#include "Widget.h"
#include "Label.h"


class Tooltip : public Widget
{
    public:
        Tooltip(SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, Anchorage anchor);

        void setColor(Uint32 colorFG, Uint32 colorBG);
        std::string getText();
        void setText(std::string text, TTF_Font* font=NULL);
        void setPosition(SDL_Rect position, Anchorage anchor);
        virtual void show();

        void setDelayMs(unsigned long delayShown);
        void manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard);

        virtual void recomputeAbsRect(SDL_Rect* windowRect=NULL);

        ~Tooltip();

    private:
        Label m_label;

        SDL_Rect m_position;
        Anchorage m_anchor;

        unsigned long m_delayShown; //In milliseconds
        unsigned long m_lastTimeShown;
};


#endif // TOOLTIP_HEADER
