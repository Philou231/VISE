#ifndef WIDGETBAR_HEADER
#define WIDGETBAR_HEADER

#include "Widget.h"

class WidgetBar : public Widget
{
    public:
        WidgetBar(SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, int w, int h);
        virtual ~WidgetBar();

    private:

};

#endif // WIDGETBAR_HEADER
