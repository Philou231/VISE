#include "PopUpYesNo.h"


using namespace std;

#define BUTTON_W 80
#define BUTTON_H 40
#define PADDING_X 30


PopUpYesNo::PopUpYesNo(SDL_Window* windowParent, SDL_Rect rect, Anchorage anchor, bool modal, string title) :
    PopUp(windowParent, rect, anchor, modal, title),
    m_callbackButtonNo(NULL),
    m_buttonNo(m_renderer,&m_rootWidget,0,(rect.w+PADDING_X)/2,rect.h*3/4-BUTTON_H/2,BUTTON_W,BUTTON_H)
{
    m_buttonOk.setRect((rect.w-PADDING_X)/2-BUTTON_W, rect.h*3/4-BUTTON_H/2, BUTTON_W, BUTTON_H);
    m_buttonOk.setText("Yes");

    m_callbackButtonNo = new function<int (void*,void*)> ([&](void* objPtr, void* arguments)
    {
        PopUpYesNo* popUpPtr = (PopUpYesNo*)objPtr;
        popUpPtr->m_status=POPUP_NO;
        return 0;
    });
    m_buttonNo.setCallback(m_callbackButtonNo,this,NULL);
    m_buttonNo.setText("No");
}

void PopUpYesNo::setText(string message, TTF_Font* font/*=NULL*/)
{
    PopUp::setText(message,font);
    m_buttonNo.setText(m_buttonNo.getText(),font);
}

void PopUpYesNo::setButtonNoText(string text, TTF_Font* font/*=NULL*/)
{
    m_buttonNo.setText(text, font);
}


PopUpYesNo::~PopUpYesNo()
{

}
