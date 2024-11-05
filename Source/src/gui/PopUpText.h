#ifndef POPUPTEXT_HEADER
#define POPUPTEXT_HEADER

#include "PopUp.h"
#include "TextField.h"
#include "Tooltip.h"


class PopUpText : public PopUp
{
    public:
        PopUpText(SDL_Window* windowParent, SDL_Rect rect, Anchorage anchor, bool modal, std::string title);

        void setTexts(std::string message, TTF_Font* fontMessage=NULL, TTF_Font* fontTextField=NULL, TTF_Font* fontTooltip=NULL, std::string tooltipText="");

        void setPossibleAnswers(std::vector<std::string> possibleAnswers);
        void setPrefilledText(std::string message);

        virtual PopUpStatus prompt(std::string* input, std::string defaultText);
        virtual PopUpStatus prompt(std::string* input, bool* foundCaseSensitive, std::string defaultText);
        virtual void manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard);

        ~PopUpText();
    private:
        virtual void recomputePosAndSize();

    private:
        std::vector<std::string> m_possibleAnswers;

        TextField m_textField;
        Tooltip m_tooltipIncorrect;

        bool m_foundCaseSensistive; //true if the item was found while being case sensitive, false if the item was found while not being case sensitive
};

#endif //POPUPTEXT_HEADER
