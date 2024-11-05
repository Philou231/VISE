#ifndef POPUP_HEADER
#define POPUP_HEADER

#include <string>
#include <SDL_ttf.h>
#include "Widget.h"
#include "Label.h"
#include "Button.h"

enum PopUpStatus{POPUP_IDLE, POPUP_OK, POPUP_NO, POPUP_CANCEL};


class PopUp
{
    public:
        PopUp(SDL_Window* windowParent, SDL_Rect rect, Anchorage anchor, bool modal, std::string title);

        void setTextAlignment(Alignment alignement);
        void setTextPadding(int padding);
        virtual void setText(std::string message, TTF_Font* font=NULL);
        void setTitle(std::string title);
        void setButtonOkText(std::string text, TTF_Font* font=NULL);
        void setButtonOkSpacingHeight(int height);
        void setPositionAndSize(SDL_Rect rect, Anchorage anchor);

        virtual PopUpStatus prompt();
        static int promptLoopCaller(void* popUpVoidPointer);
        virtual PopUpStatus promptLoop();
        virtual void manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard);
        virtual void draw();

        ~PopUp();
    protected:
        virtual void recomputePosAndSize();

    protected:
        SDL_Window* m_parentWindow;
        SDL_Window* m_window;
        SDL_Renderer* m_renderer;

        std::function<int (void*,void*)>* m_callbackButtonOk;

        SDL_Rect m_rect;
        Anchorage m_anchor;

        bool m_modal;
        bool m_beingPrompted;
        PopUpStatus m_status;

        SDL_Cursor* m_cursorArrow;

        Widget m_rootWidget;
        Label m_label;
        Button m_buttonOk;
};

#endif //POPUP_HEADER

