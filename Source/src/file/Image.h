#ifndef IMAGE_HEADER
#define IMAGE_HEADER

#include <vector>
#include <SDL.h>
#include "file/File.h"

struct TextureView
{
    void* UID;//Pointer to the object requesting the crop. Used as unique identifier
    SDL_Rect windowRect;//Used to see the difference in window rect to adjust the image in the frame
    SDL_Rect widgetRect;//Coordinates of the widget displaying this image
    SDL_Rect rect;//Coordinates of the region being seen
    SDL_Rect crop;//Coordinates of the crop region
    SDL_Texture* textureCropped;//This cropped version is used if it doesn't fit in memory (m_textureAll is scaled down)
    bool displayingCropped;
};

class Image: public File
{
    public:
        Image(SDL_Renderer* renderer, std::string path);

        int getW();
        int getOriginalW();
        int getH();
        int getOriginalH();
        SDL_Rect getSize();
        virtual int createFile(std::string path);
        virtual bool loaded();

        void setView(void* uid, SDL_Rect displayRect, SDL_Rect* windowRect=NULL);//uid is simply a pointer to the object calling this function (this). Used as identifier
        void getRects(void* uid, SDL_Rect* rectSrc, SDL_Rect* rectDst);
        SDL_Rect getWindowRect(void* uid);
        SDL_Texture* getTexture(void* uid);
        SDL_Rect getRect(void* uid);
        void setPos(void* uid, int x, int y);
        void setZoom(void* uid, SDL_Rect originalImageRect, SDL_Rect originalPos, SDL_Rect currentPos);

        SDL_Rect coordImageToWindow(void* uid, SDL_Rect rectImage); //Converts a rectangle from the coordinate system of the image's pixels coordinates to the window
        SDL_Rect coordWindowToImage(void* uid, SDL_Rect rectWindow); //Converts a rectangle from the coordinate system of the window to the image's pixels coordinates
        SDL_Rect coordImageToViewer(void* uid, SDL_Rect rectImage); //Converts a rectangle from the coordinate system of the image's pixels coordinates to the viewer
        SDL_Rect coordViewerToImage(void* uid, SDL_Rect rectViewer); //Converts a rectangle from the coordinate system of the viewer to the image's pixels coordinates

        void resetView(void* uid);
        void showAll(void* uid);
        void showPixelPerfect(void* uid);
        void pan(void* uid, int x, int y);
        void zoom(void* uid, SDL_Rect cursorPos, int z);

        virtual ~Image();
    private:
        SDL_Texture* getTexture(TextureView* textureView);
        SDL_Rect posImageToWindow(TextureView* textureView, SDL_Rect rectImage); //Converts only the XY positions from image to window coordinates
        SDL_Rect posWindowToImage(TextureView* textureView, SDL_Rect rectWindow); //Converts only the XY positions from window to image coordinates
        SDL_Rect posImageToViewer(TextureView* textureView, SDL_Rect rectImage); //Converts only the XY positions from image to viewer coordinates
        SDL_Rect posViewerToImage(TextureView* textureView, SDL_Rect rectViewer); //Converts only the XY positions from viewer to image coordinates
        void setX(TextureView* textureView, int x);
        void setY(TextureView* textureView, int y);
        void setW(TextureView* textureView, int w); //H is automatically adjusted to respect scale
        int getImageW(TextureView* textureView);
        int getImageH(TextureView* textureView);
        void emptyTextureViews();
        TextureView* getTextureView(void* uid);

    private:
        static int m_graphicsCard_maxW;//Maximal width of a texture that can fit in the graphics card
        static int m_graphicsCard_maxH;//Maximal height of a texture that can fit in the graphics card

        SDL_Renderer* m_renderer;

        int m_width;
        int m_height;
        float m_scaling;//Hoe much bigger the original image is to the current image (if resized to fit in the GPU)

        //This vector allows multiple viewers to each have there unique view of the same image while reducing memory usage by sharing the m_textureAll.
        std::vector<TextureView> m_textureViews;//This contains all view parameters of all users

        SDL_Texture* m_textureAll;//This version is the whole image or the biggest scaled down version that can fit in the graphics card's memory
};

#endif // IMAGE_HEADER
