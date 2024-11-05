#ifndef IMAGEVIEWER_HEADER
#define IMAGEVIEWER_HEADER

#include "Widget.h"
#include "file/Image.h"
#include "file/InspectionReport.h"
#include "FileViewer.h"
#include "PopUpText.h"
#include "PopUpYesNo.h"
#include "Tooltip.h"



enum CursorType{CURSOR_ARROW, CURSOR_CALIBRATION, CURSOR_INSPECTION, CURSOR_PAN, CURSOR_ZOOM, CURSOR_COMMENT, CURSOR_MEASURE};
enum Workspace{WORKSPACE_DEFAULT,WORKSPACE_CALIBRATION,WORKSPACE_INSPECTION};
enum ToolState{TOOL_NONE,TOOL_COMMENT,TOOL_MEASUREMENT};
enum CallbackMessages{MSG_CHANGES_MADE, MSG_END_COMMENT, MSG_END_MEASUREMENT};
enum {VIS_MEASURE=1,VIS_COMMENT=2,VIS_FITTED=4,VIS_NOT_FITTED=8,VIS_ERROR=16,VIS_UNDEFINED=32};


class ImageViewer : public Widget
{
    public:
        ImageViewer(SDL_Window* window, SDL_Renderer* renderer, Widget* parent, uint8_t renderLevel, int x, int y, int w, int h, InspectionReport* inspectionReport, FileViewer* fileViewer);
        void manageEvent(SDL_Event event, MouseAndKeyboardState mouseAndKeyboard);

        void setCallback(std::function<int (CallbackMessages,void*)>* callbackFunction);

        void setCurrentImage(Image* image, SDL_Rect* windowRect=NULL);
        SDL_Rect getImageAbsRect();

        void setVisibleItems(uint16_t visibleItems);

        void setFontTooltip(TTF_Font* font);
        void setPopUps(PopUpText* popUpCalibration, PopUpText* popUpComment, PopUpYesNo* popUpYesNo);
        void setWorkspace(Workspace workspace);//All workspaces are tied to the pick and place (where all infos are stored)

        bool selectAndShow(std::string designator);
        bool selectAndShowUndefined();
        void setSelectedComponentState(CompState state);
        bool enableCommenting();
        void disableCommenting();
        void associateComment(std::string designator);
        void createFloatingComment(SDL_Rect coordOnWindow);
        void placeMeasurement(SDL_Rect coordOnWindow);

        void toggleMeasurement();
        void enableMeasurement();
        void disableMeasurement();

        std::string getSelectedComponent();

        virtual void draw(SDL_Surface* surface, uint8_t renderLevel);
        virtual void recomputeAbsRect(SDL_Rect* windowRect=NULL);
        virtual ~ImageViewer();
    private:
        void deleteItem(std::string designator);
        void drawItem(Item* item);
        bool drawMarker(SDL_Rect position, SDL_Texture** texturePtr, SDL_Rect* offset=NULL, SDL_Rect* markerRectReturned=NULL);
        void drawMeasurementLine(SDL_Rect pos1, SDL_Rect pos2, Uint32 color);
        std::string getMeasurementString(Measurement measurement, bool useCoord1, bool complete=true);
        void callback(CallbackMessages message);
        void setCursor(CursorType cursorType);
        void resetCursor();
        static SDL_Surface* createCrossMarkerSurface(Uint8 r, Uint8 g, Uint8 b);
        void select(std::string designator);
        void deselect();
        void centerOn(SDL_Rect posImage);

    private:
        std::function<int (CallbackMessages,void*)>* m_callbackFunction;

        SDL_Window* m_window;

        Image* m_image;

        PopUpText* m_popUpCalibration;
        PopUpText* m_popUpComment;
        PopUpYesNo* m_popUpYesNo;

        Tooltip m_tooltipSelected;
        Tooltip m_tooltipOver;

        Tooltip m_tooltipMeasurement;


        SDL_Texture* m_textureCalibMarker;
        SDL_Texture* m_textureCalibComponentMarker;
        SDL_Texture* m_textureUndefinedComponentMarker;
        SDL_Texture* m_textureFittedComponentMarker;
        SDL_Texture* m_textureNotFittedComponentMarker;
        SDL_Texture* m_textureErrorComponentMarker;
        SDL_Texture* m_textureCommentMarker;
        SDL_Texture* m_textureMeasureMarker;

        SDL_Texture* m_textureZoomCursor;

        ToolState m_toolState;
        bool m_panning;
        bool m_zooming;
        bool m_clicking;
        bool m_updateMarkers;
        bool m_cursorIsIn;
        bool m_cursorWasIn;

        Measurement m_currentMeasurement;
        bool m_measurementFirstSet;

        SDL_Rect m_panZoom_originalCursorPos;
        SDL_Rect m_panZoom_originalImageRect;
        SDL_Rect m_lastCursorPos;

        uint16_t m_visibleItems;

        Workspace m_workspace;

        InspectionReport* m_inspectionReport;
        std::string m_itemOver;
        std::string m_itemSelected;
        std::vector<std::pair<std::string,SDL_Rect>> m_itemsVisible;

        FileViewer* m_fileViewer;//Used to skip to next image during inspection

        SDL_Cursor** m_previousCursor;
        SDL_Cursor** m_currentCursor;
        SDL_Cursor* m_cursorArrow;
        SDL_Cursor* m_cursorCalibration;
        SDL_Cursor* m_cursorInspection;
        SDL_Cursor* m_cursorPan;
        SDL_Cursor* m_cursorZoom;
        SDL_Cursor* m_cursorComment;
        SDL_Cursor* m_cursorMeasure;
};

#endif // IMAGEVIEWER_HEADER
