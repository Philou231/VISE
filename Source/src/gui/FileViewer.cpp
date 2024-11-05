#include "FileViewer.h"

#include "misc/Utilities.h"


#define SCROLLBAR_W 20
#define BORDER_W 5


//TODO: Right Click to drag (pan) the list
//TODO: The scrollbar should be made into an object and an instance of it should be inside this file viewer
//TODO: Each file preview should be an instance of a FilePreview class
//TODO: Add a little label of ~10-15 pixels height on each preview to write the filename
//TODO: Add an X icon at the top right of each preview to close a file


FileViewer::FileViewer(SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, int w, int h) :
    Widget(renderer,parent,renderLevel,x,y,w,h),
    m_callbackFunction(NULL),
    m_callbackObjPtr(NULL),
    m_font(NULL), //Different from the button font because of the size
    m_files(NULL),
    m_previewBackground(NULL),
    m_borderIdle(NULL),
    m_borderOver(NULL),
    m_borderSelected(NULL),
    m_fileSelected(""),
    m_panRelCursorPos(0),
    m_lastFileCount(0),
    m_scrollBarBG(NULL),
    m_thumbIdleTexture(NULL),
    m_thumbOverTexture(NULL),
    m_thumbPressedTexture(NULL),
    m_currentThumbTexture(&m_thumbIdleTexture),
    m_filePreviewW(m_rectAbs.w-SCROLLBAR_W),
    m_filePreviewH(m_rectAbs.w-SCROLLBAR_W),
    m_clickedInPreview(false),
    m_scrollProgress(0),
    m_clickedInScrollbar(false)
{
    m_colorBG = Utilities::colorRGB(127,127,127);
    SDL_FillRect(m_surface,NULL,m_colorBG);

    SDL_Surface* tempSurface = SDL_CreateRGBSurface(0,1,1,32,0,0,0,0);//W=1 & H=1 because it will be stretched
    SDL_FillRect(tempSurface,NULL,Utilities::colorRGB(0,0,0));
    m_previewBackground = SDL_CreateTextureFromSurface(m_renderer,tempSurface);
    SDL_FreeSurface(tempSurface);

    tempSurface = SDL_CreateRGBSurface(0,1,1,32,0,0,0,0);//W=1 & H=1 because it will be stretched
    SDL_FillRect(tempSurface,NULL,Utilities::colorRGB(200,200,200));
    m_scrollBarBG = SDL_CreateTextureFromSurface(m_renderer,tempSurface);
    SDL_FreeSurface(tempSurface);

    FileViewer::redrawThumbs();
    FileViewer::redrawBorders();
}

void FileViewer::setFont(TTF_Font* font)
{
    m_font = font;
}

int FileViewer::maxListHeight()
{
    return 1 + m_files->size() * (m_filePreviewH+1);
}

void FileViewer::redrawThumbs()
{
    if(m_thumbIdleTexture!=NULL)
        SDL_DestroyTexture(m_thumbIdleTexture);
    if(m_thumbOverTexture!=NULL)
        SDL_DestroyTexture(m_thumbOverTexture);
    if(m_thumbPressedTexture!=NULL)
        SDL_DestroyTexture(m_thumbPressedTexture);


    m_rectScrollbar.w = SCROLLBAR_W-2;
    if(m_files!=NULL && m_files->size()>0)
        m_rectScrollbar.h = (float(m_rectAbs.h) / FileViewer::maxListHeight()) * (m_rectAbs.h-2);
    else
        m_rectScrollbar.h = m_rectAbs.h-2;

    if(m_rectScrollbar.h > m_rectAbs.h-2)
        m_rectScrollbar.h = m_rectAbs.h-2;
    if(m_rectScrollbar.h < 50)
        m_rectScrollbar.h = 50;


    SDL_Surface* thumbIdle = SDL_CreateRGBSurface(0,m_rectScrollbar.w,m_rectScrollbar.h,32,0,0,0,0);
    SDL_Surface* thumbOver = SDL_CreateRGBSurface(0,m_rectScrollbar.w,m_rectScrollbar.h,32,0,0,0,0);
    SDL_Surface* thumbPressed = SDL_CreateRGBSurface(0,m_rectScrollbar.w,m_rectScrollbar.h,32,0,0,0,0);

    SDL_FillRect(thumbIdle,NULL,Utilities::colorRGB(50,50,50));
    SDL_FillRect(thumbOver,NULL,Utilities::colorRGB(50,0,0));
    SDL_FillRect(thumbPressed,NULL,Utilities::colorRGB(100,0,0));


    //Draw three "grip" bars
    for(int y=m_rectScrollbar.h/2 -6;y<m_rectScrollbar.h/2+2 -6;y++)
    {
        for(int x=2;x<m_rectScrollbar.w-2;x++)
        {
            Utilities::setPixel(thumbIdle,x,y,Utilities::colorRGB(255,255,255));
            Utilities::setPixel(thumbOver,x,y,Utilities::colorRGB(255,255,255));
            Utilities::setPixel(thumbPressed,x,y,Utilities::colorRGB(255,255,255));
        }
    }

    for(int y=m_rectScrollbar.h/2;y<m_rectScrollbar.h/2+2;y++)
    {
        for(int x=2;x<m_rectScrollbar.w-2;x++)
        {
            Utilities::setPixel(thumbIdle,x,y,Utilities::colorRGB(255,255,255));
            Utilities::setPixel(thumbOver,x,y,Utilities::colorRGB(255,255,255));
            Utilities::setPixel(thumbPressed,x,y,Utilities::colorRGB(255,255,255));
        }
    }

    for(int y=m_rectScrollbar.h/2 +6;y<m_rectScrollbar.h/2+2 +6;y++)
    {
        for(int x=2;x<m_rectScrollbar.w-2;x++)
        {
            Utilities::setPixel(thumbIdle,x,y,Utilities::colorRGB(255,255,255));
            Utilities::setPixel(thumbOver,x,y,Utilities::colorRGB(255,255,255));
            Utilities::setPixel(thumbPressed,x,y,Utilities::colorRGB(255,255,255));
        }
    }


    m_thumbIdleTexture = SDL_CreateTextureFromSurface(m_renderer,thumbIdle);
    m_thumbOverTexture = SDL_CreateTextureFromSurface(m_renderer,thumbOver);
    m_thumbPressedTexture = SDL_CreateTextureFromSurface(m_renderer,thumbPressed);

    SDL_FreeSurface(thumbIdle);
    SDL_FreeSurface(thumbOver);
    SDL_FreeSurface(thumbPressed);
}

void FileViewer::redrawBorders()
{
    if(m_borderIdle!=NULL)
        SDL_DestroyTexture(m_borderIdle);
    if(m_borderOver!=NULL)
        SDL_DestroyTexture(m_borderOver);
    if(m_borderSelected!=NULL)
        SDL_DestroyTexture(m_borderSelected);

    SDL_Surface* surfaceBorderIdle = SDL_CreateRGBSurfaceWithFormat(0,m_filePreviewW,m_filePreviewH,32,SDL_PIXELFORMAT_BGRA32);
    SDL_Surface* surfaceBorderOver = SDL_CreateRGBSurfaceWithFormat(0,m_filePreviewW,m_filePreviewH,32,SDL_PIXELFORMAT_BGRA32);
    SDL_Surface* surfaceBorderSelected = SDL_CreateRGBSurfaceWithFormat(0,m_filePreviewW,m_filePreviewH,32,SDL_PIXELFORMAT_BGRA32);

    SDL_FillRect(surfaceBorderIdle,NULL,Utilities::colorRGBA(0,0,0,SDL_ALPHA_TRANSPARENT));
    SDL_FillRect(surfaceBorderOver,NULL,Utilities::colorRGBA(0,0,0,SDL_ALPHA_TRANSPARENT));
    SDL_FillRect(surfaceBorderSelected,NULL,Utilities::colorRGBA(0,0,0,SDL_ALPHA_TRANSPARENT));


    Uint32 colorIdle = Utilities::colorRGBA(50,50,50,SDL_ALPHA_OPAQUE);
    Uint32 colorOver = Utilities::colorRGBA(100,0,0,SDL_ALPHA_OPAQUE);
    Uint32 colorSelected = Utilities::colorRGBA(255,0,0,SDL_ALPHA_OPAQUE);

    for(int y=0;y<BORDER_W;y++)
    {
        for(int x=0;x<m_filePreviewW;x++)
        {
            Utilities::setPixel(surfaceBorderIdle,x,y,colorIdle);
            Utilities::setPixel(surfaceBorderOver,x,y,colorOver);
            Utilities::setPixel(surfaceBorderSelected,x,y,colorSelected);
        }
    }
    for(int y=m_filePreviewH-BORDER_W;y<m_filePreviewH;y++)
    {
        for(int x=0;x<m_filePreviewW;x++)
        {
            Utilities::setPixel(surfaceBorderIdle,x,y,colorIdle);
            Utilities::setPixel(surfaceBorderOver,x,y,colorOver);
            Utilities::setPixel(surfaceBorderSelected,x,y,colorSelected);
        }
    }

    for(int y=BORDER_W;y<m_filePreviewH-BORDER_W;y++)
    {
        for(int x=0;x<BORDER_W;x++)
        {
            Utilities::setPixel(surfaceBorderIdle,x,y,colorIdle);
            Utilities::setPixel(surfaceBorderOver,x,y,colorOver);
            Utilities::setPixel(surfaceBorderSelected,x,y,colorSelected);
        }
        for(int x=m_filePreviewW-BORDER_W;x<m_filePreviewW;x++)
        {
            Utilities::setPixel(surfaceBorderIdle,x,y,colorIdle);
            Utilities::setPixel(surfaceBorderOver,x,y,colorOver);
            Utilities::setPixel(surfaceBorderSelected,x,y,colorSelected);
        }
    }

    m_borderIdle = SDL_CreateTextureFromSurface(m_renderer,surfaceBorderIdle);
    m_borderOver = SDL_CreateTextureFromSurface(m_renderer,surfaceBorderOver);
    m_borderSelected = SDL_CreateTextureFromSurface(m_renderer,surfaceBorderSelected);
}

void FileViewer::linkToFileList(vector<File*>* files)
{
    m_files = files;
}

void FileViewer::setSelectionCallback(function<int (void*,void*)>* callbackFunction, void* objectPointer)
{
    m_callbackFunction = callbackFunction;
    m_callbackObjPtr = objectPointer;
}


void FileViewer::manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard)
{
    switch(event.type)
    {
        case SDL_MOUSEWHEEL:
            if(cursorIn(mouseAndKeyboard.mousePos) &&!m_clickedInScrollbar)
            {
                FileViewer::modifyProgress(-event.wheel.y*30);
                Widget::needGeneralDraw();
            }
            break;

        case SDL_MOUSEMOTION:
            if(m_clickedInScrollbar)
            {
                m_rectScrollbar.y = mouseAndKeyboard.mousePos.y - m_panRelCursorPos;
                if(m_rectScrollbar.y+m_rectScrollbar.h > m_rectAbs.y+m_rectAbs.h-1)
                    m_rectScrollbar.y = m_rectAbs.y+m_rectAbs.h-1-m_rectScrollbar.h;
                if(m_rectScrollbar.y < m_rectAbs.y+1)
                    m_rectScrollbar.y = m_rectAbs.y+1;

                if(m_files->size()>0 && m_rectAbs.h-2-m_rectScrollbar.h != 0)
                    m_scrollProgress = float(m_rectScrollbar.y-1-m_rectAbs.y) / (m_rectAbs.h-2-m_rectScrollbar.h) * (FileViewer::maxListHeight()-m_rectAbs.h);
                else
                    m_scrollProgress = 0;

                Widget::needGeneralDraw();
            }
            if(cursorIn(mouseAndKeyboard.mousePos, &m_rectScrollbar))
            {
                if(m_clickedInScrollbar)
                {
                    if(m_currentThumbTexture != &m_thumbPressedTexture)
                        Widget::needGeneralDraw();
                }
                else
                {
                    if(m_currentThumbTexture != &m_thumbOverTexture)
                        Widget::needGeneralDraw();
                }
            }
            else if(!m_clickedInScrollbar)
            {
                if(m_currentThumbTexture != &m_thumbIdleTexture)
                    Widget::needGeneralDraw();
            }

            if(m_clickedInPreview)
                updateSelection(mouseAndKeyboard.mousePos);

            if(cursorIn(mouseAndKeyboard.mousePos) || cursorIn(m_lastMousePos))
                Widget::needGeneralDraw();//To update the preview borders

            m_lastMousePos = mouseAndKeyboard.mousePos;
            break;

        case SDL_MOUSEBUTTONDOWN:
            if(cursorIn(mouseAndKeyboard.mousePos, &m_rectScrollbar))//Click on scrollbar
            {
                m_clickedInScrollbar=true;
                SDL_CaptureMouse(SDL_TRUE);
                m_panRelCursorPos = mouseAndKeyboard.mousePos.y - m_rectScrollbar.y;
                m_currentThumbTexture = &m_thumbPressedTexture;
                Widget::needGeneralDraw();
            }
            else if(cursorIn(mouseAndKeyboard.mousePos))//click on preview
            {
                m_clickedInPreview=true;
                FileViewer::updateSelection(mouseAndKeyboard.mousePos);
            }
            break;

        case SDL_MOUSEBUTTONUP:
            SDL_CaptureMouse(SDL_FALSE);
            if(cursorIn(mouseAndKeyboard.mousePos, &m_rectScrollbar))
            {
                m_currentThumbTexture = &m_thumbOverTexture;
                Widget::needGeneralDraw();
            }
            else if(m_clickedInScrollbar)
            {
                m_currentThumbTexture = &m_thumbIdleTexture;
                Widget::needGeneralDraw();
            }
            m_clickedInScrollbar=false;
            m_clickedInPreview=false;
            break;
    }

    Widget::manageEvent(event, mouseAndKeyboard);
}

void FileViewer::selectFile(File* file)
{
    if(file==NULL)
        return;

    m_fileSelected = file->getPath();
}

int FileViewer::selectFileAndCallback(File* file)
{
    if(file==NULL)
        return 1;

    if(m_callbackFunction==NULL)
        return 2;

    m_fileSelected = file->getPath();
    return (*m_callbackFunction)(m_callbackObjPtr,file);
}

File* FileViewer::selected()
{
    for(size_t i=0;i<m_files->size();i++)
    {
        if(m_fileSelected == (*m_files)[i]->getPath())
            return (*m_files)[i];
    }

    return NULL;
}

Image* FileViewer::getNextImage(string currentImagePath)
{
    for(size_t i=0;i<m_files->size();i++)
    {
        if(currentImagePath == (*m_files)[i]->getPath())
        {
            for(size_t j=i+1;j<m_files->size();j++)
            {
                if((*m_files)[j]->getType()==FILE_IMAGE)
                    return (Image*)((*m_files)[j]);
            }
            //Not found, lets loop back
            for(size_t j=0;j<m_files->size();j++)
            {
                if((*m_files)[j]->getType()==FILE_IMAGE)
                    return (Image*)((*m_files)[j]);
            }
        }
    }

    return NULL;
}

void FileViewer::modifyProgress(int progressChange)
{
    m_scrollProgress += progressChange;

    if(m_scrollProgress > FileViewer::maxListHeight() - m_rectAbs.h)
        m_scrollProgress = FileViewer::maxListHeight() - m_rectAbs.h;
    if(m_scrollProgress<0)
        m_scrollProgress=0;
}

void FileViewer::computeListRect(int index, SDL_Rect* rectSrc, SDL_Rect* rectDst)
{
    SDL_Rect returnedSrc,returnedDst;

    returnedSrc.x=0;
    returnedSrc.y=0;
    returnedSrc.w=m_filePreviewW;
    returnedSrc.h=m_filePreviewH;

    returnedDst.x=m_rectAbs.x;
    returnedDst.y=m_rectAbs.y;
    returnedDst.w=m_filePreviewW;
    returnedDst.h=m_filePreviewH;

    returnedDst.y = 1 + m_rectAbs.y + index*(m_filePreviewH+1) - m_scrollProgress;

    Utilities::keepInsideRect(&returnedSrc, &returnedDst, m_rectAbs);

    if(rectSrc!=NULL)
        *rectSrc = returnedSrc;
    if(rectDst!=NULL)
        *rectDst = returnedDst;
}

void FileViewer::updateSelection(SDL_Rect mousePos)
{
    for(size_t i=0;i<m_files->size();i++)
    {
        SDL_Rect tempDstRect;
        computeListRect(i, NULL, &tempDstRect);
        if(cursorIn(mousePos, &tempDstRect))
        {
            string newSelection = (*m_files)[i]->getPath();

            if(newSelection != m_fileSelected)
            {
                FileViewer::selectFileAndCallback((*m_files)[i]);
                Widget::needGeneralDraw();
            }
        }
    }
}


SDL_Texture* FileViewer::getFilePreview(File* file, SDL_Rect srcRectComputed, SDL_Rect dstRectComputed, SDL_Rect* srcRect, SDL_Rect* dstRect)
{
    if(file->getType()==FILE_IMAGE)
    {
        Image* image = (Image*)file;

        //Compute the normal rects
        srcRect->x = 0;
        srcRect->y = 0;
        srcRect->w = image->getW();
        srcRect->h = image->getH();

        if(image->getW() > (m_filePreviewW-BORDER_W*2) * image->getH() / (m_filePreviewH-BORDER_W*2))
        {
            dstRect->w = m_filePreviewW - BORDER_W*2;
            dstRect->h = image->getH() * dstRect->w / image->getW();
        }
        else
        {
            dstRect->h = m_filePreviewH - BORDER_W*2;
            dstRect->w = image->getW() * dstRect->h / image->getH();
        }
        dstRect->x = (m_filePreviewW-dstRect->w)/2 + dstRectComputed.x;
        dstRect->y = (m_filePreviewH-dstRect->h)/2;

        //Adjust if too low or too high
        if(dstRectComputed.h < m_filePreviewH)
        {
            if(srcRectComputed.y > 0) //Too high
            {
                if(srcRectComputed.y > dstRect->y)
                {
                    int gap = srcRectComputed.y-dstRect->y;

                    srcRect->y = gap * srcRect->h / dstRect->h;
                    srcRect->h = (dstRect->h-gap) * srcRect->h / dstRect->h;

                    dstRect->y = dstRectComputed.y;
                    dstRect->h -= gap;
                }
                else
                {
                    dstRect->y += dstRectComputed.y - srcRectComputed.y;
                }
            }
            else //Too low
            {
                dstRect->y += dstRectComputed.y;

                if(dstRectComputed.y+dstRectComputed.h < dstRect->y+dstRect->h)
                {
                    int gap = (dstRect->y+dstRect->h) - (dstRectComputed.y+dstRectComputed.h);

                    srcRect->h = (dstRect->h-gap) * srcRect->h / dstRect->h;

                    dstRect->h -= gap;
                }
            }
        }
        else
        {
             dstRect->y += dstRectComputed.y;
        }


        return image->getTexture(this);
    }

    for(size_t i=0;i<m_filePreviews.size();i++)
    {
        if(file->getPath() == m_filePreviews[i].first)
        {
            if(srcRect!=NULL)
            {
                (*srcRect) = srcRectComputed;
            }
            if(dstRect!=NULL)
            {
                (*dstRect) = dstRectComputed;
            }
            return m_filePreviews[i].second;
        }
    }


    SDL_Texture* previewTexture=NULL;
    if(file->getType()==FILE_PICKANDPLACE)
        previewTexture = FileViewer::createPreviewPickAndPlace((PickAndPlace*)file);

    pair<string, SDL_Texture*> filePreview={file->getPath(),previewTexture};
    m_filePreviews.push_back(filePreview);


    return previewTexture;
}

SDL_Texture* FileViewer::createPreviewPickAndPlace(PickAndPlace* pickAndPlace)
{
    SDL_Color fgColor={255,255,255};
    SDL_Color bgColor={0,0,100};

    SDL_Surface* returnedSurface = SDL_CreateRGBSurface(0,m_filePreviewW,m_filePreviewH,32,0,0,0,0);
    SDL_FillRect(returnedSurface,NULL,Utilities::colorRGB(bgColor.r,bgColor.g,bgColor.b));


    SDL_Surface* textPickAndPlace = TTF_RenderUTF8_Shaded(m_font,"Pick & Place",fgColor,bgColor);
    SDL_Surface* textComponents = TTF_RenderUTF8_Shaded(m_font,(to_string(pickAndPlace->count())+string(" units")).c_str(),fgColor,bgColor);

    SDL_Rect dstRect;
    dstRect.x = (returnedSurface->w - textPickAndPlace->w)/2;
    dstRect.y = returnedSurface->h/2 - textPickAndPlace->h;
    dstRect.w = textPickAndPlace->w;
    dstRect.h = textPickAndPlace->h;

    SDL_BlitSurface(textPickAndPlace,NULL,returnedSurface,&dstRect);
    SDL_FreeSurface(textPickAndPlace);

    dstRect.x = (returnedSurface->w - textComponents->w)/2;
    dstRect.y = returnedSurface->h/2;
    dstRect.w = textComponents->w;
    dstRect.h = textComponents->h;

    SDL_BlitSurface(textComponents,NULL,returnedSurface,&dstRect);
    SDL_FreeSurface(textComponents);

    SDL_Texture* returnedTexture = SDL_CreateTextureFromSurface(m_renderer,returnedSurface);
    return returnedTexture;
}

void FileViewer::draw(SDL_Surface* surface, uint8_t renderLevel)
{
    if(renderLevel == m_renderLevel)
    {
        //Draw the background
        SDL_RenderCopy(m_renderer, m_texture, NULL, &m_rectAbs);

        if(m_files==NULL)
            return;


        //Draw the previews
        for(size_t i=0;i<m_files->size();i++)
        {
            SDL_Rect tempSrcRect, tempDstRect;
            FileViewer::computeListRect(i, &tempSrcRect, &tempDstRect);
            FileType fileType = (*m_files)[i]->getType();

            if(tempDstRect.y+tempDstRect.h < m_rectAbs.y || tempDstRect.y>=m_rectAbs.y+m_rectAbs.h || //Out of screen
              (fileType!=FILE_IMAGE && fileType!=FILE_PICKANDPLACE)) //Unknown file type
            {
                continue;
            }


            //Draw the background
            SDL_RenderCopy(m_renderer, m_previewBackground, NULL, &tempDstRect);

            //Draw the preview
            SDL_Rect srcRect, dstRect;
            SDL_Texture* texture = FileViewer::getFilePreview((*m_files)[i], tempSrcRect, tempDstRect, &srcRect, &dstRect);
            SDL_RenderCopy(m_renderer, texture, &srcRect, &dstRect);

            //Draw the border
            if(m_fileSelected == (*m_files)[i]->getPath())
                SDL_RenderCopy(m_renderer, m_borderSelected, &tempSrcRect, &tempDstRect);
            else if(cursorIn(m_lastMousePos,&tempDstRect))
                SDL_RenderCopy(m_renderer, m_borderOver, &tempSrcRect, &tempDstRect);
            else
                SDL_RenderCopy(m_renderer, m_borderIdle, &tempSrcRect, &tempDstRect);
        }


        //Draw the scrollbar
        if(m_lastFileCount!=m_files->size())
        {
            FileViewer::redrawThumbs();
            m_lastFileCount = m_files->size();
        }

        float scrollProgressFloat=0.0;
        if(m_files->size()>0)
            scrollProgressFloat = float(m_scrollProgress)/(FileViewer::maxListHeight() - m_rectAbs.h);

        SDL_Rect tempDstRect;
        tempDstRect.x=m_rectAbs.x+m_rectAbs.w-SCROLLBAR_W;
        tempDstRect.y=m_rectAbs.y;
        tempDstRect.w=SCROLLBAR_W;
        tempDstRect.h=m_rectAbs.h;
        SDL_RenderCopy(m_renderer, m_scrollBarBG, NULL, &tempDstRect);


        if(m_rectScrollbar.h < m_rectAbs.h-2)
        {
            m_rectScrollbar.x = m_rectAbs.x + m_rectAbs.w - m_rectScrollbar.w - 1;
            m_rectScrollbar.y = m_rectAbs.y+1 + scrollProgressFloat*(m_rectAbs.h - m_rectScrollbar.h - 2);


            if(m_clickedInScrollbar)
            {
                m_currentThumbTexture = &m_thumbPressedTexture;
            }
            else
            {
                if(cursorIn(m_lastMousePos, &m_rectScrollbar))
                    m_currentThumbTexture = &m_thumbOverTexture;
                else
                    m_currentThumbTexture = &m_thumbIdleTexture;
            }
            SDL_RenderCopy(m_renderer, *m_currentThumbTexture, NULL, &m_rectScrollbar);
        }
    }
}

void FileViewer::recomputeAbsRect(SDL_Rect* windowRect/*=NULL*/)
{
    Widget::recomputeAbsRect(windowRect);
    m_filePreviewW = m_rectAbs.w-SCROLLBAR_W;
    m_filePreviewH = m_filePreviewW;


    Widget::redraw();
    if(m_texture!=NULL)
        SDL_DestroyTexture(m_texture);
    m_texture = SDL_CreateTextureFromSurface(m_renderer,m_surface);

    FileViewer::modifyProgress(0);
    FileViewer::redrawThumbs();
    FileViewer::redrawBorders();
}


FileViewer::~FileViewer()
{
    if(m_thumbPressedTexture!=NULL)
        SDL_DestroyTexture(m_thumbPressedTexture);
    if(m_thumbOverTexture!=NULL)
        SDL_DestroyTexture(m_thumbOverTexture);
    if(m_thumbIdleTexture!=NULL)
        SDL_DestroyTexture(m_thumbIdleTexture);
    if(m_scrollBarBG!=NULL)
        SDL_DestroyTexture(m_scrollBarBG);

    if(m_borderSelected!=NULL)
        SDL_DestroyTexture(m_borderSelected);
    if(m_borderOver!=NULL)
        SDL_DestroyTexture(m_borderOver);
    if(m_borderIdle!=NULL)
        SDL_DestroyTexture(m_borderIdle);
    if(m_previewBackground!=NULL)
        SDL_DestroyTexture(m_previewBackground);

    for(size_t i=0;i<m_filePreviews.size();i++)
    {
        SDL_DestroyTexture(m_filePreviews[i].second);
    }
}





