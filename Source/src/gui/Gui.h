#ifndef GUI_HEADER
#define GUI_HEADER

#include "interface/CommDDE.h"

#include <vector>
#include <SDL.h>

#include "misc/Utilities.h"
#include "Widget.h"
#include "WidgetBar.h"
#include "ImageViewer.h"
#include "FileViewer.h"
#include "Button.h"
#include "PopUpText.h"
#include "PopUpYesNo.h"
#include "file/Image.h"
#include "file/PickAndPlace.h"
#include "file/InspectionReport.h"


#define W 800
#define H 800



class Gui
{
    public:
        Gui(SDL_Window* window, SDL_Renderer* renderer);
        int loop(int argc, char* argv[]);

        virtual ~Gui();

    private:
        void setTitle(std::string inspectionFileName="");
        void initVisibleButtons();
        void setWorkspace(Workspace workspace);
        int loadImage(const std::string imagePath);
        int loadPickAndPlace(const std::string pickAndPlacePath);
        int openInspectionFile(const std::string inspectionFilePath="");
        int fileGotSelected(File* file);
        void reportWasSaved();
        bool saveScreenshot(std::string defaultName, SDL_Rect area);

        void getWindowPosition(SDL_Rect* windowRect);
        void getWindowSize(SDL_Rect* windowRect);
        SDL_Rect getWindowSize();
        void getWindowRect(SDL_Rect* windowRect);

        void startInspection();
        void stopInspection();
        int searchForItem(std::string designator="", bool placeCursorOnComponent=true);
        void getNextComponent();
        void setSelectedComponentState(CompState state);


    private:
        CommDDE m_DDEServer;

        std::vector<File*> m_files;
        PickAndPlace* m_pickAndPlace;//Only one file is allowed
        InspectionReport m_inspectionReport;//The architecture that saves all the data regarding files, calibration, statuses, etc.

        std::string m_savePath;
        unsigned long m_lastTimeSave;
        bool m_unsavedChanges;

        unsigned long m_captureCounter;

        uint16_t m_visibleItems;

        Workspace m_workspace;

        SDL_Window* m_window;
        SDL_Renderer* m_renderer;

        SDL_Rect m_windowRect;

        TTF_Font* m_fontButton;
        TTF_Font* m_fontPopUp;
        TTF_Font* m_fontPopUpTextField;
        TTF_Font* m_fontTooltip;
        TTF_Font* m_fontTooltipRefDes;

        SDL_Surface* m_icon;

        Widget m_rootWidget;//Main widget, root of all
        WidgetBar m_barTop;//Toolbar for buttons
        WidgetBar m_barTools;//Toolbar for tools specific to each workspace
        WidgetBar m_barRight;//Toolbar for the file viewer

        FileViewer m_fileViewer;//List of all images and Pick and Place files

        Label m_labelIcon;//Just a little icon to decorate
        Button m_buttonOpenConfig;//Load calibration and file paths from a config file
        Button m_buttonSaveConfig;//Save calibration and file paths in a config file
        Button m_buttonLoadPP;//Load Pick & Place file
        Button m_buttonLoadImage;//Load image for analysis
        Button m_buttonCalibrate;//Perform calibration procedure on an image
        Button m_buttonInspect;//Perform the verification procedure on all calibrated images
        Button m_buttonSnapshot;//Saves a screenshot of the current image view
        Button m_buttonUserManual;//Used to display the user manual on a webpage
        Button m_buttonReportBug;//Used to display the bug report webpage
        Button m_buttonHelp;//Used to show a help popup

        Button m_buttonVisibleMeasure;//Chooses if measurements are visible
        Button m_buttonMeasure;//Allows the user to take measurements
        Button m_buttonVisibleComment;//Chooses if comments are visible
        Button m_buttonComment;//Adds a comment on a component
        Button m_buttonVisibleFitted;//Chooses if fitted component are visible
        Button m_buttonComponentFitted;//Changes the state of the selected component for "Fitted"
        Button m_buttonVisibleNotFitted;//Chooses if not fitted components are visible
        Button m_buttonComponentNotFitted;//Changes the state of the selected component for "Not Fitted"
        Button m_buttonVisibleError;//Chooses if components in error are visible
        Button m_buttonComponentError;//Changes the state of the selected component for "Error"
        Button m_buttonVisibleUndefined;//Chooses if undefined components are visible
        Button m_buttonComponentUndefined;//Changes the state of the selected component for "Undefined"
        Button m_buttonSearch;//Searches for a component
        Button m_buttonNextComponent;//Moves the camera to be centered onto the next component

        ImageViewer m_imageViewer;//View for the images

        PopUpText m_popUpRefDes;//Used to ask the user the refdes of a calibrated component
        PopUpText m_popUpSearch;//Used to ask the user the refdes of the component to search for
        PopUpText m_popUpComment;//Used to ask the user the comment's message
        PopUpYesNo m_popUpYesNo;//Used to verify if the user wants to delete a comment or measurement
        PopUp m_popUpOkButton;//Used to inform the user of various informations with an ok button
        PopUpYesNo m_popUpExit;//Prompts the user to make sure they want to exit
        PopUp m_popUpHelp;//Used to display some basic informations about the software

        Tooltip m_tooltipSaved;//Informs the user that the file was saved
};

#endif // GUI_HEADER
