#include "TextField.h"

#include "../misc/Utilities.h"

#define CARRET_TIMEUP 800 //ms
#define CARRET_TIMEDN 200 //ms

using namespace std;


TextField::TextField(SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, int w, int h) :
    Widget(renderer,parent,renderLevel,x,y,w,h),
    m_font(NULL),
    m_callback(NULL),
    m_enabled(true),
    m_editing(false),
    m_copyPasteAction(false),
    m_paddingHeight(5),
    m_paddingLeft(5),
    m_paddingRight(5),
    m_selection(NULL),
    m_selectionIndex(0),
    m_carret(NULL),
    m_carretIndex(0),
    m_initialText(""),
    m_text("")
{
    m_colorFG     = Utilities::colorRGB(255,255,255);
    m_colorBG     = Utilities::colorRGB(100,100,100);
    m_colorBGEdit = Utilities::colorRGB( 50, 50, 50);

    m_selection = SDL_CreateRGBSurfaceWithFormat(0,1,1,32,SDL_PIXELFORMAT_BGRA32);
    SDL_FillRect(m_selection,NULL,Utilities::colorRGBA(255,0,0,50));

    m_carret = SDL_CreateRGBSurface(0,1,1,32,0,0,0,0);
    SDL_FillRect(m_carret,NULL,Utilities::colorRGBA(200,0,0,0));

    m_selectionRect = {0,m_paddingHeight,0,h-2*m_paddingHeight};
    m_carretRect = {0,m_paddingHeight, 2, h-2*m_paddingHeight};

    m_textRect = {m_paddingLeft+m_carretRect.w/2, m_paddingHeight, w-m_paddingLeft-m_paddingRight, m_selectionRect.h};

    m_surface = SDL_CreateRGBSurface(0,w,h,32,0,0,0,0);
    SDL_FillRect(m_surface,NULL,m_colorBG);
}

bool TextField::isEditing()
{
    return m_editing;
}

void TextField::edit(bool editing/*=true*/)
{
    if(m_editing && !editing)//Not editing anymore
    {
        if(m_callback!=NULL)
            m_callback(m_text);
    }

    m_editing = editing;
    setCarretIndex(m_carretIndex,true);

    if(m_editing)
    {
        SDL_Rect absRect = Widget::getAbsRect();
        SDL_SetTextInputRect(&absRect);
        SDL_StartTextInput();
        m_initialText=m_text;
    }

    Widget::needRedraw();
}

void TextField::enable()
{
    m_enabled = true;
}

void TextField::disable()
{
    m_enabled = false;
    m_editing = false;
}

void TextField::visible(bool visible)
{
    Widget::visible(visible);
    if(!visible)
        m_editing = false;
}


void TextField::link(std::function<void(std::string)> callback)
{
    m_callback = callback;
}

void TextField::unlink()
{
    m_callback = NULL;
}



void TextField::manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard)
{
    switch(event.type)
    {
        case SDL_MOUSEMOTION:
            if(m_editing && mouseAndKeyboard.mouseState&SDL_BUTTON(SDL_BUTTON_LEFT))
                TextField::setCarretIndex(positionToIndex(mouseAndKeyboard.mousePos.x),false);
            break;
        case SDL_MOUSEBUTTONDOWN:
            if(cursorIn(mouseAndKeyboard.mousePos) && m_enabled && m_visible && mouseAndKeyboard.mouseState&SDL_BUTTON(SDL_BUTTON_LEFT))
            {
                edit(true);
                m_initialText = getText();
                size_t index = positionToIndex(mouseAndKeyboard.mousePos.x);

                bool update = true;
                if(SDL_GetModState()&KMOD_SHIFT)
                    update = false;
                TextField::setCarretIndex(index,update);
            }
            else if(m_editing)
            {
                edit(false);
            }
            break;
        case SDL_TEXTINPUT:
            if(m_editing)
                insertCharacter(event.edit.text);
            break;
        case SDL_KEYDOWN:
            if(m_editing)
            {
                switch(event.key.keysym.scancode)
                {
                    case SDL_SCANCODE_KP_ENTER:
                    case SDL_SCANCODE_RETURN:
                    case SDL_SCANCODE_ESCAPE:
                        edit(false);
                        break;
                    case SDL_SCANCODE_HOME:
                        {
                            bool update=true;
                            if(SDL_GetModState()&KMOD_SHIFT)
                                update = false;

                            setCarretIndex(0,update);
                        }
                        break;
                    case SDL_SCANCODE_END:
                        {
                            bool update=true;
                            if(SDL_GetModState()&KMOD_SHIFT)
                                update = false;

                            setCarretIndex(m_charCount.size(),update);
                        }
                        break;
                    case SDL_SCANCODE_LEFT:
                        if(m_carretIndex>0)
                        {
                            bool update=true;
                            if(SDL_GetModState()&KMOD_SHIFT)
                                update = false;

                            setCarretIndex(m_carretIndex-1,update);
                        }
                        else if(!(SDL_GetModState()&KMOD_SHIFT))
                            setCarretIndex(0,true);
                        break;
                    case SDL_SCANCODE_RIGHT:
                        if(m_carretIndex<m_charCount.size())
                        {
                            bool update=true;
                            if(SDL_GetModState()&KMOD_SHIFT)
                                update = false;
                            setCarretIndex(m_carretIndex+1,update);
                        }
                        else if(!(SDL_GetModState()&KMOD_SHIFT))
                            setCarretIndex(m_charCount.size(),true);
                        break;
                    case SDL_SCANCODE_BACKSPACE:
                        if(m_carretIndex==m_selectionIndex && m_carretIndex!=0)
                            m_carretIndex--;
                        eraseSelection();
                        updateCarretPosition();
                        break;
                    case SDL_SCANCODE_DELETE:
                        if(m_carretIndex==m_selectionIndex && m_selectionIndex<m_charCount.size())
                            m_selectionIndex++;
                        eraseSelection();
                        updateCarretPosition();
                        break;
                    case SDL_SCANCODE_C:
                        if(!m_copyPasteAction && SDL_GetModState()&KMOD_CTRL)
                        {
                            m_copyPasteAction = true;
                            SDL_SetClipboardText(getSelectedText().c_str());
                        }
                        break;
                    case SDL_SCANCODE_V:
                        if(SDL_GetModState()&KMOD_CTRL && SDL_HasClipboardText())
                        {
                            insertString(SDL_GetClipboardText());
                        }
                        break;
                    case SDL_SCANCODE_X:
                        if(!m_copyPasteAction && SDL_GetModState()&KMOD_CTRL)
                        {
                            m_copyPasteAction = true;
                            SDL_SetClipboardText(getSelectedText().c_str());
                            eraseSelection();
                        }
                    default:
                        break;
                }
            }
            break;
        case SDL_KEYUP:
            switch(event.key.keysym.scancode)
            {
                case SDL_SCANCODE_C:
                    m_copyPasteAction = false;
                    break;
                case SDL_SCANCODE_V:
                    m_copyPasteAction = false;
                    break;
                case SDL_SCANCODE_X:
                    m_copyPasteAction = false;
                default:
                    break;
            }
            break;
    }
}

size_t TextField::indexToUTF8(size_t index)
{
    size_t indexUTF8=0;
    for(size_t i=0;i<index && i<m_charCount.size();i++)
    {
        indexUTF8 += m_charCount[i];
    }
    return indexUTF8;
}

void TextField::eraseSelection()
{
    size_t minPos = min(m_selectionIndex,m_carretIndex);
    size_t maxPos = max(m_selectionIndex,m_carretIndex);

    m_text.erase(indexToUTF8(minPos),indexToUTF8(maxPos)-indexToUTF8(minPos));
    m_charCount.erase(m_charCount.begin()+minPos,m_charCount.begin()+maxPos);
    setCarretIndex(minPos,true);

    if(maxPos-minPos>0 && m_callback!=NULL)
        m_callback(m_text);

    Widget::needRedraw();
}

void TextField::insertCharacter(const char* character)
{
    if(m_selectionIndex != m_carretIndex)//Something is selected
        eraseSelection();

    if(character==NULL)
        return;

    m_text.insert(indexToUTF8(m_carretIndex),character);
    m_charCount.insert(m_charCount.begin()+m_carretIndex,strlen(character));
    setCarretIndex(m_carretIndex+1,true);

    if(m_callback!=NULL)
        m_callback(m_text);

    Widget::needRedraw();
}

void TextField::insertString(string text)
{
    for(size_t i=0;i<text.size();)
    {
        int numChar=0;

        if((text[i] & 0b10000000) == 0b00000000)
            numChar = 1;
        else if((text[i] & 0b11100000) == 0b11000000)
            numChar = 2;
        else if((text[i] & 0b11110000) == 0b11100000)
            numChar = 3;
        else if((text[i] & 0b11111000) == 0b11110000)
            numChar = 4;

        TextField::insertCharacter(text.substr(i,numChar).c_str());

        i += numChar = 1;
    }
}

size_t TextField::positionToIndex(int x)
{
    if(m_charCount.size()<1)
        return 0;

    x = x-m_parent->getAbsRect().x-Widget::getAbsRect().x-m_textRect.x;
    int width=0;
    SDL_Surface* tempSurface=NULL;

    size_t index=0;
    size_t indexUTF8=0;
    for( ;index<m_charCount.size();index++)
    {
        indexUTF8+=m_charCount[index];

        tempSurface = TTF_RenderUTF8_Shaded(m_font,m_text.substr(0,indexUTF8).c_str(),Utilities::toColor(m_colorFG),Utilities::toColor(m_colorBG));

        if(x <= width + (tempSurface->w-width)/2)
        {
            if(tempSurface!=NULL)
                SDL_FreeSurface(tempSurface);

            break;
        }

        width = tempSurface->w;

        if(tempSurface!=NULL)
            SDL_FreeSurface(tempSurface);
    }

    return index;
}

int TextField::indexToPosition(size_t index)
{
    int width=0;

    string subString = m_text.substr(0,indexToUTF8(index));
    TTF_SizeUTF8(m_font,subString.c_str(),&width,NULL);

    return width + m_textRect.x;
}

void TextField::updateCarretPosition()
{
    if(m_editing)
        Widget::needGeneralDraw();

    int width = Widget::getAbsRect().w;
    int height = Widget::getAbsRect().h;
    int textWidth = 0;
    TTF_SizeUTF8(m_font,m_text.c_str(),&textWidth,NULL);

    if(textWidth > width-m_paddingLeft-m_paddingRight)
    {
        if(m_textRect.x+textWidth < width-m_paddingRight)
            m_textRect.x = width-m_paddingRight - textWidth;
    }
    else
    {
        m_textRect.x = m_paddingLeft + m_carretRect.w/2;
    }
    m_textRect.w = width-m_paddingLeft-m_paddingRight;
    m_textRect.h = height-m_paddingHeight;

    int position=indexToPosition(m_carretIndex);
    m_carretRect.x = position - m_carretRect.w/2;


    if(m_carretRect.x > width-m_paddingRight)
    {
        m_carretRect.x = width - m_paddingRight - m_carretRect.w;
        m_textRect.x = m_carretRect.x+m_carretRect.w/2 - (position - m_textRect.x);
        m_textRect.w = m_textRect.w-m_carretRect.w/2;
        m_textRect.h = m_textRect.h;
    }
    if(m_carretRect.x < m_paddingLeft)
    {
        m_carretRect.x = m_paddingLeft;
        m_textRect.x = m_carretRect.x+m_carretRect.w/2 - (position - m_textRect.x);
        m_textRect.w = m_textRect.w-m_carretRect.w/2;
        m_textRect.h = m_textRect.h;
    }

    Widget::needRedraw();
}

void TextField::setSelectionIndex(size_t index)
{
    m_selectionIndex = index;
    m_selectionRect.w = 0;
}

void TextField::setCarretIndex(size_t index, bool updateSelection)
{
    m_carretIndex = index;
    updateCarretPosition();

    if(updateSelection)
    {
        setSelectionIndex(index);
    }
    else
    {
        int minPos = indexToPosition(min(m_carretIndex,m_selectionIndex));
        int maxPos = indexToPosition(max(m_carretIndex,m_selectionIndex));

        int width = Widget::getAbsRect().w;

        if(minPos < m_paddingLeft)
            minPos = m_paddingLeft;
        if(maxPos > width-m_paddingRight)
            maxPos = width-m_paddingRight;

        m_selectionRect.x=minPos;
        m_selectionRect.w=maxPos-minPos;
    }
}


void TextField::clear()
{
    TextField::setCarretIndex(0,false);
    TextField::setSelectionIndex(m_charCount.size());
    TextField::eraseSelection();
}

void TextField::setRect(SDL_Rect position)
{
    Widget::setRect(position);

    m_selectionRect.h = Widget::getAbsRect().h;
    m_carretRect.h = m_selectionRect.h;

    Widget::needRedraw();
}

void TextField::setFont(TTF_Font* font)
{
    m_font = font;

    TTF_SizeUTF8(m_font,"A",NULL,&(m_textRect.h));
    m_textRect.y = (m_rectAbs.h - m_textRect.h)/2;

    Widget::needRedraw();
}

void TextField::setTextColor(Uint32 color)
{
    m_colorFG = color;
    SDL_FillRect(m_carret,NULL,m_colorFG);
}

void TextField::setTextColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a/*=SDL_ALPHA_OPAQUE*/)
{
    setTextColor(Utilities::colorRGBA(r,g,b,a));
}

void TextField::setText(std::string text)
{
    TextField::clear();

    m_text = text;

    m_charCount.clear();
    for(size_t i=0;i<m_text.size();)
    {
        if((m_text[i] & 0b10000000) == 0b00000000)
            m_charCount.push_back(1);
        else if((m_text[i] & 0b11100000) == 0b11000000)
            m_charCount.push_back(2);
        else if((m_text[i] & 0b11110000) == 0b11100000)
            m_charCount.push_back(3);
        else if((m_text[i] & 0b11111000) == 0b11110000)
            m_charCount.push_back(4);

        i += m_charCount[m_charCount.size()-1];
    }

    TextField::setCarretIndex(m_charCount.size(),true);

    Widget::needRedraw();
}

std::string TextField::getText()
{
    return m_text;
}

std::string TextField::getSelectedText()
{
    size_t minPos = min(m_selectionIndex,m_carretIndex);
    size_t maxPos = max(m_selectionIndex,m_carretIndex);

    return m_text.substr(indexToUTF8(minPos),indexToUTF8(maxPos)-indexToUTF8(minPos));
}

void TextField::redraw()
{
    if(m_font==NULL)
        return;


    if(m_editing)
        SDL_FillRect(m_surface,NULL,m_colorBGEdit);
    else
        SDL_FillRect(m_surface,NULL,m_colorBG);

    SDL_Surface* textSurface = NULL;
    textSurface = TTF_RenderUTF8_Blended(m_font, m_text.c_str(), Utilities::toColor(m_colorFG));

    int width=getAbsRect().w;
    SDL_Rect tempRect = {-m_textRect.x + m_paddingLeft,0,width-m_paddingLeft-m_paddingRight,m_textRect.h};
    SDL_Rect tempRect2 = m_textRect;//Prevents it from being overwritten by the blit function
    tempRect2.x = m_paddingLeft;
    SDL_BlitSurface(textSurface,&tempRect,m_surface,&tempRect2);
    SDL_FreeSurface(textSurface);

    if(m_editing && m_selectionRect.w > 0)
        SDL_BlitScaled(m_selection,NULL,m_surface,&m_selectionRect);

    if(m_editing)
    {
        tempRect = m_carretRect;//Prevents it from being overwritten by the blit function
        SDL_BlitScaled(m_carret,NULL,m_surface,&tempRect);
    }

    Widget::needGeneralDraw();
}


TextField::~TextField()
{
    if(m_carret != NULL)
        SDL_FreeSurface(m_carret);

    if(m_selection != NULL)
        SDL_FreeSurface(m_selection);
}
