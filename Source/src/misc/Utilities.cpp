#include "misc/Utilities.h"
#include <algorithm>
#include <cmath>
#include <sstream>


string Utilities::convertBackSlashesToFront(string stringWithSlashes)
{
    replace(stringWithSlashes.begin(), stringWithSlashes.end(), '\\', '/');
    return stringWithSlashes;
}

string Utilities::getFileName(string path)
{
    size_t position = path.find_last_of("/\\");
    if(position==string::npos)
        return "";
    else
        return path.substr(position+1);
}

string Utilities::getBasePath()
{
    char* base_path = SDL_GetBasePath();
    string returnedString;

    if(base_path!=NULL)
    {
        returnedString = base_path;
    }
    else
    {
        cerr <<"Couldn't get base path: \"" <<SDL_GetError() <<"\"" <<endl;
        returnedString = "./";
    }

    SDL_free(base_path);

    return convertBackSlashesToFront(returnedString);
}

string res(string resType, string path)
{
#ifdef __WIN32__
    return Utilities::getBasePath()+resType+"/"+path;
#else
    return resType+"/"+path;
#endif
}

string Utilities::res_fonts(string path)
{
    return res("res_fonts",path);
}

string Utilities::res_images(string path)
{
    return res("res_images",path);
}

string Utilities::toLower(string text)
{
    string returnedString="";
    for(size_t i=0;i<text.size();i++)
    {
        returnedString += tolower(text[i]);
    }

    return returnedString;
}


vector<string> Utilities::csvLineToCells(string csvLine, char separator)
{
    vector<string> lineVector;

    stringstream lineStream(csvLine);

    char nextSeparator=separator;
    if(csvLine.size()>0 && csvLine[0]=='"')
        nextSeparator='\"';

    bool append=false;
    string token;
    while(getline(lineStream, token, nextSeparator))
    {
        if(append)
            lineVector[lineVector.size()-1] += token;
        else
            lineVector.push_back(token);

        if(lineStream.peek()=='"')
        {
            if(nextSeparator=='\"')//We were looking for a double quote but found two in a row. This is a double quote within a cell
            {
                append=true;
                lineVector[lineVector.size()-1] += '\"'; //Add one of the two double quotes that were destroyed
            }

            lineStream.get(); //Destroy the opening double quotes character
            nextSeparator='\"';
        }
        else
        {
            if(nextSeparator=='\"') //We just found the end of the quotes-enclosed cell
                lineStream.get(); //Destroy the separating comma that was not consumed by the getline

            append=false;
            nextSeparator=separator;
        }
    }

    return lineVector;
}


void Utilities::printRect(SDL_Rect rect)
{
    cout <<"{" <<rect.x <<", " <<rect.y <<", " <<rect.w <<", " <<rect.h <<"}" <<endl;
}

Dbl_Rect Utilities::toDblRect(SDL_Rect rect)
{
    Dbl_Rect dblRect;
    dblRect.x = rect.x;
    dblRect.y = rect.y;
    dblRect.w = rect.w;
    dblRect.h = rect.h;

    return dblRect;
}

void Utilities::keepInsideRect(SDL_Rect* srcRect, SDL_Rect* dstRect, SDL_Rect boundaryRect)
{
    if(dstRect->x<boundaryRect.x)
    {
        srcRect->x = boundaryRect.x-dstRect->x;
        srcRect->w -= srcRect->x;
        dstRect->x = boundaryRect.x;
        dstRect->w = srcRect->w;
    }
    else if(dstRect->x+dstRect->w>boundaryRect.x+boundaryRect.w)
    {
        srcRect->w -= (dstRect->x+dstRect->w)-(boundaryRect.x+boundaryRect.w);
        dstRect->w = srcRect->w;
    }
    if(dstRect->y<boundaryRect.y)
    {
        srcRect->y = boundaryRect.y-dstRect->y;
        srcRect->h -= srcRect->y;
        dstRect->y = boundaryRect.y;
        dstRect->h = srcRect->h;
    }
    else if(dstRect->y+dstRect->h>boundaryRect.y+boundaryRect.h)
    {
        srcRect->h -= (dstRect->y+dstRect->h)-(boundaryRect.y+boundaryRect.h);
        dstRect->h = srcRect->h;
    }
}



double Utilities::computeLength(Dbl_Rect coord1, Dbl_Rect coord2)
{
    return sqrt( pow(coord2.x-coord1.x, 2) + pow(coord2.y-coord1.y, 2));
}

double Utilities::computeAngle(Dbl_Rect coord1, Dbl_Rect coord2)
{
    double angle = atan2(coord2.y-coord1.y, coord2.x-coord1.x) * 180 / PI;
    if(angle<0)
        angle += 360;
    return angle;
}



Uint32 Utilities::colorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    return Uint32(a)*0x1000000+Uint32(r)*0x10000+Uint32(g)*0x100+Uint32(b);
}

Uint32 Utilities::colorRGB(Uint8 r, Uint8 g, Uint8 b)
{
    return Uint32(SDL_ALPHA_OPAQUE)*0x1000000+Uint32(r)*0x10000+Uint32(g)*0x100+Uint32(b);
}

Uint8 Utilities::R(Uint32 rgba)
{
    return (rgba&0x00ff0000)>>(8*2);
}

Uint8 Utilities::G(Uint32 rgba)
{
    return (rgba&0x0000ff00)>>(8*1);
}

Uint8 Utilities::B(Uint32 rgba)
{
    return (rgba&0x000000ff)>>(8*0);
}

Uint8 Utilities::A(Uint32 rgba)
{
    return (rgba&0xff000000)>>(8*3);
}

SDL_Color Utilities::toColor(Uint32 color)
{
    SDL_Color sdlColor;
    sdlColor.r = Utilities::R(color);
    sdlColor.g = Utilities::G(color);
    sdlColor.b = Utilities::B(color);
    sdlColor.a = Utilities::A(color);

    return sdlColor;
}

void Utilities::setPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

Uint32 Utilities::getPixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;
        break;

    case 2:
        return *(Uint16 *)p;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;
        break;

    case 4:
        return *(Uint32 *)p;
        break;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}
