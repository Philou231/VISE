#ifndef LABEL_HEADER
#define LABEL_HEADER

#include <string>
#include <SDL_ttf.h>
#include "Widget.h"

//TODO: Manage \n characters for multiple lines

enum Alignment{ALIGN_LEFT,ALIGN_CENTER,ALIGN_RIGHT};


class Label : public Widget
{
    public:
        Label(SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, int w, int h);

        std::string getText();
        void setAlignment(Alignment alignment);
        void setPadding(int padding);
        void setColor(Uint32 colorFG, Uint32 colorBG);
        void setColorFG(Uint32 colorFG);
        void setColorBG(Uint32 colorBG);
        void setText(std::string text, TTF_Font* font=NULL);
        void setImage(std::string path);
        void setImageColorMod(Uint8 r, Uint8 g, Uint8 b);
        SDL_Rect getTextSize();

        ~Label();
    private:
        virtual void redraw();

    private:
        TTF_Font* m_font;

        Alignment m_alignment;
        int m_padding;
        std::string m_text;

        Uint32 m_colorFG;

        SDL_Surface* m_textSurface;
        SDL_Surface* m_image;
};


#endif // LABEL_HEADER
