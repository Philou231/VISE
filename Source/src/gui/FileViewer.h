#ifndef FILEVIEWER_HEADER
#define FILEVIEWER_HEADER

#include <vector>
#include <functional>
#include "SDL_ttf.h"
#include "Widget.h"
#include "file/File.h"
#include "file/Image.h"
#include "file/PickAndPlace.h"


//It could be useful to have a FileManager object if other types of files gets included or if the software becomes bigger
//This FileViewer would receive notifications from the FileManager and send requests for removal or access to the data
//It would be essential if the software becomes time driven and parallelism is used since a mutex on the list would be required


class FileViewer : public Widget
{
    public:
        FileViewer(SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, int w, int h);

        void setFont(TTF_Font* font);

        void linkToFileList(std::vector<File*>* files);
        void setSelectionCallback(std::function<int (void*,void*)>* callbackFunction, void* objectPointer);

        void manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard);

        void selectFile(File* file);
        int selectFileAndCallback(File* file);
        File* selected();

        Image* getNextImage(std::string currentImagePath);

        virtual void draw(SDL_Surface* surface, uint8_t renderLevel);
        virtual void recomputeAbsRect(SDL_Rect* windowRect=NULL);
        ~FileViewer();
    private:
        int maxListHeight();
        void redrawThumbs();
        void redrawBorders();
        void modifyProgress(int progressChange);
        void computeListRect(int index, SDL_Rect* rectSrc, SDL_Rect* rectDst);
        void updateSelection(SDL_Rect mousePos);
        SDL_Texture* getFilePreview(File* file, SDL_Rect rectSrcComputed, SDL_Rect rectDstComputed, SDL_Rect* rectSrc, SDL_Rect* rectDst);
        SDL_Texture* createPreviewPickAndPlace(PickAndPlace* pickAndPlace);

    private:
        std::function<int (void*,void*)>* m_callbackFunction;
        void* m_callbackObjPtr;

        TTF_Font* m_font;

        std::vector<File*>* m_files;
        std::vector<std::pair<std::string,SDL_Texture*>> m_filePreviews;
        SDL_Texture* m_previewBackground;
        SDL_Texture* m_borderIdle;
        SDL_Texture* m_borderOver;
        SDL_Texture* m_borderSelected;

        std::string m_fileSelected;//Store the path as a unique identifier

        int m_panRelCursorPos;

        SDL_Rect m_lastMousePos;
        SDL_Rect m_rectScrollbar;

        size_t m_lastFileCount;

        SDL_Texture* m_scrollBarBG;
        SDL_Texture* m_thumbIdleTexture;
        SDL_Texture* m_thumbOverTexture;
        SDL_Texture* m_thumbPressedTexture;
        SDL_Texture** m_currentThumbTexture;

        int m_filePreviewW;
        int m_filePreviewH;
        bool m_clickedInPreview;

        int m_scrollProgress;
        bool m_clickedInScrollbar;
};

#endif //#ifndef FILEVIEWER_HEADER


