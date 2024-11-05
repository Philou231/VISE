#ifndef INSPECTION_REPORT_HEADER
#define INSPECTION_REPORT_HEADER

#include <vector>
#include <functional>
#include "misc/Utilities.h"
#include "PickAndPlace.h"
#include "Image.h"


struct ConversionParameters
{
    double Tx=0.0;
    double Ty=0.0;
    double Sx=1.0;
    double Sy=1.0;
    double R=0.0;
};

struct ImageCalibration
{
    std::string imagePath="";
    int imageW=0;
    int imageH=0;

    std::vector<Component> markers;
    ConversionParameters conversionParams;

    std::vector<Item*> items; //Only the items present in a certain image. They have to be copies because an item can be in multiple pictures (2 different pos attributes)
};


class InspectionReport
{
    public:
        InspectionReport();

        void setCallbackResetWorkspace(std::function<int (void*,void*)>* callbackFunction, void* objectPointer);
        void setCallbackLoadPickAndPlace(std::function<int (void*,void*)>* callbackFunction, void* objectPointer);
        void setCallbackLoadImage(std::function<int (void*,void*)>* callbackFunction, void* objectPointer);

        void setPickAndPlace(PickAndPlace* pickAndPlace);
        std::vector<std::string> getSearchableDesignatorList(); //"Searchable" is mentionned because, for example, "Comment1" and "VISE_Comment1" are both added to the list
        std::string getUnits();

        void addImage(Image* image);
        void addCalibrationMarker(Image* image, SDL_Rect clickPosition, std::string designator);
        ImageCalibration getCalibration(Image* image);

        static bool isCalibrated(ImageCalibration calibration);

        void computeCalibration(ImageCalibration* calibration); //Computes the transformation matrix
        static SDL_Rect convertPickAndPlacetoImage(Dbl_Rect pickAndPlacePos, ConversionParameters params);
        static Dbl_Rect convertImageToPickAndPlace(SDL_Rect imagePos, ConversionParameters params);
        Dbl_Rect convertImageToPickAndPlace(SDL_Rect imagePos, std::string imagePath);

        bool getComponentState(std::string designator, CompState* state);
        bool getComponentPolarized(std::string designator, bool* polarized);
        void setComponentState(std::string designator, CompState state);
        bool setItemComment(std::string designator, std::string comment);
        void addComment(std::string comment, SDL_Rect position, std::string imagePath);
        bool getItem(std::string designator, Item* item);
        void deleteItem(std::string designator);
        std::string getFirstUndefined(std::string imagePath);
        std::string getImageOfItem(std::string designator);
        std::string getItemComment(std::string designator);
        ItemType getItemType(std::string designator);

        void addMeasurement(SDL_Rect pos, SDL_Rect pos2, std::string imagePath);
        void addMeasurement(Measurement measurement);

        std::string saveInspectionFile();
        std::string saveInspectionFile(std::string savePath);
        bool openInspectionFile();
        bool openInspectionFile(std::string path);

        ~InspectionReport();
    private:
        ImageCalibration* getCalibrationPtr(std::string imagePath);
        void addComment(std::string designator, std::string comment, SDL_Rect position, std::string imagePath);
        std::vector<Item*> findAndConvertApplicableItems(ImageCalibration* calibration);
        void saveFormatVersion1(string savePath);
        bool openFormatVersion1(string filePath);
        void resetInspectionReport();

    private:
        std::function<int (void*,void*)>* m_callbackResetWorkspace;
        void* m_callbackResetWorkspaceObjPtr;

        std::function<int (void*,void*)>* m_callbackLoadPickAndPlace;
        void* m_callbackPickAndPlaceObjPtr;

        std::function<int (void*,void*)>* m_callbackLoadImage;
        void* m_callbackImageObjPtr;


        PickAndPlace* m_pickAndPlace;
        unsigned int m_numberOfComments;
        unsigned int m_numberOfMeasurements;
        std::vector<Item*> m_items; //This is the main list with all the items where the states/comments are recorded

        std::vector<ImageCalibration> m_imageCalibrations;
};



#endif //INSPECTION_REPORT_HEADER
