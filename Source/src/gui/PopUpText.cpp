#include "PopUpText.h"
#include <iostream>
#include "../misc/Utilities.h"

using namespace std;


#define PADDING_X 10
#define PADDING_Y 5
#define BUTTON_W (50-PADDING_Y*2)


PopUpText::PopUpText(SDL_Window* windowParent, SDL_Rect rect, Anchorage anchor, bool modal, string title) :
    PopUp(windowParent, rect, anchor, modal, title),
    m_textField(m_renderer,&m_rootWidget,0,PADDING_X,rect.h/2+PADDING_Y,rect.w-BUTTON_W-3*PADDING_X,rect.h/2-2*PADDING_Y),
    m_tooltipIncorrect(m_renderer,&m_rootWidget,0,-PADDING_X*2-BUTTON_W-5,-rect.h/4,CENTER_RIGHT),
    m_foundCaseSensistive(false)
{
    m_buttonOk.setRect(rect.w-PADDING_X-BUTTON_W, rect.h/2+PADDING_Y, BUTTON_W, rect.h/2-2*PADDING_Y);
    m_tooltipIncorrect.setColor(Utilities::colorRGB(255,255,255), Utilities::colorRGB(127,0,0));
}

void PopUpText::setTexts(string message, TTF_Font* fontMessage/*=NULL*/, TTF_Font* fontTextField/*=NULL*/, TTF_Font* fontTooltip/*=NULL*/, string tooltipText/*=""*/)
{
    PopUp::setText(message, fontMessage);

    m_textField.setFont(fontTextField);
    if(tooltipText!="")
        m_tooltipIncorrect.setText(tooltipText,fontTooltip);
}

void PopUpText::setPossibleAnswers(vector<string> possibleAnswers)
{
    m_possibleAnswers = possibleAnswers;
}

void PopUpText::setPrefilledText(std::string message)
{
    m_textField.setText(message);
}

PopUpStatus PopUpText::prompt(string* input, string defaultText) //Modal window
{
    return PopUpText::prompt(input, NULL, defaultText);
}

PopUpStatus PopUpText::prompt(string* input, bool* foundCaseSensitive, string defaultText) //Modal window
{
    m_textField.setText(defaultText);
    m_textField.edit(true);

    PopUpStatus status = PopUp::prompt();

    if(input!=NULL)
    {
        if(status==POPUP_OK)
            (*input) = m_textField.getText();
        else if(status==POPUP_CANCEL)
            (*input) = "";
    }

    if(foundCaseSensitive!=NULL)
        (*foundCaseSensitive) = m_foundCaseSensistive;

    return status;
}

void PopUpText::manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard)
{
    PopUp::manageEvent(event,mouseAndKeyboard);

    if(m_status==POPUP_OK)
    {
        if(m_possibleAnswers.size()!=0)
        {
            m_status=POPUP_IDLE; //Don't allow quitting with OK if the input is not acceptable

            string textInput = m_textField.getText();
            for(size_t i=0;i<m_possibleAnswers.size();i++)
            {
                if(textInput == m_possibleAnswers[i])//It exists
                {
                    m_foundCaseSensistive = true;
                    m_status = POPUP_OK; //Found it!
                    break;
                }
            }

            //Try without being case sensitive
            textInput = Utilities::toLower(m_textField.getText());
            for(size_t i=0;i<m_possibleAnswers.size();i++)
            {
                if(textInput == Utilities::toLower(m_possibleAnswers[i]))//It exists
                {
                    m_textField.setText(m_possibleAnswers[i]); //Change the input text field to match the case of the text
                    m_foundCaseSensistive = false;
                    m_status = POPUP_OK; //Found it!
                    break;
                }
            }

            if(m_status==POPUP_IDLE)//Was not found
            {
                m_tooltipIncorrect.show();
                m_textField.edit();
            }
        }
    }
}



void PopUpText::recomputePosAndSize()
{
    PopUp::recomputePosAndSize();

    SDL_Rect widgetRect={0, m_rect.h/2, m_rect.w, m_rect.h/2};
    m_textField.setRect(widgetRect);
}


PopUpText::~PopUpText()
{

}
