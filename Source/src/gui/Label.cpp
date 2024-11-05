#include "Label.h"

#include <SDL_image.h>
#include "misc/Utilities.h"


using namespace std;


Label::Label(SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, int w, int h) :
    Widget(renderer,parent,renderLevel,x,y,w,h),
    m_font(NULL),
    m_alignment(ALIGN_CENTER),
    m_padding(0),
    m_text(""),
    m_textSurface(NULL),
    m_image(NULL)
{
    m_colorBG = Utilities::colorRGB(200,200,200);
    m_colorFG = Utilities::colorRGB(50,50,50);
}

string Label::getText()
{
    return m_text;
}

void Label::setAlignment(Alignment alignment)
{
    m_alignment = alignment;
    Label::setText(m_text,m_font);
}

void Label::setPadding(int padding)
{
    m_padding = padding;
}

void Label::setColor(Uint32 colorFG, Uint32 colorBG)
{
    m_colorFG = colorFG;
    m_colorBG = colorBG;

    Label::setText(m_text, m_font);

    Widget::needRedraw();
}

void Label::setColorFG(Uint32 colorFG)
{
    m_colorFG = colorFG;

    Label::setText(m_text, m_font);

    Widget::needRedraw();
}

void Label::setColorBG(Uint32 colorBG)
{
    m_colorBG = colorBG;

    Label::setText(m_text, m_font);

    Widget::needRedraw();
}

void Label::setText(string text, TTF_Font* font/*=NULL*/)
{
    m_text = text;

    if(font!=NULL)
        m_font = font;

    if(m_font==NULL)
        return;

    if(m_textSurface!=NULL)
    {
        SDL_FreeSurface(m_textSurface);
        m_textSurface = NULL;
    }

    if(m_text == "")
        return;

    //Decompose the text into multiple lines following the '\n' character
    int width=0, height=0, lastHeight=0;
    vector<SDL_Surface*> surfacePerLine;
    size_t index;
    do
    {
        index = text.find('\n');

        string line;
        if(index != string::npos)
        {
            line = text.substr(0,index);
            text = text.substr(index+1);
        }
        else
            line = text;

        SDL_Surface* newSurface = TTF_RenderUTF8_Blended(m_font, line.c_str(), Utilities::toColor(m_colorFG));
        surfacePerLine.push_back(newSurface);

        if(newSurface==NULL)
        {
            height += lastHeight;
            continue;
        }

        if(newSurface->w>width)
            width = newSurface->w;
        height += newSurface->h;
        lastHeight = newSurface->h;

        SDL_SetSurfaceBlendMode(newSurface, SDL_BLENDMODE_NONE);

    } while(index != string::npos);

    m_textSurface = SDL_CreateRGBSurfaceWithFormat(surfacePerLine[0]->flags,width,height,surfacePerLine[0]->format->BitsPerPixel,surfacePerLine[0]->format->format);

    for(size_t i=0;i<surfacePerLine.size();i++)
    {
        if(surfacePerLine[i]==NULL)
            continue;

        SDL_Rect rectDst;
        rectDst.w = surfacePerLine[i]->w;
        rectDst.h = surfacePerLine[i]->h;
        if(m_alignment==ALIGN_LEFT)
            rectDst.x = 0;
        else if(m_alignment==ALIGN_RIGHT)
            rectDst.x = width-rectDst.w;
        else
            rectDst.x = (width-rectDst.w)/2;
        rectDst.y = height*i/surfacePerLine.size();

        SDL_BlitSurface(surfacePerLine[i],NULL,m_textSurface,&rectDst);
        SDL_FreeSurface(surfacePerLine[i]);
    }


    Widget::needRedraw();
}

void Label::setImage(string path)
{
    if(m_image!=NULL)
        SDL_FreeSurface(m_image);

    m_image = IMG_Load(path.c_str());

    Widget::needRedraw();
}

void Label::setImageColorMod(Uint8 r, Uint8 g, Uint8 b)
{
    SDL_SetSurfaceColorMod(m_image,r,g,b);
}

SDL_Rect Label::getTextSize()
{
    SDL_Rect rectSize={0,0,0,0};

    if(m_textSurface == NULL)
        return rectSize;

    rectSize.x = 0;
    rectSize.y = 0;
    rectSize.w = m_textSurface->w;
    rectSize.h = m_textSurface->h;

    return rectSize;
}

void Label::redraw()
{
    Widget::redraw(); //Changes size if needed


    SDL_FillRect(m_surface,NULL,m_colorBG);

    SDL_Rect rect;
    if(m_image!=NULL)
    {
        rect.x = (m_surface->w-m_image->w)/2;
        rect.y = (m_surface->h-m_image->h)/2;
        rect.w = m_image->w;
        rect.h = m_image->h;
        SDL_BlitSurface(m_image,NULL,m_surface,&rect);
    }
    if(m_textSurface!=NULL)
    {
        rect.x = (m_surface->w-m_textSurface->w)/2 + m_padding;
        rect.y = (m_surface->h-m_textSurface->h)/2;
        rect.w = m_textSurface->w;
        rect.h = m_textSurface->h;
        SDL_BlitSurface(m_textSurface,NULL,m_surface,&rect);
    }

    Widget::needGeneralDraw();
}


Label::~Label()
{
    if(m_image!=NULL)
        SDL_FreeSurface(m_image);
    if(m_textSurface!=NULL)
        SDL_FreeSurface(m_textSurface);
}
