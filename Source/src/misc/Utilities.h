#ifndef UTILITIES_HEADER
#define UTILITIES_HEADER

#include <SDL.h>
#include <iostream>
#include <vector>

using namespace std;


#define PI 3.1415926535897932384626433


struct Dbl_Rect{
    double x=0.0;
    double y=0.0;
    double w=0.0;
    double h=0.0;
};


class Utilities
{
    public:
        static std::string convertBackSlashesToFront(string stringWithSlashes);
        static std::string getFileName(std::string path);
        static std::string getBasePath();
        static std::string res_fonts(std::string path="");
        static std::string res_images(std::string path="");
        static std::string toLower(std::string text);

        static std::vector<std::string> csvLineToCells(std::string csvLine, char separator);

        static void printRect(SDL_Rect rect);
        static Dbl_Rect toDblRect(SDL_Rect rect);
        static void keepInsideRect(SDL_Rect* srcRect, SDL_Rect* dstRect, SDL_Rect boundaryRect);

        static double computeLength(Dbl_Rect coord1, Dbl_Rect coord2);
        static double computeAngle(Dbl_Rect coord1, Dbl_Rect coord2);

        static Uint32 colorRGBA(Uint8 r, Uint8 g, Uint8 b,Uint8 a);
        static Uint32 colorRGB(Uint8 r, Uint8 g, Uint8 b);
        static Uint8 R(Uint32 rgba);
        static Uint8 G(Uint32 rgba);
        static Uint8 B(Uint32 rgba);
        static Uint8 A(Uint32 rgba);
        static SDL_Color toColor(Uint32 color);
        static void setPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
        static Uint32 getPixel(SDL_Surface *surface, int x, int y);
};


#endif // UTILITIES_HEADER
