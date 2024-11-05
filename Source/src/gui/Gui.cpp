#include "gui/Gui.h"

#include <filesystem>
#include <bitset>
#include <SDL_image.h>
#include <nfd.h>


#define VERSION_MAJOR 1
#define VERSION_MINOR 4

#define TIME_AUTOSAVE (1000*60*5)

#define TOPBAR_H 30
#define RIGHTBAR_W 120

#define BUTTON_W 70
#define BUTTON_H TOPBAR_H

#define MIN_W ((BUTTON_H+1 + BUTTON_W+1)*6 + RIGHTBAR_W)
#define MIN_H 300//MIN_W


namespace fs = std::filesystem;



Gui::Gui(SDL_Window* window, SDL_Renderer* renderer) :
    m_DDEServer(),
    m_pickAndPlace(NULL),
    m_inspectionReport(),
    m_savePath(""),
    m_lastTimeSave(0),
    m_unsavedChanges(false),
    m_captureCounter(0),
    m_visibleItems(VIS_MEASURE|VIS_COMMENT|VIS_FITTED|VIS_NOT_FITTED|VIS_ERROR|VIS_UNDEFINED),
    m_workspace(WORKSPACE_DEFAULT),
    m_window(window),
    m_renderer(renderer),
    m_fontButton(NULL),
    m_fontPopUp(NULL),
    m_fontPopUpTextField(NULL),
    m_fontTooltip(NULL),
    m_fontTooltipRefDes(NULL),
    m_icon(NULL),
    m_rootWidget(m_renderer,NULL,0,0,0,W,H),
    m_barTop(m_renderer,&m_rootWidget,0,0,0,0,TOPBAR_H),
    m_barTools(m_renderer,&m_rootWidget,0,0,TOPBAR_H,0,(TOPBAR_H+1)*2),
    m_barRight(m_renderer,&m_rootWidget,0,-RIGHTBAR_W,TOPBAR_H,RIGHTBAR_W,-TOPBAR_H),
    m_fileViewer(m_renderer,&m_barRight,1,0,0,0,0),
    m_labelIcon(m_renderer,&m_barTop,0,0,0,BUTTON_H,BUTTON_H),
    m_buttonOpenConfig(m_renderer,&m_barTop,0,BUTTON_H+1,0,BUTTON_W,BUTTON_H),
    m_buttonSaveConfig(m_renderer,&m_barTop,0,BUTTON_H+1+(BUTTON_W+1),0,BUTTON_W,BUTTON_H),
    m_buttonLoadPP(m_renderer,&m_barTop,0,BUTTON_H+1+(BUTTON_W+1)*2,0,BUTTON_W,BUTTON_H),
    m_buttonLoadImage(m_renderer,&m_barTop,0,BUTTON_H+1+(BUTTON_W+1)*3,0,BUTTON_W,BUTTON_H),
    m_buttonCalibrate(m_renderer,&m_barTop,0,BUTTON_H+1+(BUTTON_W+1)*4,0,BUTTON_W,BUTTON_H),
    m_buttonInspect(m_renderer,&m_barTop,0,BUTTON_H+1+(BUTTON_W+1)*5,0,BUTTON_W,BUTTON_H),
    m_buttonSnapshot(m_renderer,&m_barTop,0,BUTTON_H+1+(BUTTON_W+1)*6,0,BUTTON_W,BUTTON_H),
    m_buttonUserManual(m_renderer,&m_barTop,0,-BUTTON_H*3-2,0,BUTTON_H,BUTTON_H),
    m_buttonReportBug(m_renderer,&m_barTop,0,-BUTTON_H*2-1,0,BUTTON_H,BUTTON_H),
    m_buttonHelp(m_renderer,&m_barTop,0,-BUTTON_H,0,BUTTON_H,BUTTON_H),
    m_buttonVisibleMeasure(m_renderer,&m_barTools,0,0,1,BUTTON_H,BUTTON_H),
    m_buttonMeasure(m_renderer,&m_barTools,0,m_buttonVisibleMeasure.getAbsRect().x+m_buttonVisibleMeasure.getAbsRect().w+1,1,BUTTON_W,BUTTON_H),
    m_buttonVisibleComment(m_renderer,&m_barTools,0,m_buttonMeasure.getAbsRect().x+m_buttonMeasure.getAbsRect().w+1,1,BUTTON_H,BUTTON_H),
    m_buttonComment(m_renderer,&m_barTools,0,m_buttonVisibleComment.getAbsRect().x+m_buttonVisibleComment.getAbsRect().w+1,1,BUTTON_W,BUTTON_H),
    m_buttonVisibleFitted(m_renderer,&m_barTools,0,m_buttonComment.getAbsRect().x+m_buttonComment.getAbsRect().w+1,1,BUTTON_H,BUTTON_H),
    m_buttonComponentFitted(m_renderer,&m_barTools,0,m_buttonVisibleFitted.getAbsRect().x+m_buttonVisibleFitted.getAbsRect().w+1,1,BUTTON_W,BUTTON_H),
    m_buttonVisibleNotFitted(m_renderer,&m_barTools,0,m_buttonComponentFitted.getAbsRect().x+m_buttonComponentFitted.getAbsRect().w+1,1,BUTTON_H,BUTTON_H),
    m_buttonComponentNotFitted(m_renderer,&m_barTools,0,m_buttonVisibleNotFitted.getAbsRect().x+m_buttonVisibleNotFitted.getAbsRect().w+1,1,BUTTON_W,BUTTON_H),
    m_buttonVisibleError(m_renderer,&m_barTools,0,m_buttonComponentNotFitted.getAbsRect().x+m_buttonComponentNotFitted.getAbsRect().w+1,1,BUTTON_H,BUTTON_H),
    m_buttonComponentError(m_renderer,&m_barTools,0,m_buttonVisibleError.getAbsRect().x+m_buttonVisibleError.getAbsRect().w+1,1,BUTTON_W,BUTTON_H),
    m_buttonVisibleUndefined(m_renderer,&m_barTools,0,m_buttonComponentError.getAbsRect().x+m_buttonComponentError.getAbsRect().w+1,1,BUTTON_H,BUTTON_H),
    m_buttonComponentUndefined(m_renderer,&m_barTools,0,m_buttonVisibleUndefined.getAbsRect().x+m_buttonVisibleUndefined.getAbsRect().w+1,1,BUTTON_W,BUTTON_H),
    m_buttonSearch(m_renderer,&m_barTools,0,0,1+BUTTON_H+1,BUTTON_W,BUTTON_H),
    m_buttonNextComponent(m_renderer,&m_barTools,0,(BUTTON_W+1),1+BUTTON_H+1,BUTTON_W,BUTTON_H),
    m_imageViewer(m_window, m_renderer,&m_rootWidget,1,0,TOPBAR_H,-RIGHTBAR_W,-TOPBAR_H,&m_inspectionReport,&m_fileViewer),
    m_popUpRefDes(m_window, {m_imageViewer.getAbsRect().x+20,m_imageViewer.getAbsRect().y+25+20,400,100}, TOP_LEFT, true, "Input reference designator"),
    m_popUpSearch(m_window, {m_imageViewer.getAbsRect().x+20,TOPBAR_H*3+25+20,400,100}, TOP_LEFT, true, "Search"),
    m_popUpComment(m_window, {m_imageViewer.getAbsRect().x+20,TOPBAR_H*3+25+20,400,100}, TOP_LEFT, true, "Comment"),
    m_popUpYesNo(m_window, {m_rootWidget.getAbsRect().w/2,m_rootWidget.getAbsRect().h/2,350,120}, CENTERED, true, ""),
    m_popUpOkButton(m_window, {m_imageViewer.getAbsRect().x+m_imageViewer.getAbsRect().w/2,m_imageViewer.getAbsRect().y+m_imageViewer.getAbsRect().h/2,400,120}, CENTERED, true, ""),
    m_popUpExit(m_window,{m_rootWidget.getAbsRect().w/2,m_rootWidget.getAbsRect().h/2,300,120}, CENTERED, true, "Exit?"),
    m_popUpHelp(m_window,{m_rootWidget.getAbsRect().w/2,m_rootWidget.getAbsRect().h/2,650,510}, CENTERED, false, "Help!"),
    m_tooltipSaved(m_renderer,&m_rootWidget,m_imageViewer.getRenderLevel()+1,10,TOPBAR_H+10,TOP_LEFT)
{
    //Init GUI
    Gui::setTitle("");

    const char* fontName="Font_bold.ttf";

    SDL_SetWindowMinimumSize(m_window,MIN_W,MIN_H);
    m_icon = IMG_Load(Utilities::res_images("icon.png").c_str());
    SDL_SetWindowIcon(m_window,m_icon);

    m_barTools.hide();

    m_fontButton = TTF_OpenFont(Utilities::res_fonts(fontName).c_str(), 12);
    m_fontPopUp = TTF_OpenFont(Utilities::res_fonts(fontName).c_str(), 16);
    m_fontPopUpTextField = TTF_OpenFont(Utilities::res_fonts(fontName).c_str(), 20);
    m_fontTooltip = TTF_OpenFont(Utilities::res_fonts(fontName).c_str(), 12);
    m_fontTooltipRefDes = TTF_OpenFont(Utilities::res_fonts(fontName).c_str(), 15);

    m_imageViewer.setFontTooltip(m_fontTooltipRefDes);

    m_fileViewer.setFont(m_fontButton);
    m_labelIcon.setImage(Utilities::res_images("lightning.png"));
    m_labelIcon.setColorBG(Utilities::colorRGB(50,50,50));
    m_buttonSaveConfig.setText("Save Insp", m_fontButton);
    m_buttonOpenConfig.setText("Open Insp", m_fontButton);
    m_buttonLoadImage.setText("Load Img", m_fontButton);
    m_buttonLoadPP.setText("Load P&P", m_fontButton);
    m_buttonCalibrate.setText("Calibrate", m_fontButton);//Calibrate and Inspect can't be pressable because
    m_buttonInspect.setText("Inspect", m_fontButton);//some conditions are needed in order to change the state
    m_buttonSnapshot.setText("Snap!", m_fontButton);
    m_buttonUserManual.setIcon(Utilities::res_images("userManual.png"));
    m_buttonReportBug.setIcon(Utilities::res_images("bugReport.png"));
    m_buttonHelp.setIcon(Utilities::res_images("help.png"));
    m_buttonMeasure.setText("Meas. [M]", m_fontButton);
    m_buttonMeasure.setTextColor(255,87,255);
    m_buttonMeasure.pressable();
    m_buttonComment.setText("Comm. [C]", m_fontButton);
    m_buttonComment.setTextColor(255,255,0);
    m_buttonComponentFitted.setText("Fitted [F]", m_fontButton);
    m_buttonComponentFitted.setTextColor(0,255,0);
    m_buttonComponentNotFitted.setText("Not Fit. [N]", m_fontButton);
    m_buttonComponentNotFitted.setTextColor(35,104,255);
    m_buttonComponentError.setText("Error [E]", m_fontButton);
    m_buttonComponentError.setTextColor(255,0,0);
    m_buttonComponentUndefined.setText("Undef. [U]", m_fontButton);
    m_buttonComponentUndefined.setTextColor(255,127,0);
    m_buttonSearch.setText("Search [^F]", m_fontButton);
    m_buttonNextComponent.setText("Next [⏎]", m_fontButton);
    Gui::initVisibleButtons();

    m_popUpRefDes.setTexts("What is this component's reference designator?",m_fontPopUp,m_fontPopUpTextField,m_fontTooltip,"Incorrect refdes");
    m_popUpSearch.setTexts("What is the component's reference designator?",m_fontPopUp,m_fontPopUpTextField,m_fontTooltip,"Incorrect refdes");
    m_popUpComment.setTexts("",m_fontPopUp,m_fontPopUpTextField);
    m_popUpYesNo.setText("",m_fontPopUp);
    m_popUpOkButton.setText("",m_fontPopUp);
    m_popUpExit.setText("You have unsaved changes.\nDo you really want to exit?",m_fontPopUp);
    m_popUpExit.setButtonOkText("Exit");
    m_popUpExit.setButtonNoText("Go back");
    m_popUpHelp.setTextAlignment(ALIGN_LEFT);
    m_popUpHelp.setButtonOkSpacingHeight(50);
    m_popUpHelp.setText(string("If you are starting a new projet:\n")+
                               "   1) Load a pick & place using the button \""+m_buttonLoadPP.getText()+"\"\n"+
                               "   2) Load an image using the button \""+m_buttonLoadImage.getText()+"\"\n"+
                               "   3) Load all other project's images using the button repeatedly\n"+
                               "   4) Save your project using the button \""+m_buttonSaveConfig.getText()+"\"\n"+
                               "      From now on, your work will autmatically save every 5 minutes\n"+
                               "   5) Press the \""+m_buttonCalibrate.getText()+"\" button\n"+
                               "   6) Click in the middle of a component listed in the P&P (Fiducial works best)\n"+
                               "   7) Type its reference designator in the pop-up box\n"+
                               "   8) Repeat steps 6 & 7 for two additional components\n"+
                               "   9) Repeat steps 6 to 8 for every additional images\n"+
                               "  10) Save the inspection file. You are ready to start!\n"+
                               "  11) Press the \""+m_buttonInspect.getText()+"\" button\n"+
                               "  12) Press Return or click on any component to select it\n"+
                               "  13) Click on one of the colorful state buttons to attribute a state\n"+
                               "  14) Double click a component to add a comment\n"+
                               "  15) Press the \""+m_buttonComment.getText()+"\" button to add a comment anywhere\n"+
                               "  15) Repeat steps 12 to 15 for every component\n"+
                               "  16) Read the User Manual for additionnal controls and tips to save time!\n"+
                               "\n"+
                               "If you are resuming an ongoing inspection:\n"+
                               "   1) Press the \""+m_buttonOpenConfig.getText()+"\" button\n"+
                               "   2) Open the .csvvise file previously saved\n"+
                               "   3) Press the \""+m_buttonInspect.getText()+"\" button\n"+
                               "   3) Resume the inspection\n"+
                               "\n"+
                               "For any help or comment, send a message to Philippe Lamarche :)"
                               ,m_fontPopUp);
    m_popUpHelp.setButtonOkText("Thanks!");

    m_imageViewer.setPopUps(&m_popUpRefDes, &m_popUpComment, &m_popUpYesNo);//Illegal, breaks encapsulation

    m_tooltipSaved.setText("Saved!", m_fontPopUpTextField);

    //Draw GUI
    m_rootWidget.needGeneralDraw();
}

void Gui::setTitle(string inspectionFileName)
{
    string title = "Visual Inspection Software for Electronics - ";

    if(inspectionFileName.compare("")!=0)
        title += inspectionFileName + " - ";

    title += "Version " + to_string(VERSION_MAJOR) + ".";
    if(VERSION_MINOR<10)
        title+="0";
    title += to_string(VERSION_MINOR);

    SDL_SetWindowTitle(m_window,title.c_str());
}

void Gui::initVisibleButtons()
{
    m_buttonVisibleMeasure.setIcon(Utilities::res_images("eyeClosed.png"));
    m_buttonVisibleMeasure.setIconPressed(Utilities::res_images("eyeOpenned.png"));
    m_buttonVisibleMeasure.pressable();
    m_buttonVisibleMeasure.press();
    m_buttonVisibleMeasure.setIconPressedColorMod(50,50,50);
    m_buttonVisibleMeasure.setColorBGPressedInIdle(255,87,255);
    m_buttonVisibleMeasure.setColorBGPressedInOver(174,59,174);
    m_buttonVisibleMeasure.setColorBGPressed(116,40,116);
    m_buttonVisibleMeasure.setColorBGOver(87,30,87);
    m_buttonVisibleMeasure.setIconColorMod(255,87,255);

    m_buttonVisibleComment.setIcon(Utilities::res_images("eyeClosed.png"));
    m_buttonVisibleComment.setIconPressed(Utilities::res_images("eyeOpenned.png"));
    m_buttonVisibleComment.pressable();
    m_buttonVisibleComment.press();
    m_buttonVisibleComment.setIconPressedColorMod(50,50,50);
    m_buttonVisibleComment.setColorBGPressedInIdle(220,220,0);
    m_buttonVisibleComment.setColorBGPressedInOver(150,150,0);
    m_buttonVisibleComment.setColorBGPressed(100,100,0);
    m_buttonVisibleComment.setColorBGOver(75,75,0);
    m_buttonVisibleComment.setIconColorMod(255,255,0);

    m_buttonVisibleFitted.setIcon(Utilities::res_images("eyeClosed.png"));
    m_buttonVisibleFitted.setIconPressed(Utilities::res_images("eyeOpenned.png"));
    m_buttonVisibleFitted.pressable();
    m_buttonVisibleFitted.press();
    m_buttonVisibleFitted.setIconPressedColorMod(50,50,50);
    m_buttonVisibleFitted.setColorBGPressedInIdle(0,220,0);
    m_buttonVisibleFitted.setColorBGPressedInOver(0,150,0);
    m_buttonVisibleFitted.setColorBGPressed(0,100,0);
    m_buttonVisibleFitted.setColorBGOver(0,75,0);
    m_buttonVisibleFitted.setIconColorMod(0,255,0);

    m_buttonVisibleNotFitted.setIcon(Utilities::res_images("eyeClosed.png"));
    m_buttonVisibleNotFitted.setIconPressed(Utilities::res_images("eyeOpenned.png"));
    m_buttonVisibleNotFitted.pressable();
    m_buttonVisibleNotFitted.press();
    m_buttonVisibleNotFitted.setIconPressedColorMod(50,50,50);
    m_buttonVisibleNotFitted.setColorBGPressedInIdle(35,104,255);
    m_buttonVisibleNotFitted.setColorBGPressedInOver(25,78,191);
    m_buttonVisibleNotFitted.setColorBGPressed(16,47,116);
    m_buttonVisibleNotFitted.setColorBGOver(12,35,87);
    m_buttonVisibleNotFitted.setIconColorMod(35,104,255);

    m_buttonVisibleError.setIcon(Utilities::res_images("eyeClosed.png"));
    m_buttonVisibleError.setIconPressed(Utilities::res_images("eyeOpenned.png"));
    m_buttonVisibleError.pressable();
    m_buttonVisibleError.press();
    m_buttonVisibleError.setIconPressedColorMod(50,50,50);
    m_buttonVisibleError.setColorBGPressedInIdle(220,0,0);
    m_buttonVisibleError.setColorBGPressedInOver(150,0,0);
    m_buttonVisibleError.setColorBGPressed(100,0,0);
    m_buttonVisibleError.setColorBGOver(75,0,0);
    m_buttonVisibleError.setIconColorMod(255,0,0);

    m_buttonVisibleUndefined.setIcon(Utilities::res_images("eyeClosed.png"));
    m_buttonVisibleUndefined.setIconPressed(Utilities::res_images("eyeOpenned.png"));
    m_buttonVisibleUndefined.pressable();
    m_buttonVisibleUndefined.press();
    m_buttonVisibleUndefined.setIconPressedColorMod(50,50,50);
    m_buttonVisibleUndefined.setColorBGPressedInIdle(220,110,0);
    m_buttonVisibleUndefined.setColorBGPressedInOver(150,75,0);
    m_buttonVisibleUndefined.setColorBGPressed(100,50,0);
    m_buttonVisibleUndefined.setColorBGOver(75,38,0);
    m_buttonVisibleUndefined.setIconColorMod(255,127,0);
}

int Gui::loop(int argc, char* argv[])
{
    function<int (string,string)> callbackDDE = [&](string functionToExecute, string argument)
    {
        //cout <<"Callback for \"" <<functionToExecute <<"\" with argument \"" <<argument <<"\"" <<endl;
        if(functionToExecute.compare("Find")==0)
        {
            int found = searchForItem(argument,false);
            m_rootWidget.manageDraw();
            SDL_RaiseWindow(m_window);
            return found;
        }

        return 0;
    };
    m_DDEServer.setCallback(&callbackDDE);


    //Create callback functions
    function<int (CallbackMessages,void*)> callbackImageViewer = [&](CallbackMessages message, void* arguments)
    {
        switch(message)
        {
            case MSG_CHANGES_MADE:
                m_unsavedChanges = true;
                break;

            case MSG_END_COMMENT:
                m_buttonComment.depress();
                break;

            case MSG_END_MEASUREMENT:
                m_buttonMeasure.depress();
                break;

            default:
                break;
        }


        return 0;
    };
    m_imageViewer.setCallback(&callbackImageViewer);

    function<int (void*,void*)> callbackSaveCfg = [&](void* objPtr, void* arguments)
    {
        string savePath = m_inspectionReport.saveInspectionFile();

        if(savePath!="")
            Gui::reportWasSaved();

        if(m_savePath=="" || savePath!="")
            m_savePath = savePath;


        return 0;
    };
    m_buttonSaveConfig.setCallback(&callbackSaveCfg,this,NULL);

    function<int (void*,void*)> callbackOpenCfg = [&](void* objPtr, void* arguments)
    {
        Gui::openInspectionFile();
        return 0;
    };
    m_buttonOpenConfig.setCallback(&callbackOpenCfg,this,NULL);

    function<int (void*,void*)> callbackResetWorkplace = [&](void* objPtr, void* arguments)
    {
        Gui::setTitle(*((string*)arguments));

        for(int i=m_files.size()-1;i>=0;i--)
        {
            if(m_files[i]->getType()==FILE_IMAGE)
            {
                delete m_files[i];
                m_files.erase(m_files.begin()+i);
            }
        }

        return 0;
    };
    m_inspectionReport.setCallbackResetWorkspace(&callbackResetWorkplace,this);

    function<int (void*,void*)> callbackLoadImage = [&](void* objPtr, void* arguments)
    {
        nfdu8char_t* imagePath = (char*)arguments;

        string path;
        if(imagePath==NULL)
        {
            nfdfilteritem_t filters[1] = { {"Images", "bmp,png,jpg"} };
            NFD_OpenDialog(&imagePath, filters, 1, NULL);
            if(imagePath!=NULL)
            {
                path = imagePath;
                delete imagePath;
            }
        }
        else
            path = imagePath;

        return Gui::loadImage(path);
    };
    m_buttonLoadImage.setCallback(&callbackLoadImage,this,NULL);
    m_inspectionReport.setCallbackLoadImage(&callbackLoadImage,this);

    function<int (void*,void*)> callbackLoadPP = [&](void* objPtr, void* arguments)
    {
        string path;
        if(arguments==NULL)
        {
            nfdfilteritem_t filters[1] = { {"CSV Pick and Place", "csv"} };
            char* pickAndPlacePath = NULL;
            NFD_OpenDialog(&pickAndPlacePath, filters, 1, NULL);
            if(pickAndPlacePath!=NULL)
            {
                path = pickAndPlacePath;
                delete pickAndPlacePath;
            }
        }
        else
        {
            const char* pickAndPlacePath = (const char*)arguments;
            path = pickAndPlacePath;
        }
        return Gui::loadPickAndPlace(path);
    };
    m_buttonLoadPP.setCallback(&callbackLoadPP,this,NULL);
    m_inspectionReport.setCallbackLoadPickAndPlace(&callbackLoadPP,this);


    function<int (void*,void*)> callbackCalibrate = [&](void* objPtr, void* arguments)
    {
        if(m_workspace==WORKSPACE_CALIBRATION)
        {
            m_buttonCalibrate.toggleNoCallback();
            Gui::setWorkspace(WORKSPACE_DEFAULT);
        }
        else
        {
            bool thereIsAnImage=false;
            for(size_t i=0;i<m_files.size();i++)
            {
                if(m_files[i]->getType()==FILE_IMAGE)
                {
                    thereIsAnImage=true;
                    break;
                }
            }
            if(m_pickAndPlace!=NULL && thereIsAnImage) //There is a pick and place file and at least oen image loaded
            {
                Gui::stopInspection();
                m_buttonCalibrate.toggleNoCallback();
                Gui::setWorkspace(WORKSPACE_CALIBRATION);
            }
        }
        return 0;
    };
    m_buttonCalibrate.setCallback(&callbackCalibrate,this,NULL);

    function<int (void*,void*)> callbackInspect = [&](void* objPtr, void* arguments)
    {
        if(m_workspace==WORKSPACE_INSPECTION)
        {
            Gui::setWorkspace(WORKSPACE_DEFAULT);
            Gui::stopInspection();
        }
        else
        {
            bool thereIsAnImage=false;
            for(size_t i=0;i<m_files.size();i++)
            {
                if(m_files[i]->getType()==FILE_IMAGE)
                {
                    thereIsAnImage=true;
                    break;
                }
            }
            if(m_pickAndPlace!=NULL && thereIsAnImage) //There is a pick and place file and at least one image loaded
            {
                m_buttonCalibrate.depress();
                Gui::startInspection();
            }
        }
        return 0;
    };
    m_buttonInspect.setCallback(&callbackInspect,this,NULL);

    function<int (void*,void*)> callbackSnapshot = [&](void* objPtr, void* arguments)
    {
        string selectedComponent = m_imageViewer.getSelectedComponent();

        string title;
        if(selectedComponent!="")
        {
            Component component = m_pickAndPlace->getComponent(selectedComponent);
            title = component.layer + "_" + selectedComponent + ".png";
        }
        else
        {
            title = "Capture " + to_string(m_captureCounter) + ".png";
        }

        if(Gui::saveScreenshot(title, m_imageViewer.getAbsRect()) && selectedComponent=="") //or getImageAbsRect()
            m_captureCounter++;

        return 0;
    };
    m_buttonSnapshot.setCallback(&callbackSnapshot,this,NULL);

    function<int (void*,void*)> callbackUserManual = [&](void* objPtr, void* arguments)
    {
        ShellExecute(NULL, "open", "", NULL, NULL, SW_SHOWNORMAL); //TODO: Add a path to the User Manual
        return 0;
    };
    m_buttonUserManual.setCallback(&callbackUserManual,this,NULL);

    function<int (void*,void*)> callbackBugReport = [&](void* objPtr, void* arguments)
    {
        ShellExecute(NULL, "open", "", NULL, NULL, SW_SHOWNORMAL); //TODO: Add a path to the bug/feature request Github page
        return 0;
    };
    m_buttonReportBug.setCallback(&callbackBugReport,this,NULL);

    function<int (void*,void*)> callbackHelp = [&](void* objPtr, void* arguments)
    {
        m_popUpHelp.prompt();
        return 0;
    };
    m_buttonHelp.setCallback(&callbackHelp,this,NULL);

    function<int (void*,void*)> callbackComment = [&](void* objPtr, void* arguments)
    {
        bool componentSelected = m_imageViewer.enableCommenting();
        if(!componentSelected)
            m_buttonComment.toggleNoCallback();
        return 0;
    };
    m_buttonComment.setCallback(&callbackComment,this,NULL);

    function<int (void*,void*)> callbackSetComponentState = [&](void* objPtr, void* arguments)
    {
        CompState* state = (CompState*)arguments;
        Gui::setSelectedComponentState(*state);
        return 0;
    };
    CompState compFitted=COMP_FITTED, compNotFitted=COMP_NOT_FITTED, compError=COMP_ERROR, compUndefined=COMP_UNDEFINED;
    m_buttonComponentFitted.setCallback(&callbackSetComponentState,this,&compFitted);
    m_buttonComponentNotFitted.setCallback(&callbackSetComponentState,this,&compNotFitted);
    m_buttonComponentError.setCallback(&callbackSetComponentState,this,&compError);
    m_buttonComponentUndefined.setCallback(&callbackSetComponentState,this,&compUndefined);

    function<int (void*,void*)> callbackSetVisibleState = [&](void* objPtr, void* arguments)
    {
        m_visibleItems = 0;
        if(m_buttonVisibleMeasure.pressed())
            m_visibleItems |= VIS_MEASURE;
        if(m_buttonVisibleComment.pressed())
            m_visibleItems |= VIS_COMMENT;
        if(m_buttonVisibleFitted.pressed())
            m_visibleItems |= VIS_FITTED;
        if(m_buttonVisibleNotFitted.pressed())
            m_visibleItems |= VIS_NOT_FITTED;
        if(m_buttonVisibleError.pressed())
            m_visibleItems |= VIS_ERROR;
        if(m_buttonVisibleUndefined.pressed())
            m_visibleItems |= VIS_UNDEFINED;

        m_imageViewer.setVisibleItems(m_visibleItems);

        return 0;
    };
    m_buttonVisibleMeasure.setCallback(&callbackSetVisibleState,this,NULL);
    m_buttonVisibleComment.setCallback(&callbackSetVisibleState,this,NULL);
    m_buttonVisibleFitted.setCallback(&callbackSetVisibleState,this,NULL);
    m_buttonVisibleNotFitted.setCallback(&callbackSetVisibleState,this,NULL);
    m_buttonVisibleError.setCallback(&callbackSetVisibleState,this,NULL);
    m_buttonVisibleUndefined.setCallback(&callbackSetVisibleState,this,NULL);

    function<int (void*,void*)> callbackSearch = [&](void* objPtr, void* arguments)
    {
        Gui::searchForItem();
        return 0;
    };
    m_buttonSearch.setCallback(&callbackSearch,this,NULL);

    function<int (void*,void*)> callbackNextComponent = [&](void* objPtr, void* arguments)
    {
        Gui::getNextComponent();
        return 0;
    };
    m_buttonNextComponent.setCallback(&callbackNextComponent,this,NULL);

    function<int (void*,void*)> callbackMeasure = [&](void* objPtr, void* arguments)
    {
        m_imageViewer.toggleMeasurement();
        return 0;
    };
    m_buttonMeasure.setCallback(&callbackMeasure,this,NULL);

    function<int (void*,void*)> callbackFileSelected = [&](void* objPtr, void* arguments)
    {
        File* filePtr = (File*)arguments;
        return Gui::fileGotSelected(filePtr);
    };
    m_fileViewer.setSelectionCallback(&callbackFileSelected,this);


    m_fileViewer.linkToFileList(&m_files);//This is fairly illegal (breaks encapsulation) but this software is simple and sequential

    Gui::getWindowRect(&m_windowRect);



    //Manage argument inputs
    for(int i=1;i<argc;i++)//Ignore the first argument which is the path to the executable
    {
        string argvString = argv[i];
        if(argvString.substr(argvString.size()-8) == ".csvvise")
            Gui::openInspectionFile(argvString);
        else if(argvString.substr(argvString.size()-4) == ".jpg" ||
                argvString.substr(argvString.size()-4) == ".bmp" ||
                argvString.substr(argvString.size()-4) == ".png")
            Gui::loadImage(argvString);
        else if(argvString.substr(argvString.size()-4) == ".csv")
            Gui::loadPickAndPlace(argvString);
    }




    MouseAndKeyboardState mouseAndKeyboard;
    mouseAndKeyboard.mousePos.w=0;
    mouseAndKeyboard.mousePos.h=0;
    int numberOfKeys;
    mouseAndKeyboard.keyboardState = SDL_GetKeyboardState(&numberOfKeys);

    bool windowMoved=false;
    Uint32 windowMoveTime=0;
    SDL_Rect newWindowRect=m_windowRect;

    bool done=false;
    SDL_Event event;

    Uint32 windowID = SDL_GetWindowID(m_window);
    bool windowInFocus = true;

    //Main loop
    while(!done && Widget::waitForMyEvent(&event, windowID, windowInFocus))
    {
        //unsigned long timeBeggining = SDL_GetTicks();
        if(windowMoved && SDL_GetTicks()-windowMoveTime>50) //Update movement of window with a delay to allow resizing that moves the window to be dealt correctly
        {
            m_windowRect.x = newWindowRect.x;
            m_windowRect.y = newWindowRect.y;
            windowMoved = false;

            m_rootWidget.setRect(getWindowSize(),&m_windowRect);
        }

        switch(event.type)
        {
            case SDL_QUIT:
                if(!m_unsavedChanges || m_popUpExit.prompt()==POPUP_OK)
                    done = true;
                break;

            case SDL_WINDOWEVENT:
                switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_CLOSE:
                        if(!m_unsavedChanges || m_popUpExit.prompt()==POPUP_OK)
                            done = true;
                        break;

                    case SDL_WINDOWEVENT_MOVED:
                        windowMoved=true;
                        getWindowPosition(&newWindowRect);
                        windowMoveTime = SDL_GetTicks();
                        break;

                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        Gui::getWindowRect(&m_windowRect);
                        m_rootWidget.setRect(getWindowSize(), &m_windowRect);
                        break;

                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                    case SDL_WINDOWEVENT_TAKE_FOCUS:
                        windowInFocus = true;
                        break;

                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        windowInFocus = false;
                        break;
                }
                break;

            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                mouseAndKeyboard.mouseState = SDL_GetMouseState(&mouseAndKeyboard.mousePos.x, &mouseAndKeyboard.mousePos.y);
                break;

            case SDL_KEYDOWN:
                mouseAndKeyboard.keyboardState = SDL_GetKeyboardState(&numberOfKeys);
                switch(event.key.keysym.scancode)
                {
                    case SDL_SCANCODE_RETURN:
                    case SDL_SCANCODE_KP_ENTER:
                        if(m_workspace==WORKSPACE_INSPECTION)
                            Gui::getNextComponent();
                        break;

                    case SDL_SCANCODE_B:
                        if(mouseAndKeyboard.keyboardState[SDL_SCANCODE_LCTRL] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RCTRL])
                            m_buttonReportBug.executeAction();

                    case SDL_SCANCODE_C:
                        if(mouseAndKeyboard.keyboardState[SDL_SCANCODE_LSHIFT] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RSHIFT])
                            m_buttonVisibleComment.toggle();
                        break;

                    case SDL_SCANCODE_E:
                        if(mouseAndKeyboard.keyboardState[SDL_SCANCODE_LSHIFT] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RSHIFT])
                            m_buttonVisibleError.toggle();
                        else
                            Gui::setSelectedComponentState(COMP_ERROR);
                        break;

                    case SDL_SCANCODE_F:
                        if(mouseAndKeyboard.keyboardState[SDL_SCANCODE_LCTRL] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RCTRL])
                            Gui::searchForItem();
                        else if(mouseAndKeyboard.keyboardState[SDL_SCANCODE_LSHIFT] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RSHIFT])
                            m_buttonVisibleFitted.toggle();
                        else
                            Gui::setSelectedComponentState(COMP_FITTED);
                        break;

                    case SDL_SCANCODE_H:
                        if(mouseAndKeyboard.keyboardState[SDL_SCANCODE_LCTRL] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RCTRL])
                            m_buttonHelp.executeAction();
                        break;

                    case SDL_SCANCODE_M:
                        if(mouseAndKeyboard.keyboardState[SDL_SCANCODE_LCTRL] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RCTRL])
                            m_buttonUserManual.executeAction();
                        else if(mouseAndKeyboard.keyboardState[SDL_SCANCODE_LSHIFT] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RSHIFT])
                            m_buttonVisibleMeasure.toggle();
                        else
                            m_buttonMeasure.toggle();
                        break;

                    case SDL_SCANCODE_N:
                        if(mouseAndKeyboard.keyboardState[SDL_SCANCODE_LSHIFT] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RSHIFT])
                            m_buttonVisibleNotFitted.toggle();
                        else
                            Gui::setSelectedComponentState(COMP_NOT_FITTED);
                        break;

                    case SDL_SCANCODE_O:
                        if(mouseAndKeyboard.keyboardState[SDL_SCANCODE_LCTRL] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RCTRL])
                            Gui::openInspectionFile();
                        break;

                    case SDL_SCANCODE_P:
                        if(mouseAndKeyboard.keyboardState[SDL_SCANCODE_LCTRL] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RCTRL])
                            m_buttonSnapshot.executeAction();
                        break;

                    case SDL_SCANCODE_S:
                        if(mouseAndKeyboard.keyboardState[SDL_SCANCODE_LCTRL] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RCTRL])
                        {
                            if(m_savePath=="")
                            {
                                m_savePath = m_inspectionReport.saveInspectionFile();
                                if(m_savePath != "")//saved
                                    Gui::reportWasSaved();
                            }
                            else
                            {
                                m_inspectionReport.saveInspectionFile(m_savePath);
                                Gui::reportWasSaved();
                            }
                        }
                        break;

                    case SDL_SCANCODE_U:
                        if(mouseAndKeyboard.keyboardState[SDL_SCANCODE_LSHIFT] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RSHIFT])
                            m_buttonVisibleUndefined.toggle();
                        else
                            Gui::setSelectedComponentState(COMP_UNDEFINED);
                        break;

                    default:
                        break;
                }
                break;

            case SDL_KEYUP:
                mouseAndKeyboard.keyboardState = SDL_GetKeyboardState(&numberOfKeys);

                switch(event.key.keysym.scancode)
                {
                    case SDL_SCANCODE_C:
                        if(mouseAndKeyboard.keyboardState[SDL_SCANCODE_LCTRL] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RCTRL])
                            callbackCalibrate(this,NULL);
                        else if(m_workspace==WORKSPACE_INSPECTION && !mouseAndKeyboard.keyboardState[SDL_SCANCODE_LSHIFT] && !mouseAndKeyboard.keyboardState[SDL_SCANCODE_RSHIFT])
                        {
                            if(m_buttonComment.pressed())
                                m_imageViewer.disableCommenting();
                            else
                                callbackComment(this,NULL);
                        }
                        break;

                    case SDL_SCANCODE_I:
                        if(mouseAndKeyboard.keyboardState[SDL_SCANCODE_LCTRL] || mouseAndKeyboard.keyboardState[SDL_SCANCODE_RCTRL])
                            callbackInspect(this,NULL);
                        break;

                    default:
                        break;
                }
                break;

            default:
                break;
        }

        if(m_savePath!="" && SDL_GetTicks() - m_lastTimeSave > TIME_AUTOSAVE)
        {
            m_inspectionReport.saveInspectionFile(m_savePath);
            Gui::reportWasSaved();
        }

        m_rootWidget.manageEvent(event,mouseAndKeyboard);
        m_rootWidget.manageDraw();

        //unsigned long timeEnd = SDL_GetTicks();
        //cout <<"Time: " <<timeEnd-timeBeggining <<endl;
        //SDL_Delay(10);//In case we have a swamp of events? Makes it a little laggy and I don't like it...

    }

    return 0;
}



void Gui::getWindowPosition(SDL_Rect* windowRect)
{
    int x, y;
    SDL_GetWindowPosition(m_window, &x, &y);

    windowRect->x = x;
    windowRect->y = y;
}

void Gui::getWindowSize(SDL_Rect* windowRect)
{
    int w, h;
    SDL_GetWindowSize(m_window, &w, &h);

    windowRect->w = w;
    windowRect->h = h;
}

SDL_Rect Gui::getWindowSize()
{
    SDL_Rect windowRect;

    windowRect.x=0;
    windowRect.y=0;
    getWindowSize(&windowRect);

    return windowRect;
}

void Gui::getWindowRect(SDL_Rect* windowRect)
{
    getWindowPosition(windowRect);
    getWindowSize(windowRect);
}




void Gui::setWorkspace(Workspace workspace)
{
    m_workspace = workspace;
    m_imageViewer.setWorkspace(workspace);
}

int Gui::loadImage(string imagePath)
{
    if(imagePath!="")
    {
        File* image=NULL;

        try
        {
            imagePath = fs::u8path(imagePath).u8string(); //Convert from UTF8 encoding (the format outputed by VISE) to UTF-8 (this is just to get an error if not UTF8
        }
        catch(std::exception &e)
        {
            imagePath = fs::path(imagePath).u8string(); //Convert from system page code (the format outputed by Excel) to UTF8 encoding
        }

        for(size_t i=0;i<m_files.size();i++)
        {
            if(m_files[i]->getPath() == imagePath)//Already loaded
            {
                m_files[i]->createFile(imagePath);//Reload it, in case it has changed since last loading
                image = m_files[i];
                cout <<"Reloaded this image" <<endl;
            }
        }

        if(image == NULL)
        {
            image = new Image(m_renderer,imagePath);
            if(image->loaded())
                m_files.push_back(image);
            else
                cout <<"Couldn't load the image at " <<imagePath <<endl;
        }

        if(image->loaded())
        {
            m_fileViewer.selectFileAndCallback(image);
            m_inspectionReport.addImage((Image*)image);
            m_rootWidget.needGeneralDraw();
            m_unsavedChanges = true;
        }

        return 0;
    }
    else
        return 1;
}

int Gui::loadPickAndPlace(string pickAndPlacePath)
{
    if(pickAndPlacePath!="")
    {
        File* pickAndPlace=NULL;

        for(size_t i=0;i<m_files.size();i++)
        {
            if(m_files[i]->getPath() == pickAndPlacePath)//Already loaded
            {
                m_files[i]->createFile(pickAndPlacePath);//Reload it, in case it has changed since last loading
                pickAndPlace = m_files[i];
                cout <<"Reloaded this pick and place" <<endl;
            }
        }

        if(pickAndPlace == NULL)
        {
            pickAndPlace = new PickAndPlace();
            int errorCreatingFile = pickAndPlace->createFile(pickAndPlacePath);
            if(errorCreatingFile==0)
            {
                if(m_pickAndPlace!=NULL)
                {
                    for(size_t i=0;i<m_files.size();i++)
                    {
                        if(m_files[i]->getPath()==m_pickAndPlace->getPath())
                        {
                            m_files.erase(m_files.begin()+i);
                            delete m_pickAndPlace;
                            break;
                        }
                    }
                }

                m_pickAndPlace = (PickAndPlace*)pickAndPlace;
                m_files.push_back(pickAndPlace);
                m_fileViewer.selectFileAndCallback(pickAndPlace);
                m_popUpRefDes.setPossibleAnswers(m_pickAndPlace->getDesignatorList());
                m_inspectionReport.setPickAndPlace(m_pickAndPlace);
                m_unsavedChanges = true;
            }
            else
            {
                cerr <<"Couldn't load the pick and place at " <<pickAndPlacePath <<". It might be corrupted or empty" <<endl;

                if(errorCreatingFile==5)//No polarized column
                {
                    m_popUpOkButton.setTitle("No \"Polarized\" column");
                    m_popUpOkButton.setText("The column \"Polarized\" is not\npresent in your pick and place file.\nRefer to the user manual, section Pick and Place.");
                    m_popUpOkButton.prompt();
                }

                return errorCreatingFile;
            }
        }
    }

    return 0;
}

int Gui::openInspectionFile(string inspectionFilePath/*=""*/)
{
    if(m_unsavedChanges)
    {
        m_popUpYesNo.setText("You have unsaved changes.\nDo you really want to open another file?",m_fontPopUp);
        m_popUpYesNo.setButtonOkText("Open File");
        m_popUpYesNo.setButtonNoText("Go back");

        if(m_popUpYesNo.prompt()!=POPUP_OK)
            return 1;
    }


    bool opennedSomething;
    if(inspectionFilePath=="")
        opennedSomething = m_inspectionReport.openInspectionFile();
    else
        opennedSomething = m_inspectionReport.openInspectionFile(inspectionFilePath);

    if(opennedSomething)
        m_unsavedChanges = false;

    for(size_t i=0;i<m_files.size();i++)
    {
        if(m_files[i]->getType()==FILE_IMAGE)
        {
            m_fileViewer.selectFileAndCallback(m_files[i]);
            return 0;
        }
    }

    return 1;
}

int Gui::fileGotSelected(File* file)
{
    if(file==NULL)
        return 1;

    if(file->getType()==FILE_IMAGE)
        m_imageViewer.setCurrentImage((Image*)file, &m_windowRect);

    return 0;
}

void Gui::reportWasSaved()
{
    m_tooltipSaved.show();
    m_lastTimeSave = SDL_GetTicks();
    m_unsavedChanges = false;
}

bool Gui::saveScreenshot(string defaultName, SDL_Rect area)
{
    nfdu8char_t* outPathChar = NULL;
    nfdfilteritem_t filters[1] = { {"Images", "png"} };

    nfdresult_t result = NFD_SaveDialog(&outPathChar,filters,1,NULL,defaultName.c_str());
    if(result!=NFD_OKAY)
        return false;

    string outPath = outPathChar;
    delete outPathChar;


    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0,area.w,area.h,32,SDL_PIXELFORMAT_BGRA32);
    SDL_RenderReadPixels(m_renderer,&area,surface->format->format,surface->pixels,surface->pitch);

    if(surface!=NULL)
        IMG_SavePNG(surface, outPath.c_str());

    return true;
}

void Gui::startInspection()
{
    m_buttonInspect.press();
    m_barTools.show();
    m_imageViewer.setRect(0,TOPBAR_H*3+2,-RIGHTBAR_W,-TOPBAR_H*3-2);
    m_tooltipSaved.setPosition({5,TOPBAR_H*3+2+5},TOP_LEFT);

    Gui::setWorkspace(WORKSPACE_INSPECTION);
}

void Gui::stopInspection()
{
    m_buttonInspect.depress();
    m_buttonComment.depress();
    m_buttonMeasure.depress();
    m_barTools.hide();
    m_imageViewer.setRect(0,TOPBAR_H,-RIGHTBAR_W,-TOPBAR_H);
    m_tooltipSaved.setPosition({5,TOPBAR_H+5},TOP_LEFT);
}

int Gui::searchForItem(string designator, bool placeCursorOnComponent)
{
    m_popUpSearch.setPossibleAnswers(m_inspectionReport.getSearchableDesignatorList()); //A bit of a brute force to put it here but it can never miss a new addition

    if(designator.compare("")==0)
    {
        if(m_popUpSearch.prompt(&designator,"")!=POPUP_OK)
            return 0;
    }

    bool found=false;
    if(!m_imageViewer.selectAndShow(designator)) //Component is not in current image
    {
        string imagePath = m_inspectionReport.getImageOfItem(designator);
        if(imagePath != "")
        {
            for(size_t i=0;i<m_files.size();i++)
            {
                if(m_files[i]->getPath()==imagePath)
                {
                    m_fileViewer.selectFileAndCallback(m_files[i]);
                    found = m_imageViewer.selectAndShow(designator);
                }
            }
        }
    }

    if(placeCursorOnComponent)
    {
        //Place cursor at the middle of the viewer, on the component
        SDL_Rect viewerRect = m_imageViewer.getAbsRect();
        SDL_WarpMouseInWindow(m_window, viewerRect.x+viewerRect.w/2, viewerRect.y+viewerRect.h/2);
    }

    return (found?1:0);
}

void Gui::getNextComponent()
{
    bool done = m_imageViewer.selectAndShowUndefined();

    if(done)
    {
        m_popUpOkButton.setTitle("No more undefined components");
        m_popUpOkButton.setText("There are no undefined components left.\nWell done!",m_fontPopUp);
        m_popUpOkButton.prompt();
    }
}

void Gui::setSelectedComponentState(CompState state)
{
    m_imageViewer.setSelectedComponentState(state);
}



Gui::~Gui()
{
    for(int i=(int)m_files.size()-1;i>=0;i--)
    {
        delete m_files[i];
    }
    SDL_FreeSurface(m_icon);
    TTF_CloseFont(m_fontTooltipRefDes);
    TTF_CloseFont(m_fontTooltip);
    TTF_CloseFont(m_fontPopUpTextField);
    TTF_CloseFont(m_fontPopUp);
    TTF_CloseFont(m_fontButton);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
}
