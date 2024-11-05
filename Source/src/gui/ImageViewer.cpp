#include "ImageViewer.h"

#include <cmath>
#include <sstream>
#include <iomanip>
#include <SDL_image.h>

#include "misc/Utilities.h"



ImageViewer::ImageViewer(SDL_Window* window, SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, int w, int h, InspectionReport* inspectionReport, FileViewer* fileViewer) :
    Widget(renderer,parent,renderLevel,x,y,w,h),
    m_callbackFunction(NULL),
    m_window(window),
    m_image(NULL),
    m_popUpCalibration(NULL),
    m_popUpComment(NULL),
    m_popUpYesNo(NULL),
    m_tooltipSelected(m_renderer,this,renderLevel,0,0,CENTER_LEFT),
    m_tooltipOver(m_renderer,this,renderLevel,0,0,CENTER_LEFT),
    m_tooltipMeasurement(m_renderer,this,renderLevel,0,0,CENTER_LEFT),
    m_textureCalibMarker(NULL),
    m_textureCalibComponentMarker(NULL),
    m_textureUndefinedComponentMarker(NULL),
    m_textureFittedComponentMarker(NULL),
    m_textureNotFittedComponentMarker(NULL),
    m_textureErrorComponentMarker(NULL),
    m_textureCommentMarker(NULL),
    m_textureMeasureMarker(NULL),
    m_textureZoomCursor(NULL),
    m_toolState(TOOL_NONE),
    m_panning(false),
    m_zooming(false),
    m_clicking(false),
    m_cursorIsIn(false),
    m_cursorWasIn(false),
    m_measurementFirstSet(false),
    m_visibleItems(VIS_MEASURE|VIS_COMMENT|VIS_FITTED|VIS_NOT_FITTED|VIS_ERROR|VIS_UNDEFINED),
    m_workspace(WORKSPACE_DEFAULT),
    m_inspectionReport(inspectionReport),
    m_itemOver(""),
    m_itemSelected(""),
    m_fileViewer(fileViewer),
    m_previousCursor(NULL),
    m_currentCursor(NULL),
    m_cursorArrow(NULL),
    m_cursorCalibration(NULL),
    m_cursorInspection(NULL),
    m_cursorPan(NULL),
    m_cursorZoom(NULL),
    m_cursorComment(NULL),
    m_cursorMeasure(NULL)
{
    m_colorBG = Utilities::colorRGB(200,200,200);
    SDL_FillRect(m_surface,NULL,m_colorBG);

    m_tooltipSelected.setColor(Utilities::colorRGB(255,255,255),Utilities::colorRGBA(0,0,0,127));
    m_tooltipSelected.setDelayMs(0);//Never disapear
    m_tooltipOver.setColor(Utilities::colorRGB(255,255,255),Utilities::colorRGBA(0,0,0,127));
    m_tooltipOver.setDelayMs(0);//Never disapear
    m_tooltipMeasurement.setColor(Utilities::colorRGB(255,255,255),Utilities::colorRGBA(255,87,255,127));
    m_tooltipMeasurement.setDelayMs(0);//Never disapear

    SDL_Surface* surfaceCalibMarker = ImageViewer::createCrossMarkerSurface(255,0,0);
    m_textureCalibMarker = SDL_CreateTextureFromSurface(m_renderer,surfaceCalibMarker);

    SDL_Surface* surfaceCalibComponentMarker = IMG_Load(Utilities::res_images("calibCompMarker.png").c_str());
    m_textureCalibComponentMarker = SDL_CreateTextureFromSurface(m_renderer,surfaceCalibComponentMarker);
    SDL_FreeSurface(surfaceCalibComponentMarker);
    SDL_Surface* surfaceUndefinedComponentMarker = IMG_Load(Utilities::res_images("undefinedCompMarker.png").c_str());
    m_textureUndefinedComponentMarker = SDL_CreateTextureFromSurface(m_renderer,surfaceUndefinedComponentMarker);
    SDL_FreeSurface(surfaceUndefinedComponentMarker);
    SDL_Surface* surfaceFittedComponentMarker = IMG_Load(Utilities::res_images("fittedCompMarker.png").c_str());
    m_textureFittedComponentMarker = SDL_CreateTextureFromSurface(m_renderer,surfaceFittedComponentMarker);
    SDL_FreeSurface(surfaceFittedComponentMarker);
    SDL_Surface* surfaceNotFittedComponentMarker = IMG_Load(Utilities::res_images("notFittedCompMarker.png").c_str());
    m_textureNotFittedComponentMarker = SDL_CreateTextureFromSurface(m_renderer,surfaceNotFittedComponentMarker);
    SDL_FreeSurface(surfaceNotFittedComponentMarker);
    SDL_Surface* surfaceErrorComponentMarker = IMG_Load(Utilities::res_images("errorCompMarker.png").c_str());
    m_textureErrorComponentMarker = SDL_CreateTextureFromSurface(m_renderer,surfaceErrorComponentMarker);
    SDL_FreeSurface(surfaceErrorComponentMarker);
    SDL_Surface* surfaceCommentMarker = IMG_Load(Utilities::res_images("commentMarker.png").c_str());
    SDL_SetSurfaceColorMod(surfaceCommentMarker, 255, 255, 0);
    m_textureCommentMarker = SDL_CreateTextureFromSurface(m_renderer,surfaceCommentMarker);
    SDL_BlitSurface(surfaceCommentMarker,NULL,surfaceCommentMarker,NULL);
    SDL_Surface* surfaceMeasureMarker = ImageViewer::createCrossMarkerSurface(255,255,255);
    SDL_SetSurfaceColorMod(surfaceMeasureMarker, 255, 87, 255);
    m_textureMeasureMarker = SDL_CreateTextureFromSurface(m_renderer,surfaceMeasureMarker);
    SDL_BlitSurface(surfaceMeasureMarker,NULL,surfaceMeasureMarker,NULL);

    SDL_SetTextureColorMod(m_textureUndefinedComponentMarker,255,127,  0);
    SDL_SetTextureColorMod(m_textureFittedComponentMarker   ,  0,255,  0);
    SDL_SetTextureColorMod(m_textureNotFittedComponentMarker, 35,104,255);
    SDL_SetTextureColorMod(m_textureErrorComponentMarker    ,255,  0,  0);

    SDL_Surface* surfaceZoom = IMG_Load(Utilities::res_images("cursorZoom.png").c_str());
    SDL_Surface* surfacePan = IMG_Load(Utilities::res_images("cursorPan.png").c_str());

    m_textureZoomCursor = SDL_CreateTextureFromSurface(m_renderer,surfaceZoom);

    m_cursorArrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    m_cursorInspection = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    m_cursorCalibration = SDL_CreateColorCursor(surfaceCalibMarker,surfaceMeasureMarker->w/2,surfaceMeasureMarker->h/2);
    m_cursorPan = SDL_CreateColorCursor(surfacePan,7,13);
    m_cursorZoom = SDL_CreateColorCursor(surfaceZoom,6,13);
    m_cursorComment = SDL_CreateColorCursor(surfaceCommentMarker,24,25);
    m_cursorMeasure = SDL_CreateColorCursor(surfaceMeasureMarker,surfaceMeasureMarker->w/2,surfaceMeasureMarker->h/2);
    m_previousCursor = &m_cursorArrow;

    SDL_FreeSurface(surfacePan);
    SDL_FreeSurface(surfaceZoom);
    SDL_FreeSurface(surfaceCommentMarker);
    SDL_FreeSurface(surfaceMeasureMarker);
    SDL_FreeSurface(surfaceCalibMarker);
}

void ImageViewer::setCallback(function<int (CallbackMessages,void*)>* callbackFunction)
{
    m_callbackFunction = callbackFunction;
}

void ImageViewer::manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard)
{
    const bool ctrlPressed = mouseAndKeyboard.keyboardState[SDL_SCANCODE_LCTRL] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RCTRL];
    const bool shiftPressed = mouseAndKeyboard.keyboardState[SDL_SCANCODE_LSHIFT] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RSHIFT];

    m_cursorIsIn = cursorIn(mouseAndKeyboard.mousePos);

    switch(event.type) //Actions that needs to happen even if the cursor is out of the viewer
    {
        case SDL_MOUSEBUTTONUP:
            switch(event.button.button)
            {
                case SDL_BUTTON_LEFT:
                    if(!m_cursorIsIn)
                        m_clicking = false;
                    break;

                case SDL_BUTTON_MIDDLE:
                    if(!m_panning)
                    {
                        SDL_CaptureMouse(SDL_FALSE);
                        ImageViewer::captureCursor(NULL);

                        if(m_cursorIsIn)
                            ImageViewer::resetCursor();
                        else
                            ImageViewer::setCursor(CURSOR_ARROW);
                        m_zooming=false;

                        SDL_WarpMouseInWindow(m_window, m_panZoom_originalCursorPos.x, m_panZoom_originalCursorPos.y);
                        ImageViewer::needRedraw();
                    }
                    break;

                case SDL_BUTTON_RIGHT:
                    if(!m_zooming)
                    {
                        SDL_CaptureMouse(SDL_FALSE);
                        ImageViewer::captureCursor(NULL);

                        if(m_cursorIsIn)
                            ImageViewer::resetCursor();
                        else
                            ImageViewer::setCursor(CURSOR_ARROW);
                        m_panning=false;
                    }
                    break;
            }
            break;

        case SDL_MOUSEMOTION:
            if(m_panning)
            {
                m_image->setPos(this, mouseAndKeyboard.mousePos.x-m_panZoom_originalCursorPos.x+m_panZoom_originalImageRect.x,
                                      mouseAndKeyboard.mousePos.y-m_panZoom_originalCursorPos.y+m_panZoom_originalImageRect.y);

                m_updateMarkers = true;
                Widget::needGeneralDraw();
            }
            else if(m_zooming)
            {
                m_image->setZoom(this, m_panZoom_originalImageRect, m_panZoom_originalCursorPos, mouseAndKeyboard.mousePos);

                m_updateMarkers = true;
                Widget::needGeneralDraw();
            }
            else
            {
                if(m_cursorIsIn && !m_cursorWasIn)
                    ImageViewer::resetCursor();
                else if(!m_cursorIsIn && m_cursorWasIn)
                {
                    ImageViewer::setCursor(CURSOR_ARROW);
                    ImageViewer::needRedraw(); //For the tooltip over
                }
            }

            m_lastCursorPos=mouseAndKeyboard.mousePos;
            break;

        case SDL_KEYDOWN:
            switch(event.key.keysym.scancode)
            {
                case SDL_SCANCODE_ESCAPE:
                    if(m_toolState==TOOL_MEASUREMENT)
                    {
                        if(m_measurementFirstSet)
                        {
                            m_measurementFirstSet=false;
                            Widget::needGeneralDraw();
                        }
                        else
                        {
                            ImageViewer::disableMeasurement();
                        }
                    }
                    else if(m_toolState==TOOL_COMMENT)
                    {
                        ImageViewer::disableCommenting();
                    }
                    else if(m_toolState==TOOL_NONE)
                    {
                        if(m_workspace == WORKSPACE_INSPECTION)
                        {
                            m_itemSelected = "";
                            ImageViewer::needRedraw();
                        }
                    }
                    break;

                case SDL_SCANCODE_DELETE:
                    if(m_itemSelected != "")
                        ImageViewer::deleteItem(m_itemSelected);
                    break;

                case SDL_SCANCODE_R:
                    m_image->resetView(this);
                    m_updateMarkers = true;
                    Widget::needGeneralDraw();
                    break;

                case SDL_SCANCODE_0:
                case SDL_SCANCODE_KP_0:
                    m_image->showAll(this);
                    m_updateMarkers = true;
                    Widget::needGeneralDraw();
                    break;

                case SDL_SCANCODE_1:
                case SDL_SCANCODE_KP_1:
                    m_image->showPixelPerfect(this);
                    m_updateMarkers = true;
                    Widget::needGeneralDraw();
                    break;

                default:
                    break;
            }
    }



    if(m_cursorIsIn && m_image!=NULL)
    {
        switch(event.type)
        {
            case SDL_MOUSEMOTION:
                if(m_workspace==WORKSPACE_INSPECTION)
                {
                    if(m_toolState==TOOL_MEASUREMENT)
                        needGeneralDraw(); //For the measurement tooltip
                    else
                    {
                        bool foundIt=false;
                        for(size_t i=0;i<m_itemsVisible.size();i++)
                        {
                            if(cursorIn(mouseAndKeyboard.mousePos,&(m_itemsVisible[i].second)))
                            {
                                foundIt = true;
                                if(m_itemOver != m_itemsVisible[i].first)
                                {
                                    m_itemOver = m_itemsVisible[i].first;
                                    if(m_itemOver.substr(0,5) == "VISE_")
                                        m_tooltipOver.setText(m_itemOver.substr(5));
                                    else
                                        m_tooltipOver.setText(m_itemOver);
                                }
                                needGeneralDraw();
                                break;
                            }
                        }
                        if(!foundIt && m_itemOver != "")
                        {
                            m_itemOver="";
                            needGeneralDraw();
                        }
                    }
                }
                else
                    m_itemOver="";
                break;

            case SDL_MOUSEBUTTONDOWN:
                switch(event.button.button)
                {
                    case SDL_BUTTON_LEFT:
                        if(!m_panning && !m_zooming)
                        {
                            m_clicking = true;
                        }
                        break;

                    case SDL_BUTTON_MIDDLE:
                        if(!m_clicking && !m_panning)
                        {
                            SDL_ShowCursor(false);
                            m_panZoom_originalCursorPos=mouseAndKeyboard.mousePos;
                            m_panZoom_originalImageRect=m_image->getRect(this);
                            SDL_CaptureMouse(SDL_TRUE);
                            ImageViewer::captureCursor(this);
                            m_zooming=true;
                            ImageViewer::needRedraw();
                        }
                        break;

                    case SDL_BUTTON_RIGHT:
                        if(!m_clicking && !m_zooming)
                        {
                            ImageViewer::setCursor(CURSOR_PAN);
                            m_panZoom_originalCursorPos=mouseAndKeyboard.mousePos;
                            m_panZoom_originalImageRect=m_image->getRect(this);
                            SDL_CaptureMouse(SDL_TRUE);
                            ImageViewer::captureCursor(this);
                            m_panning=true;
                        }
                        break;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                switch(event.button.button)
                {
                    case SDL_BUTTON_LEFT:
                        if(m_clicking)
                        {
                            m_clicking=false;
                            if(m_workspace==WORKSPACE_CALIBRATION)
                            {
                                string designator;
                                if(m_popUpCalibration->prompt(&designator,"")==POPUP_OK)
                                {
                                    SDL_Rect clickPosition = m_image->coordWindowToImage(this, mouseAndKeyboard.mousePos);
                                    m_inspectionReport->addCalibrationMarker(m_image, clickPosition, designator);
                                    m_updateMarkers=true;
                                    ImageViewer::callback(MSG_CHANGES_MADE);
                                    Widget::needGeneralDraw();
                                }
                            }
                            else if(m_workspace==WORKSPACE_INSPECTION)
                            {
                                if(m_toolState==TOOL_COMMENT)//Clicking on the PCB (not a component) with the comment tool
                                {
                                    if(m_itemOver=="")
                                        ImageViewer::createFloatingComment(mouseAndKeyboard.mousePos);
                                }
                                else if(m_toolState==TOOL_MEASUREMENT)
                                {
                                    ImageViewer::placeMeasurement(mouseAndKeyboard.mousePos);
                                }

                                if(m_itemSelected != m_itemOver)
                                {
                                    ImageViewer::select(m_itemOver);
                                    needGeneralDraw();

                                    if(m_toolState==TOOL_COMMENT)
                                    {
                                        if(m_itemSelected!="")
                                        {
                                            ImageViewer::associateComment(m_itemSelected);
                                            ImageViewer::disableCommenting();
                                        }
                                    }
                                }
                                else
                                {
                                    if(event.button.clicks>=2)//Double clicked on a selected component
                                    {
                                        if(m_itemSelected!="")
                                            ImageViewer::associateComment(m_itemSelected);
                                    }
                                }
                            }
                        }
                        break;
                }
                break;

            case SDL_MOUSEWHEEL:
                if(ctrlPressed)//zoom
                {
                    m_image->zoom(this,mouseAndKeyboard.mousePos,event.wheel.y*30);
                }
                else//pan
                {
                    if(shiftPressed)
                        m_image->pan(this,event.wheel.y*30,-event.wheel.x*30);
                    else
                        m_image->pan(this,-event.wheel.x*30,event.wheel.y*30);
                }
                m_updateMarkers = true;
                Widget::needGeneralDraw();
                break;
        }

        m_cursorWasIn = true;
    }
    else //Cursor out
    {
        m_cursorWasIn = false;
        m_itemOver = "";
    }

    Widget::manageEvent(event, mouseAndKeyboard);
}

void ImageViewer::setCurrentImage(Image* image, SDL_Rect* windowRect/*=NULL*/)
{
    m_image = image;
    m_image->setView(this, m_rectAbs, windowRect);
    m_updateMarkers=true;
}

SDL_Rect ImageViewer::getImageAbsRect()
{
    SDL_Rect rectSrc, rectDst;
    m_image->getRects(this, &rectSrc, &rectDst);

    return rectDst;
}

void ImageViewer::setVisibleItems(uint16_t visibleItems)
{
    m_visibleItems = visibleItems;
}

void ImageViewer::setFontTooltip(TTF_Font* font)
{
    m_tooltipSelected.setText(m_tooltipSelected.getText(),font);
    m_tooltipOver.setText(m_tooltipOver.getText(),font);
    m_tooltipMeasurement.setText(m_tooltipMeasurement.getText(),font);
}

void ImageViewer::setPopUps(PopUpText* popUpCalibration, PopUpText* popUpComment, PopUpYesNo* popUpYesNo)
{
    m_popUpCalibration = popUpCalibration;
    m_popUpComment = popUpComment;
    m_popUpYesNo = popUpYesNo;
}



void ImageViewer::setWorkspace(Workspace workspace)
{
    m_workspace = workspace;

    if(m_workspace!=WORKSPACE_INSPECTION)
        ImageViewer::disableCommenting();

    ImageViewer::resetCursor();
}


bool ImageViewer::selectAndShow(string designator)
{
    ImageCalibration calibration = m_inspectionReport->getCalibration(m_image);
    for(size_t i=0;i<calibration.items.size();i++)
    {
        if(calibration.items[i]->designator == designator)
        {
            ImageViewer::select(designator);
            ImageViewer::centerOn(calibration.items[i]->pos);
            return true;
        }
    }

    //It wasn't found, maybe it is an internal item (comment or measurement) that is prefixed with VISE_
    for(size_t i=0;i<calibration.items.size();i++)
    {
        if(calibration.items[i]->designator == "VISE_"+designator)
        {
            ImageViewer::select("VISE_"+designator);
            ImageViewer::centerOn(calibration.items[i]->pos);
            return true;
        }
    }

    return false;
}

bool ImageViewer::selectAndShowUndefined()
{
    const string originalImagePath=m_image->getPath();
    string imagePath = m_image->getPath();

    string designator="";
    do
    {
        designator = m_inspectionReport->getFirstUndefined(imagePath);

        if(designator=="")//No more undefined in this image
        {
            Image* newImage = m_fileViewer->getNextImage(imagePath);
            m_fileViewer->selectFile(newImage);
            ImageViewer::setCurrentImage(newImage);

            imagePath = m_image->getPath();
        }
        else
        {
            ImageViewer::selectAndShow(designator);
            return false;
        }
    } while(designator=="" && imagePath!=originalImagePath);

    return true; //Done!
}

void ImageViewer::setSelectedComponentState(CompState state)
{
    bool inView=false; //Only set component state if they are visible
    for(size_t i=0;i<m_itemsVisible.size();i++)
    {
        if(m_itemSelected == m_itemsVisible[i].first)
        {
            inView=true;
            break;
        }
    }

    if(!inView)
        return;

    CompState currentState;
    if(m_inspectionReport->getComponentState(m_itemSelected,&currentState) && currentState!=state)
    {
        bool polarized;
        if(state!=COMP_UNDEFINED && m_inspectionReport->getComponentPolarized(m_itemSelected,&polarized) && polarized)
        {
            m_popUpYesNo->setTitle("Polarization checked?");
            m_popUpYesNo->setText("Have you verified this\ncomponent's polarization?");
            m_popUpYesNo->setButtonOkText("Yes!");
            m_popUpYesNo->setButtonNoText("No...");
            if(m_popUpYesNo->prompt()!=POPUP_OK)
                return;
        }

        callback(MSG_CHANGES_MADE);
        m_inspectionReport->setComponentState(m_itemSelected,state);
        Widget::needRedraw();
    }
}

bool ImageViewer::enableCommenting()
{
    disableMeasurement();

    if(m_itemSelected=="")
    {
        if(m_toolState==TOOL_COMMENT)
            m_toolState=TOOL_NONE;
        else
            m_toolState=TOOL_COMMENT;

        if(m_cursorIsIn)
        {
            if(m_toolState==TOOL_COMMENT)
                ImageViewer::setCursor(CURSOR_COMMENT);
            else
                ImageViewer::resetCursor();
        }
        return false;
    }
    else
    {
        ImageViewer::associateComment(m_itemSelected);
        return true;
    }
}

void ImageViewer::disableCommenting()
{
    m_toolState=TOOL_NONE;
    ImageViewer::resetCursor();
    ImageViewer::callback(MSG_END_COMMENT);
}

void ImageViewer::associateComment(string designator)
{
    string previousComment = m_inspectionReport->getItemComment(designator);

    string messageDesignator = designator;
    if(designator.substr(0,5) == "VISE_")
        messageDesignator = designator.substr(5);

    m_popUpComment->setText("Type the comment associated to \""+messageDesignator+"\"");
    string comment;
    if(m_popUpComment->prompt(&comment,previousComment)==POPUP_OK)
    {
        bool erasedItem = m_inspectionReport->setItemComment(designator, comment);

        if(erasedItem)
            m_itemSelected = "";

        ImageViewer::callback(MSG_CHANGES_MADE);
    }
}

void ImageViewer::createFloatingComment(SDL_Rect coordOnWindow)
{
    SDL_Rect coordOnImage = m_image->coordWindowToImage(this,coordOnWindow);
    SDL_Rect imageSize = m_image->getSize();

    if(Widget::cursorIn(coordOnImage,&imageSize))
    {
        m_popUpComment->setText("Type the comment associated with the PCB");
        string comment;
        if(m_popUpComment->prompt(&comment,"")==POPUP_OK && comment!="")
        {
            m_inspectionReport->addComment(comment, coordOnImage, m_image->getPath());
            ImageViewer::disableCommenting();
            ImageViewer::callback(MSG_CHANGES_MADE);
        }
    }
}

void ImageViewer::placeMeasurement(SDL_Rect coordOnWindow)
{
    SDL_Rect coordOnImage = m_image->coordWindowToImage(this,coordOnWindow);
    SDL_Rect imageSize = m_image->getSize();

    if(Widget::cursorIn(coordOnImage,&imageSize))
    {
        if(m_measurementFirstSet)
        {
            if(coordOnImage.x == m_currentMeasurement.pos.x && coordOnImage.y == m_currentMeasurement.pos.y)//Clicked on the same pixel of the image
                return;

            m_measurementFirstSet = false;

            string imagePath = m_image->getPath();
            if(m_currentMeasurement.imagePath == imagePath)
            {
                m_inspectionReport->addMeasurement(m_currentMeasurement.pos, coordOnImage, imagePath);
                ImageViewer::callback(MSG_CHANGES_MADE);
                Widget::needRedraw();
                return;
            }
        }

        if(!m_measurementFirstSet)//There was no measurement or the user changed image
        {
            m_currentMeasurement.imagePath = m_image->getPath();
            m_currentMeasurement.pos = coordOnImage;
            m_currentMeasurement.posPickAndPlace = m_inspectionReport->convertImageToPickAndPlace(coordOnImage, m_currentMeasurement.imagePath);
            m_measurementFirstSet = true;
            Widget::needRedraw();
        }
    }
}

void ImageViewer::toggleMeasurement()
{
    if(m_toolState==TOOL_MEASUREMENT)
        disableMeasurement();
    else if(m_workspace==WORKSPACE_INSPECTION)
        enableMeasurement();
}

void ImageViewer::enableMeasurement()
{
    if(m_toolState==TOOL_COMMENT)
        disableCommenting();

    m_itemOver = "";
    m_itemSelected = "";
    m_toolState=TOOL_MEASUREMENT;
    ImageViewer::setCursor(CURSOR_MEASURE);
}

void ImageViewer::disableMeasurement()
{
    m_toolState=TOOL_NONE;
    callback(MSG_END_MEASUREMENT);
    ImageViewer::resetCursor();
}

string ImageViewer::getSelectedComponent()
{
    return m_itemSelected;
}



void ImageViewer::draw(SDL_Surface* surface, uint8_t renderLevel)
{
    if(renderLevel == m_renderLevel)
    {
        //Draw the background
        if(m_texture!=NULL)
            SDL_DestroyTexture(m_texture);

        //Set the texture to be the background for drawing
        m_texture = SDL_CreateTexture(m_renderer,SDL_PIXELFORMAT_BGRA32,SDL_TEXTUREACCESS_TARGET,m_rectAbs.w,m_rectAbs.h);
        SDL_SetRenderTarget(m_renderer, m_texture);
        SDL_SetRenderDrawColor(m_renderer,Utilities::R(m_colorBG), Utilities::G(m_colorBG), Utilities::B(m_colorBG), SDL_ALPHA_OPAQUE);
        SDL_RenderClear(m_renderer);

        if(m_image!=NULL)
        {
            //Draw the image
            SDL_Rect rectSrc, rectDst;
            m_image->getRects(this, &rectSrc, &rectDst);
            SDL_Texture* texture = m_image->getTexture(this);

            SDL_RenderCopy(m_renderer, texture, &rectSrc, &rectDst);


            m_tooltipSelected.hide();
            m_tooltipOver.hide();
            m_tooltipMeasurement.hide();

            //Draw the calibration markers
            ImageCalibration calibration = m_inspectionReport->getCalibration(m_image);
            if(m_workspace==WORKSPACE_CALIBRATION)
            {
                for(size_t i=0;i<calibration.markers.size();i++)
                {
                    SDL_Rect markerRect = m_image->coordImageToViewer(this,calibration.markers[i].pos);
                    SDL_QueryTexture(m_textureCalibMarker, NULL, NULL, &(markerRect.w), &(markerRect.h));
                    markerRect.x -= markerRect.w/2;
                    markerRect.y -= markerRect.h/2;


                    SDL_Rect boundaryRect=m_rectAbs;
                    boundaryRect.x=0;
                    boundaryRect.y=0;
                    if(Widget::rectTouchingIn(markerRect,&boundaryRect))
                        SDL_RenderCopy(m_renderer, m_textureCalibMarker, NULL, &markerRect);
                }
            }
            else if(m_workspace==WORKSPACE_INSPECTION)
            {
                if(m_toolState==TOOL_MEASUREMENT && cursorIn(m_lastCursorPos)) //Draw the measurement tooltip
                {
                    SDL_Rect coordImage = m_image->coordWindowToImage(this,m_lastCursorPos);
                    Dbl_Rect coordPickAndPlace = m_inspectionReport->convertImageToPickAndPlace(coordImage,m_image->getPath());

                    if(m_measurementFirstSet)
                    {
                        m_currentMeasurement.pos2 = coordImage;
                        m_currentMeasurement.posPickAndPlace2 = coordPickAndPlace;
                        m_currentMeasurement.length = Utilities::computeLength(m_currentMeasurement.posPickAndPlace,m_currentMeasurement.posPickAndPlace2);
                        m_currentMeasurement.angle = Utilities::computeAngle(m_currentMeasurement.posPickAndPlace,m_currentMeasurement.posPickAndPlace2);

                        ImageViewer::drawMeasurementLine(m_currentMeasurement.pos,m_currentMeasurement.pos2,Utilities::colorRGB(255,87,255));
                        m_tooltipMeasurement.setText(ImageViewer::getMeasurementString(m_currentMeasurement,false));
                    }
                    else
                    {
                        m_currentMeasurement.pos = coordImage;
                        m_currentMeasurement.posPickAndPlace = coordPickAndPlace;
                        m_tooltipMeasurement.setText(ImageViewer::getMeasurementString(m_currentMeasurement,true,false));
                    }

                    SDL_Rect tooltipPos = m_lastCursorPos;
                    tooltipPos.x -= 20;
                    if(tooltipPos.x < 0)
                        tooltipPos.x = 0;
                    m_tooltipMeasurement.setPosition(tooltipPos,CENTER_RIGHT);
                    m_tooltipMeasurement.show();
                }

                if(m_measurementFirstSet && (m_visibleItems&VIS_MEASURE)!=0) //Draw the temporary measurement marker
                {
                    SDL_Rect markerRect = m_image->coordImageToViewer(this,m_currentMeasurement.pos);
                    SDL_QueryTexture(m_textureMeasureMarker, NULL, NULL, &(markerRect.w), &(markerRect.h));
                    markerRect.x -= markerRect.w/2;
                    markerRect.y -= markerRect.h/2;

                    SDL_Rect boundaryRect=m_rectAbs;
                    boundaryRect.x=0;
                    boundaryRect.y=0;
                    if(Widget::rectTouchingIn(markerRect,&boundaryRect))
                        SDL_RenderCopy(m_renderer, m_textureMeasureMarker, NULL, &markerRect);
                }
            }
            //Draw the markers and items
            if(InspectionReport::isCalibrated(calibration) && (m_workspace==WORKSPACE_CALIBRATION || m_workspace==WORKSPACE_INSPECTION))
            {
                m_itemsVisible.clear();

                for(size_t i=0;i<calibration.items.size();i++)
                {
                    ImageViewer::drawItem(calibration.items[i]);
                }
            }

            //Draw the zoom cursor
            if(m_zooming)
            {
                SDL_Rect zoomCursorRect;
                SDL_QueryTexture(m_textureZoomCursor,NULL,NULL,&zoomCursorRect.w,&zoomCursorRect.h);
                zoomCursorRect.x = m_panZoom_originalCursorPos.x-m_rectAbs.x-zoomCursorRect.w/2;
                zoomCursorRect.y = m_panZoom_originalCursorPos.y-m_rectAbs.y-zoomCursorRect.h/2;
                SDL_RenderCopy(m_renderer, m_textureZoomCursor, NULL, &zoomCursorRect);
            }
        }

        SDL_SetRenderTarget(m_renderer,NULL);
        SDL_RenderCopy(m_renderer, m_texture, NULL, &m_rectAbs);
    }

    for(size_t i=0;i<m_children.size();i++)
    {
        if(m_children[i]->visible())
            m_children[i]->draw(surface,renderLevel);
    }
}

void ImageViewer::deleteItem(string designator)
{
    ItemType type = m_inspectionReport->getItemType(designator);
    if(type==ITEM_UNDEFINED)
        return;

    string designatorWithoutPrefix = designator;
    if(designator.substr(0,5)=="VISE_")
        designatorWithoutPrefix = designatorWithoutPrefix.substr(5);

    m_popUpYesNo->setTitle("Delete?");
    if(type==ITEM_COMPONENT)
    {
        m_popUpYesNo->setText(string("Are you sure you want to\nremove ")+designatorWithoutPrefix+"'s comment");
        m_popUpYesNo->setButtonOkText("Remove");
        m_popUpYesNo->setButtonNoText("Keep");
    }
    else
    {
        m_popUpYesNo->setText(string("Are you sure you want to\ndelete ")+designatorWithoutPrefix);
        m_popUpYesNo->setButtonOkText("Delete");
        m_popUpYesNo->setButtonNoText("Keep");
    }

    if(m_popUpYesNo->prompt()==POPUP_OK)
        m_inspectionReport->deleteItem(designator);
}

void ImageViewer::drawItem(Item* item)
{
    SDL_Texture** texturePtr=NULL;

    //Select texture and look for visibility
    if(m_workspace==WORKSPACE_CALIBRATION && item->type==ITEM_COMPONENT)
    {
        texturePtr = &m_textureCalibComponentMarker;
    }
    else if(m_workspace==WORKSPACE_INSPECTION)
    {
        bool itemAsAShownComment=false;
        if(item->comment!="" && (m_visibleItems&VIS_COMMENT)!=0)
            itemAsAShownComment=true;

        if(item->type==ITEM_COMPONENT)
        {
            Component* component = (Component*)item;

            switch(component->state)
            {
                case COMP_UNDEFINED:
                    if((m_visibleItems&VIS_UNDEFINED)!=0 || itemAsAShownComment)
                        texturePtr = &m_textureUndefinedComponentMarker;
                    break;

                case COMP_FITTED:
                    if((m_visibleItems&VIS_FITTED)!=0 || itemAsAShownComment)
                        texturePtr = &m_textureFittedComponentMarker;
                    break;

                case COMP_NOT_FITTED:
                    if((m_visibleItems&VIS_NOT_FITTED)!=0 || itemAsAShownComment)
                        texturePtr = &m_textureNotFittedComponentMarker;
                    break;

                case COMP_ERROR:
                    if((m_visibleItems&VIS_ERROR)!=0 || itemAsAShownComment)
                        texturePtr = &m_textureErrorComponentMarker;
                    break;
            }
        }
        else if(item->type==ITEM_COMMENT)
        {
            if((m_visibleItems&VIS_COMMENT)!=0)
                texturePtr = &m_textureCommentMarker;
        }
        else if(item->type==ITEM_MEASUREMENT)
        {
            if((m_visibleItems&VIS_MEASURE)!=0 || itemAsAShownComment)
                texturePtr = &m_textureMeasureMarker;
        }
    }
    if(texturePtr==NULL)
    {
        if(m_itemSelected == item->designator)
            m_itemSelected="";
        return;
    }


    //Apply marker color mod if over or selected
    Uint8 r=0,g=0,b=0;
    bool resetColor=false;
    if(m_itemSelected == item->designator)
    {
        resetColor=true;
        SDL_GetTextureColorMod(*texturePtr,&r,&g,&b);

        SDL_SetTextureColorMod(*texturePtr,255,255,255);
    }
    else if(m_itemOver == item->designator)
    {
        resetColor=true;
        SDL_GetTextureColorMod(*texturePtr,&r,&g,&b);

        int meanR=(255+int(r))/2, meanG=(255+int(g))/2, meanB=(255+int(b))/2;
        SDL_SetTextureColorMod(*texturePtr,meanR,meanG,meanB);
    }


    SDL_Rect markerRect;
    if((m_workspace==WORKSPACE_CALIBRATION && item->type==ITEM_COMPONENT) || m_workspace==WORKSPACE_INSPECTION)
    {
        bool markerWasDrawn = ImageViewer::drawMarker(item->pos,texturePtr,NULL,&markerRect);

        //adjust the markerRect for absolute coordinates
        markerRect.x += m_rectAbs.x;
        markerRect.y += m_rectAbs.y;


        //Draw measurement's second marker, line and info
        if(item->type==ITEM_MEASUREMENT)
        {
            Measurement* measurement = (Measurement*)item;
            SDL_Rect secondMarkerRect;
            bool secondMarkerWasDrawn = ImageViewer::drawMarker(measurement->pos2,&m_textureMeasureMarker,NULL,&secondMarkerRect);

            //adjust the markerRect for absolute coordinates
            secondMarkerRect.x += m_rectAbs.x;
            secondMarkerRect.y += m_rectAbs.y;

            //Add to the visible items' list so that both markers of hte measurement can be used by m_itemOver

            if(secondMarkerWasDrawn)
            {
                pair<string,SDL_Rect> itemVisiblePair;
                itemVisiblePair.first = item->designator;
                itemVisiblePair.second = secondMarkerRect;
                m_itemsVisible.push_back(itemVisiblePair);
            }

            Uint8 currentR, currentG, currentB;
            SDL_GetTextureColorMod(*texturePtr,&currentR,&currentG,&currentB);
            ImageViewer::drawMeasurementLine(measurement->pos,measurement->pos2, Utilities::colorRGB(currentR,currentG,currentB));
            if(markerWasDrawn && m_itemSelected == item->designator)//Draw tooltip
            {
                m_tooltipMeasurement.setText(ImageViewer::getMeasurementString(*measurement,true));

                SDL_Rect tooltipPos = m_image->coordImageToWindow(this,measurement->pos);
                tooltipPos.x -= 30;
                if(tooltipPos.x < 0)
                    tooltipPos.x = 0;
                m_tooltipMeasurement.setPosition(tooltipPos,CENTER_RIGHT);
                SDL_Rect tooltipPosAbs = m_tooltipMeasurement.getAbsRect();
                if(tooltipPosAbs.x < m_rectAbs.x)
                    tooltipPos.x += (m_rectAbs.x - tooltipPosAbs.x);
                if(tooltipPosAbs.y < m_rectAbs.y)
                    tooltipPos.y += (m_rectAbs.y - tooltipPosAbs.y);
                if(tooltipPosAbs.y + tooltipPosAbs.h > m_rectAbs.y + m_rectAbs.h)
                    tooltipPos.y -= ((tooltipPosAbs.y+tooltipPosAbs.h) - (m_rectAbs.y+m_rectAbs.h));
                m_tooltipMeasurement.setPosition(tooltipPos,CENTER_RIGHT);

                m_tooltipMeasurement.show();
            }
    }

        if(resetColor)
            SDL_SetTextureColorMod(*texturePtr,r,g,b);

        if(markerWasDrawn)
        {
            //Add to the visible items' list
            pair<string,SDL_Rect> itemVisiblePair;
            itemVisiblePair.first = item->designator;
            itemVisiblePair.second = markerRect;
            m_itemsVisible.push_back(itemVisiblePair);
        }
        else
        {
            if(resetColor)
                SDL_SetTextureColorMod(*texturePtr,r,g,b);
            return;
        }
    }

    if(m_workspace!=WORKSPACE_INSPECTION)
        return;

    //Draw the comment marker (icon that indicates there is a comment on a component)
    if(item->type!=ITEM_COMMENT && item->comment != "")
    {
        SDL_Rect offset={-5, -5};
        ImageViewer::drawMarker(item->pos,&m_textureCommentMarker,&offset);
    }


    //Position and show selected tooltip if applicable
    if(m_itemSelected == item->designator)
    {
        SDL_Rect toolTipRect;
        const int gap = 10;
        toolTipRect.x = markerRect.x + markerRect.w + gap;
        toolTipRect.y = markerRect.y + markerRect.h/2;
        m_tooltipSelected.setPosition(toolTipRect,CENTER_LEFT);

        TouchSide touchSide = rectTouchingOut(m_tooltipSelected.getAbsRect());
        if((touchSide&TOUCH_RIGHT) != 0)
        {
            toolTipRect.x = markerRect.x - gap;

            if((touchSide&TOUCH_TOP) != 0)
            {
                toolTipRect.y = markerRect.y + markerRect.h;
                m_tooltipSelected.setPosition(toolTipRect,TOP_RIGHT);
            }
            else if((touchSide&TOUCH_BOTTOM) != 0)
            {
                toolTipRect.y = markerRect.y;
                m_tooltipSelected.setPosition(toolTipRect,BOTTOM_RIGHT);
            }
            else
            {
                toolTipRect.y = markerRect.y + markerRect.h/2;
                m_tooltipSelected.setPosition(toolTipRect,CENTER_RIGHT);
            }
        }
        else
        {
            if((touchSide&TOUCH_TOP) != 0)
            {
                toolTipRect.y = markerRect.y + markerRect.h;
                m_tooltipSelected.setPosition(toolTipRect,TOP_LEFT);
            }
            else if((touchSide&TOUCH_BOTTOM) != 0)
            {
                toolTipRect.y = markerRect.y;
                m_tooltipSelected.setPosition(toolTipRect,BOTTOM_LEFT);
            }
        }

        m_tooltipSelected.show();
    }
    else if(m_itemOver == item->designator && m_itemOver!=m_itemSelected) //Position and show over tooltip if applicable
    {
        SDL_Rect toolTipRect;
        toolTipRect.x = m_lastCursorPos.x + 10;
        toolTipRect.y = m_lastCursorPos.y;
        m_tooltipOver.setPosition(toolTipRect,CENTER_LEFT);
        m_tooltipOver.show();
    }
}

bool ImageViewer::drawMarker(SDL_Rect position, SDL_Texture** texturePtr, SDL_Rect* offset/*=NULL*/, SDL_Rect* markerRectReturned/*=NULL*/)
{
    if(texturePtr==NULL)
        return false;

    //Compute the position
    SDL_Rect markerRect = m_image->coordImageToViewer(this,position);
    SDL_QueryTexture((*texturePtr), NULL, NULL, &markerRect.w, &markerRect.h);

    if((*texturePtr) == m_textureCommentMarker) //We are drawing a comment
    {
        markerRect.x -= 24;
        markerRect.y -= 25;
    }
    else
    {
        markerRect.x -= markerRect.w/2;
        markerRect.y -= markerRect.h/2;
    }
    if(offset!=NULL)
    {
        markerRect.x += offset->x;
        markerRect.y += offset->y;
    }

    SDL_Rect boundaryRect=m_rectAbs;
    boundaryRect.x=0;
    boundaryRect.y=0;
    if(!Widget::rectTouchingIn(markerRect,&boundaryRect))
        return false;

    //Return the markerRect
    if(markerRectReturned!=NULL)
        (*markerRectReturned) = markerRect;

    //Draw on screen
    SDL_RenderCopy(m_renderer, *texturePtr, NULL, &markerRect);

    return true;
}

void ImageViewer::drawMeasurementLine(SDL_Rect pos1, SDL_Rect pos2, Uint32 color)
{
    SDL_Rect posViewer1 = m_image->coordImageToViewer(this,pos1);
    SDL_Rect posViewer2 = m_image->coordImageToViewer(this,pos2);

    //Draw the line
    SDL_SetRenderDrawColor(m_renderer,Utilities::R(color),Utilities::G(color),Utilities::B(color),SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(m_renderer, posViewer1.x, posViewer1.y, posViewer2.x, posViewer2.y);
    SDL_RenderDrawLine(m_renderer, posViewer1.x+1, posViewer1.y, posViewer2.x+1, posViewer2.y);
    SDL_RenderDrawLine(m_renderer, posViewer1.x-1, posViewer1.y, posViewer2.x-1, posViewer2.y);
    SDL_RenderDrawLine(m_renderer, posViewer1.x, posViewer1.y+1, posViewer2.x, posViewer2.y+1);
    SDL_RenderDrawLine(m_renderer, posViewer1.x, posViewer1.y-1, posViewer2.x, posViewer2.y-1);
}

string ImageViewer::getMeasurementString(Measurement measurement, bool useCoord1, bool complete/*=true*/)
{
    SDL_Rect pos;
    Dbl_Rect posPickAndPlace;
    if(useCoord1)
    {
        pos = measurement.pos;
        posPickAndPlace = measurement.posPickAndPlace;
    }
    else
    {
        pos = measurement.pos2;
        posPickAndPlace = measurement.posPickAndPlace2;
    }

    stringstream cursorPosString;
    cursorPosString <<"(" <<pos.x <<", " <<pos.y <<") px\n(" \
                    <<std::setprecision(4) <<posPickAndPlace.x <<", " <<posPickAndPlace.y <<") " <<m_inspectionReport->getUnits();

    if(complete)
        cursorPosString <<"\n" <<measurement.length <<" " <<m_inspectionReport->getUnits() <<"\n" <<measurement.angle <<"°";

    return cursorPosString.str();
}

void ImageViewer::recomputeAbsRect(SDL_Rect* windowRect/*=NULL*/)
{
    Widget::recomputeAbsRect(windowRect);
    if(m_image!=NULL)
        m_image->setView(this, m_rectAbs, windowRect);
}


void ImageViewer::callback(CallbackMessages message)
{
    if(m_callbackFunction!=NULL)
        (*m_callbackFunction)(message,NULL);
}

void ImageViewer::setCursor(CursorType cursorType)
{
    m_previousCursor = m_currentCursor;

    switch(cursorType)
    {
        default:
        case CURSOR_ARROW:
            m_currentCursor = &m_cursorArrow;
            break;

        case CURSOR_CALIBRATION:
            m_currentCursor = &m_cursorCalibration;
            break;

        case CURSOR_INSPECTION:
            m_currentCursor = &m_cursorInspection;
            break;

        case CURSOR_PAN:
            m_currentCursor = &m_cursorPan;
            break;

        case CURSOR_ZOOM:
            m_currentCursor = &m_cursorZoom;
            break;

        case CURSOR_COMMENT:
            m_currentCursor = &m_cursorComment;
            break;

        case CURSOR_MEASURE:
            m_currentCursor = &m_cursorMeasure;
            break;
    }

    SDL_ShowCursor(true);
    SDL_SetCursor(*m_currentCursor);
}

void ImageViewer::resetCursor()
{
    if(m_workspace==WORKSPACE_CALIBRATION && m_cursorIsIn)
        ImageViewer::setCursor(CURSOR_CALIBRATION);
    else if(m_workspace==WORKSPACE_INSPECTION)
    {
        switch(m_toolState)
        {
            case TOOL_COMMENT:
                ImageViewer::setCursor(CURSOR_COMMENT);
                break;

            case TOOL_MEASUREMENT:
                ImageViewer::setCursor(CURSOR_MEASURE);
                break;

            case TOOL_NONE:
            default:
                ImageViewer::setCursor(CURSOR_INSPECTION);
                break;
        }
    }
    else
        ImageViewer::setCursor(CURSOR_ARROW);
}

SDL_Surface* ImageViewer::createCrossMarkerSurface(Uint8 r, Uint8 g, Uint8 b)
{
    const int w=31, h=31, t=3;

    SDL_Surface* returnedSurface = SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_BGRA32);
    SDL_FillRect(returnedSurface,NULL,Utilities::colorRGBA(0,0,0,SDL_ALPHA_TRANSPARENT));

    Uint32 pixel = Utilities::colorRGBA(r,g,b,SDL_ALPHA_OPAQUE);

    for(int y=(h-t)/2;y<(h+t)/2;y++)
    {
        for(int x=0;x<w-1;x++)
        {
            Utilities::setPixel(returnedSurface,x,y,pixel);
        }
    }
    for(int y=0;y<h-1;y++)
    {
        for(int x=(w-t)/2;x<(w+t)/2;x++)
        {
            Utilities::setPixel(returnedSurface,x,y,pixel);
        }
    }


    return returnedSurface;
}

void ImageViewer::select(string designator)
{
    if(designator=="")
    {
        ImageViewer::deselect();
    }
    else if(m_toolState != TOOL_MEASUREMENT)
    {
        m_itemSelected = designator;
        if(m_itemSelected.substr(0,5) == "VISE_")
            m_tooltipSelected.setText(m_itemSelected.substr(5));
        else
            m_tooltipSelected.setText(m_itemSelected);
    }
}

void ImageViewer::deselect()
{
    m_itemSelected = "";
    m_tooltipSelected.setText(m_itemSelected);
}

void ImageViewer::centerOn(SDL_Rect posImage)
{
    SDL_Rect posRelativeToViewer = m_image->coordImageToViewer(this, posImage);
    SDL_Rect middleViewer = {m_rectAbs.w/2, m_rectAbs.h/2};
    SDL_Rect panRect = {middleViewer.x - posRelativeToViewer.x, middleViewer.y - posRelativeToViewer.y};

    m_image->pan(this, panRect.x, panRect.y);

    ImageViewer::needGeneralDraw();
}

ImageViewer::~ImageViewer()
{
    SDL_FreeCursor(m_cursorMeasure);
    SDL_FreeCursor(m_cursorComment);
    SDL_FreeCursor(m_cursorZoom);
    SDL_FreeCursor(m_cursorPan);
    SDL_FreeCursor(m_cursorInspection);
    SDL_FreeCursor(m_cursorCalibration);
    SDL_FreeCursor(m_cursorArrow);

    SDL_DestroyTexture(m_textureZoomCursor);
    SDL_DestroyTexture(m_textureMeasureMarker);
    SDL_DestroyTexture(m_textureCommentMarker);
    SDL_DestroyTexture(m_textureErrorComponentMarker);
    SDL_DestroyTexture(m_textureNotFittedComponentMarker);
    SDL_DestroyTexture(m_textureFittedComponentMarker);
    SDL_DestroyTexture(m_textureUndefinedComponentMarker);
    SDL_DestroyTexture(m_textureCalibComponentMarker);
    SDL_DestroyTexture(m_textureCalibMarker);
}
