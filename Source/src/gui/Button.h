#ifndef BUTTON_HEADER
#define BUTTON_HEADER

#include <string>
#include <functional>
#include <SDL_ttf.h>
#include "Widget.h"
#include "Label.h"


class Button : public Widget
{
    public:
        Button(SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, int w, int h);
        void setIcon(std::string path);
        void setIconPressed(std::string path);
        void setIconColorMod(Uint8 r, Uint8 g, Uint8 b);
        void setIconPressedColorMod(Uint8 r, Uint8 g, Uint8 b);

        std::string getText();
        void setText(std::string text, TTF_Font* font=NULL);
        void setTextPressed(std::string text, TTF_Font* font=NULL);
        void setTextColor(Uint8 r, Uint8 g, Uint8 b);
        void setTextColorPressed(Uint8 r, Uint8 g, Uint8 b);

        void setColorBGIdle(Uint8 r, Uint8 g, Uint8 b);
        void setColorBGOver(Uint8 r, Uint8 g, Uint8 b);
        void setColorBGPressed(Uint8 r, Uint8 g, Uint8 b);
        void setColorBGPressedInIdle(Uint8 r, Uint8 g, Uint8 b);
        void setColorBGPressedInOver(Uint8 r, Uint8 g, Uint8 b);

        void recomposite();

        void pressable(bool isPressable=true);
        void toggle();
        void toggleNoCallback();
        void press();
        void depress();
        bool pressed();

        void setCallback(std::function<int (void*,void*)>* callbackFunction, void* objectPointer, void* arguments);
        void manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard);
        int executeAction();

        virtual ~Button();
    protected:
        virtual void redraw();

    private:
        std::function<int (void*,void*)>* m_callbackFunction;
        void* m_callbackObjPtr;
        void* m_callbackArguments;

        Label m_label;
        Label m_labelPressed;

        bool m_clickedIn;

        Uint32 m_colorBGIdle;
        Uint32 m_colorBGOver;
        Uint32 m_colorBGPressed;
        Uint32 m_colorBGPressedInIdle;
        Uint32 m_colorBGPressedInIdleOver;
        Uint32 m_colorText;
        Uint32 m_colorTextPressed;

        bool m_pressable;
        bool m_labelPressedInitialized;
        bool m_pressedIn;
};

#endif // BUTTON_HEADER
