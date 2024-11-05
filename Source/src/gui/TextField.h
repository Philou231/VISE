#ifndef TEXTFIELD_H
#define TEXTFIELD_H


#include <string>
#include <functional>
#include <SDL_ttf.h>
#include "Widget.h"


class TextField : public Widget
{
    public:
        TextField(SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, int w, int h);

        bool isEditing();
        void edit(bool editing=true);
        void enable();
        void disable();

        virtual void visible(bool visible);

        void link(std::function<void(std::string)> callback);
        void unlink();

        void manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard);
        void setSelectionIndex(size_t index);
        void setCarretIndex(size_t index, bool updateSelection);

        void clear();
        void setRect(SDL_Rect position);
        void setFont(TTF_Font* font);
        void setTextColor(Uint32 color);
        void setTextColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a=SDL_ALPHA_OPAQUE);
        void setText(std::string text);
        std::string getText();
        std::string getSelectedText();

        ~TextField();
    private:
        size_t indexToUTF8(size_t index);
        void eraseSelection();
        void insertCharacter(const char* character);
        void insertString(std::string text);
        size_t positionToIndex(int x);
        int indexToPosition(size_t index);
        void updateCarretPosition();
        virtual void redraw();

    private:
        TTF_Font* m_font;
        Uint32 m_colorFG;
        Uint32 m_colorBGEdit;

        std::function<void(std::string)> m_callback;

        bool m_enabled;

        bool m_editing;
        bool m_copyPasteAction;

        int m_paddingHeight;
        int m_paddingLeft;
        int m_paddingRight;

        SDL_Surface* m_selection;
        SDL_Rect m_selectionRect;
        size_t m_selectionIndex;//Referenced to the number of characters (not bytes)

        SDL_Surface* m_carret;
        SDL_Rect m_carretRect;
        size_t m_carretIndex;//Referenced to the number of characters (not bytes)

        std::string m_initialText;//Text present before editing. Used for comparison after editing
        std::vector<int> m_charCount;//Easy way to manage UTF-8 encoding
        std::string m_text;
        SDL_Rect m_textRect;
};



#endif // TEXTFIELD_H
