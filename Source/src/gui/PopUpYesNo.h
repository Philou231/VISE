#ifndef POPUPYESNO_HEADER
#define POPUPYESNO_HEADER


#include "PopUp.h"


class PopUpYesNo : public PopUp
{
    public:
        PopUpYesNo(SDL_Window* windowParent, SDL_Rect rect, Anchorage anchor, bool modal, std::string title);

        virtual void setText(std::string message, TTF_Font* font=NULL);
        void setButtonNoText(std::string text, TTF_Font* font=NULL);

        ~PopUpYesNo();

    private:
        std::function<int (void*,void*)>* m_callbackButtonNo;

        Button m_buttonNo;
};

#endif //POPUP_HEADER
