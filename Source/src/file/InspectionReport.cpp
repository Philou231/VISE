#include "file/InspectionReport.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cmath>
#include <nfd.h>


#define CSVVISE_FORMAT_VERSION 1

//TODO: Make it so all characters in the csvvise are encoded into ASCII


using namespace std;
namespace fs = std::filesystem;


InspectionReport::InspectionReport() :
    m_pickAndPlace(NULL),
    m_numberOfComments(0),
    m_numberOfMeasurements(0)
{

}

void InspectionReport::setCallbackResetWorkspace(std::function<int (void*,void*)>* callbackFunction, void* objectPointer)
{
    m_callbackResetWorkspace = callbackFunction;
    m_callbackResetWorkspaceObjPtr = objectPointer;
}

void InspectionReport::setCallbackLoadPickAndPlace(std::function<int (void*,void*)>* callbackFunction, void* objectPointer)
{
    m_callbackLoadPickAndPlace = callbackFunction;
    m_callbackPickAndPlaceObjPtr = objectPointer;
}

void InspectionReport::setCallbackLoadImage(std::function<int (void*,void*)>* callbackFunction, void* objectPointer)
{
    m_callbackLoadImage = callbackFunction;
    m_callbackImageObjPtr = objectPointer;
}

void InspectionReport::setPickAndPlace(PickAndPlace* pickAndPlace)
{
    m_pickAndPlace = pickAndPlace;
    vector<Component> components = m_pickAndPlace->getAllComponents();

    for(int i=(int)m_items.size()-1;i>=0;i--)//Remove all components
    {
        if(m_items[i]->type==ITEM_COMPONENT)
        {
            delete m_items[i];
            m_items.erase(m_items.begin()+i);
        }
    }
    for(size_t i=0;i<components.size();i++)
    {
        Component* newComponent = new Component(components[i]);
        m_items.push_back(newComponent);
    }

    for(size_t i=0;i<m_imageCalibrations.size();i++)
    {
        if(m_imageCalibrations[i].markers.size()>=3)
        {
            for(size_t j=0;j<m_imageCalibrations[i].markers.size();j++)
            {
                m_imageCalibrations[i].markers[j] = m_pickAndPlace->getComponent(m_imageCalibrations[i].markers[j].designator);
            }
            InspectionReport::computeCalibration(&m_imageCalibrations[i]);
        }
    }
}

vector<string> InspectionReport::getSearchableDesignatorList()
{
    vector<string> designators;
    for(size_t i=0;i<m_items.size();i++)
    {
        designators.push_back(m_items[i]->designator);

        if(m_items[i]->designator.substr(0,5) == "VISE_")
            designators.push_back(m_items[i]->designator.substr(5));
    }

    return designators;
}

string InspectionReport::getUnits()
{
    if(m_pickAndPlace!=NULL)
        return m_pickAndPlace->getUnits();
    else
        return "";
}

void InspectionReport::addImage(Image* image)
{
    if(image==NULL)
        return;

    for(size_t i=0;i<m_imageCalibrations.size();i++)
    {
        if(m_imageCalibrations[i].imagePath == image->getPath())
            return;//Already exists
    }

    ImageCalibration newCalibration;
    newCalibration.imagePath = image->getPath();
    newCalibration.imageW = image->getOriginalW();
    newCalibration.imageH = image->getOriginalH();

    m_imageCalibrations.push_back(newCalibration);
}

void InspectionReport::addCalibrationMarker(Image* image, SDL_Rect clickPosition, string designator)
{
    if(m_pickAndPlace==NULL)
        return;

    ImageCalibration* calibration=NULL;

    for(size_t i=0;i<m_imageCalibrations.size();i++)
    {
        if(m_imageCalibrations[i].imagePath == image->getPath())
        {
            calibration = &(m_imageCalibrations[i]);
            break;
        }
    }
    if(calibration==NULL)
    {
        ImageCalibration newCalibration;
        newCalibration.imagePath = image->getPath();
        newCalibration.imageW = image->getW();
        newCalibration.imageH = image->getH();

        m_imageCalibrations.push_back(newCalibration);
        calibration = &(m_imageCalibrations[m_imageCalibrations.size()-1]);
    }

    calibration->markers.push_back(m_pickAndPlace->getComponent(designator));
    calibration->markers.back().pos = clickPosition;
    if(calibration->markers.size()>3)
        calibration->markers.erase(calibration->markers.begin());

    if(calibration->markers.size()==3)//Time to recompute all the refdes available in this image
        InspectionReport::computeCalibration(calibration);
}

ImageCalibration InspectionReport::getCalibration(Image* image)
{
    for(size_t i=0;i<m_imageCalibrations.size();i++)
    {
        if(m_imageCalibrations[i].imagePath == image->getPath())
        {
            return m_imageCalibrations[i];
        }
    }

    ImageCalibration emptyCalibration;
    return emptyCalibration;
}

ImageCalibration* InspectionReport::getCalibrationPtr(string imagePath)
{
    for(size_t i=0;i<m_imageCalibrations.size();i++)
    {
        if(m_imageCalibrations[i].imagePath == imagePath)
        {
            return &(m_imageCalibrations[i]);
        }
    }

    return NULL;
}

bool InspectionReport::isCalibrated(ImageCalibration calibration)
{
    return (calibration.markers.size()==3);
}



void InspectionReport::computeCalibration(ImageCalibration* calibration)
{
    //This conversion uses translation, anisotropic scaling and rotation to map pick&place coordinates onto the image's system
    //p is for pick&place, i is for image
    ConversionParameters params;

    //Initial constants with short names
    const Dbl_Rect i0 = Utilities::toDblRect(calibration->markers[0].pos);
    const Dbl_Rect i1 = Utilities::toDblRect(calibration->markers[1].pos);
    const Dbl_Rect i2 = Utilities::toDblRect(calibration->markers[2].pos);
    const Dbl_Rect p0 = calibration->markers[0].posPickAndPlace;
    const Dbl_Rect p1 = calibration->markers[1].posPickAndPlace;
    const Dbl_Rect p2 = calibration->markers[2].posPickAndPlace;

    const Dbl_Rect vi0 = {i0.x - i1.x, i0.y - i1.y};
    const Dbl_Rect vi1 = {i2.x - i1.x, i2.y - i1.y};
    const Dbl_Rect vp0 = {p0.x - p1.x, p0.y - p1.y};
    const Dbl_Rect vp1 = {p2.x - p1.x, p2.y - p1.y};
    const double li0 = sqrt(pow(vi0.x,2) + pow(vi0.y,2));//Length of vector p0
    const double li1 = sqrt(pow(vi1.x,2) + pow(vi1.y,2));//Length of vector p1


    //SCALE: We can fix the scales by using two equations:
    //       length of vp0 = length of vector {vi0x*Sx, vi0y*Sy}
    //       length of vp1 = length of vector {vi1x*Sx, vi1y*Sy}
    if(vp0.y==0)
        params.Sx = li0/vp0.x;
    else if(vp1.y==0)
        params.Sx = li1/vp1.x;
    else
        params.Sx = sqrt( (pow(li0/vp0.y,2) - pow(li1/vp1.y,2))  /  (pow(vp0.x/vp0.y,2) - pow(vp1.x/vp1.y,2)) );
    if(vp0.x==0)
        params.Sy = li0/vp0.y;
    else if(vp1.x==0)
        params.Sy = li1/vp1.y;
    else
        params.Sy = sqrt( (pow(li0/vp0.x,2) - pow(li1/vp1.x,2))  /  (pow(vp0.y/vp0.x,2) - pow(vp1.y/vp1.x,2)) );

    const double crossProductI=(vi0.x*vi1.y)-(vi0.y*vi1.x);
    const double crossProductP=(vp0.x*vp1.y)-(vp0.y*vp1.x);// These two cross products help determine if the coordinate systems are mirrored
    bool mirrored = crossProductI*crossProductP < 0;
    if(mirrored) //Different signs, we need to flip one of the coordinates system
        params.Sy = -params.Sy;


    //ROTATION: In the case where the picture would be a bit rotated, it is preferable to add an angle correction
    double theta0 = atan2(vi0.y,vi0.x) - atan2(vp0.y*params.Sy,vp0.x*params.Sx);
    const double theta1 = atan2(vi1.y,vi1.x) - atan2(vp1.y*params.Sy,vp1.x*params.Sx);
    while(theta0-theta1>PI)
        theta0-=2*PI;
    while(theta1-theta0>PI)
        theta0+=2*PI;
    params.R = (theta0+theta1)/2;


    //TRANSLATION: Now that we have all the other parameters, we can compute the translation needed
    SDL_Rect tempI1 = InspectionReport::convertPickAndPlacetoImage(p1, params);
    params.Tx = i1.x - double(tempI1.x);
    params.Ty = i1.y - double(tempI1.y);


    calibration->conversionParams = params;
    for(size_t i=0;i<calibration->items.size();i++)
    {
        delete calibration->items[i];
    }
    calibration->items = InspectionReport::findAndConvertApplicableItems(calibration);
}

SDL_Rect InspectionReport::convertPickAndPlacetoImage(Dbl_Rect pickAndPlacePos, ConversionParameters params)
{
    SDL_Rect imageCoord;
    imageCoord.x = round(pickAndPlacePos.x*params.Sx*cos(params.R) - pickAndPlacePos.y*params.Sy*sin(params.R) + params.Tx);
    imageCoord.y = round(pickAndPlacePos.x*params.Sx*sin(params.R) + pickAndPlacePos.y*params.Sy*cos(params.R) + params.Ty);
    return imageCoord;
}

Dbl_Rect InspectionReport::convertImageToPickAndPlace(SDL_Rect imagePos, ConversionParameters params)
{
    Dbl_Rect pickAndPlacePos;
    pickAndPlacePos.x = ( (double(imagePos.x)-params.Tx)*cos(params.R) + (double(imagePos.y)-params.Ty)*sin(params.R)) / params.Sx;
    pickAndPlacePos.y = (-(double(imagePos.x)-params.Tx)*sin(params.R) + (double(imagePos.y)-params.Ty)*cos(params.R)) / params.Sy;
    return pickAndPlacePos;
}

Dbl_Rect InspectionReport::convertImageToPickAndPlace(SDL_Rect imagePos, string imagePath)
{
    ImageCalibration* calib = InspectionReport::getCalibrationPtr(imagePath);
    if(calib==NULL)
        return {0.0, 0.0, 0.0, 0.0};

    return InspectionReport::convertImageToPickAndPlace(imagePos, calib->conversionParams);
}

bool InspectionReport::getComponentState(std::string designator, CompState* state)
{
    for(size_t i=0;i<m_items.size();i++)
    {
        if(designator == m_items[i]->designator && m_items[i]->type==ITEM_COMPONENT)
        {
            Component* component = (Component*)m_items[i];

            if(state!=NULL)
                (*state) = component->state;
            return true;
        }
    }

    return false;
}

bool InspectionReport::getComponentPolarized(string designator, bool* polarized)
{
    for(size_t i=0;i<m_items.size();i++)
    {
        if(designator == m_items[i]->designator && m_items[i]->type==ITEM_COMPONENT)
        {
            Component* component = (Component*)m_items[i];

            if(polarized!=NULL)
                (*polarized) = component->polarized;
            return true;
        }
    }

    return false;
}

void InspectionReport::setComponentState(std::string designator, CompState state)
{
    for(size_t i=0;i<m_items.size();i++)
    {
        if(designator == m_items[i]->designator && m_items[i]->type==ITEM_COMPONENT)
        {
            Component* component = (Component*)m_items[i];

            if(component->state != state)
                cout <<designator <<" => " <<PickAndPlace::toString(state) <<endl;
            else
                cout <<designator <<" == " <<PickAndPlace::toString(state) <<endl;
            component->state = state;
            break;
        }
    }
    for(size_t j=0;j<m_imageCalibrations.size();j++)
    {
        for(size_t i=0;i<m_imageCalibrations[j].items.size();i++)
        {
            if(designator == m_imageCalibrations[j].items[i]->designator && m_imageCalibrations[j].items[i]->type==ITEM_COMPONENT)
            {
                Component* component = (Component*)m_imageCalibrations[j].items[i];
                component->state = state;
                break;
            }
        }
    }
}

bool InspectionReport::setItemComment(std::string designator, std::string comment)
{
    bool erasedItem = false;

    for(size_t i=0;i<m_items.size();i++)
    {
        if(designator == m_items[i]->designator)
        {
            m_items[i]->comment = comment;

            if(m_items[i]->type == ITEM_COMMENT && comment == "")//We must erase this item
            {
                delete m_items[i];
                m_items.erase(m_items.begin()+i);
                erasedItem = true;
            }

            break;
        }
    }
    for(size_t j=0;j<m_imageCalibrations.size();j++)
    {
        for(size_t i=0;i<m_imageCalibrations[j].items.size();i++)
        {
            if(designator == m_imageCalibrations[j].items[i]->designator)
            {
                m_imageCalibrations[j].items[i]->comment = comment;

                if(m_imageCalibrations[j].items[i]->type == ITEM_COMMENT && comment == "")//We must erase this item
                {
                    delete m_imageCalibrations[j].items[i];
                    m_imageCalibrations[j].items.erase(m_imageCalibrations[j].items.begin()+i);
                }

                break;
            }
        }
    }

    return erasedItem;
}

void InspectionReport::addComment(string comment, SDL_Rect position, string imagePath)
{
    string designator;

    do //Find the first available designator after the max comment (keep track of deleted comments)
    {
        m_numberOfComments++;
        designator = "VISE_Comment"+to_string(m_numberOfComments);
    }
    while(InspectionReport::getItem(designator,NULL));

    InspectionReport::addComment(designator,comment,position,imagePath);
}


void InspectionReport::addComment(string designator, string comment, SDL_Rect position, string imagePath) //This function does not increment m_numberOfComments, it assumes it has been done
{
    ImageCalibration* calib = InspectionReport::getCalibrationPtr(imagePath);

    Comment* newComment = new Comment;
    newComment->designator = designator;
    newComment->pos = position;
    newComment->posPickAndPlace = InspectionReport::convertImageToPickAndPlace(position,calib->conversionParams);

    newComment->imagePath = imagePath;
    newComment->comment = comment;

    m_items.push_back(newComment);

    Comment* newCommentDifferentPtr = new Comment(*newComment);
    calib->items.push_back(newCommentDifferentPtr);
}

bool InspectionReport::getItem(std::string designator, Item* item)
{
    for(size_t i=0;i<m_items.size();i++)
    {
        if(m_items[i]->designator == designator)
        {
            if(item!=NULL)
                (*item) = (*m_items[i]);
            return true; //found it
        }
    }

    return false;
}

void InspectionReport::deleteItem(string designator)
{
    if(InspectionReport::getItemType(designator) == ITEM_COMPONENT) //You can't delete a component
        InspectionReport::setItemComment(designator,"");
    else
    {
        for(size_t i=0;i<m_items.size();i++)
        {
            if(designator == m_items[i]->designator)
            {
                delete m_items[i];
                m_items.erase(m_items.begin()+i);
                break;
            }
        }
        for(size_t j=0;j<m_imageCalibrations.size();j++)
        {
            for(size_t i=0;i<m_imageCalibrations[j].items.size();i++)
            {
                if(designator == m_imageCalibrations[j].items[i]->designator)
                {
                    delete m_imageCalibrations[j].items[i];
                    m_imageCalibrations[j].items.erase(m_imageCalibrations[j].items.begin()+i);
                    break;
                }
            }
        }
    }
}

string InspectionReport::getFirstUndefined(string imagePath)
{
    for(size_t i=0;i<m_imageCalibrations.size();i++)
    {
        if(m_imageCalibrations[i].imagePath == imagePath)
        {
            for(size_t j=0;j<m_imageCalibrations[i].items.size();j++)
            {
                if(m_imageCalibrations[i].items[j]->type==ITEM_COMPONENT)
                {
                    Component* component = (Component*)m_imageCalibrations[i].items[j];

                    if(component->state==COMP_UNDEFINED)
                        return component->designator;
                }
            }

            break;
        }
    }

    return "";
}

string InspectionReport::getImageOfItem(string designator)
{
    for(size_t i=0;i<m_imageCalibrations.size();i++)
    {
        for(size_t j=0;j<m_imageCalibrations[i].items.size();j++)
        {
            if(m_imageCalibrations[i].items[j]->designator == designator)
                return m_imageCalibrations[i].imagePath;
        }
    }

    return "";
}

string InspectionReport::getItemComment(string designator)
{
    for(size_t i=0;i<m_items.size();i++)
    {
        if(m_items[i]->designator == designator)
            return m_items[i]->comment;
    }

    return "";
}

ItemType InspectionReport::getItemType(std::string designator)
{
    for(size_t i=0;i<m_items.size();i++)
    {
        if(m_items[i]->designator == designator)
            return m_items[i]->type;
    }

    return ITEM_UNDEFINED;
}

void InspectionReport::addMeasurement(SDL_Rect pos, SDL_Rect pos2, string imagePath)
{
    string designator;

    do //Find the first available designator (don't keep track of deleted measurements)
    {
        m_numberOfMeasurements++;
        designator = "VISE_Measurement"+to_string(m_numberOfMeasurements);
    }
    while(InspectionReport::getItem(designator,NULL));

    Measurement measurement;

    measurement.designator = designator;
    measurement.pos = pos;
    measurement.pos2 = pos2;
    measurement.imagePath = imagePath;

    InspectionReport::addMeasurement(measurement);
}

void InspectionReport::addMeasurement(Measurement measurement)
{
    ImageCalibration* calib = InspectionReport::getCalibrationPtr(measurement.imagePath);

    measurement.posPickAndPlace = InspectionReport::convertImageToPickAndPlace(measurement.pos, calib->conversionParams);
    measurement.posPickAndPlace2 = InspectionReport::convertImageToPickAndPlace(measurement.pos2, calib->conversionParams);
    measurement.length = Utilities::computeLength(measurement.posPickAndPlace, measurement.posPickAndPlace2);
    measurement.angle = Utilities::computeAngle(measurement.posPickAndPlace, measurement.posPickAndPlace2);

    Measurement* newMeasurement = new Measurement(measurement);
    m_items.push_back(newMeasurement);

    Measurement* newMeasurementDifferentPtr = new Measurement(measurement);
    calib->items.push_back(newMeasurementDifferentPtr);
}

vector<Item*> InspectionReport::findAndConvertApplicableItems(ImageCalibration* calibration)
{
    vector<Item*> items;

    if(calibration==NULL || calibration->markers.size()<3)
        return items;



    //All layers used in the calibration process are accepted layers (which means up to 3 layers per image)
    vector<string> acceptedLayers;
    for(size_t i=0;i<calibration->markers.size();i++)
    {
        string layer = calibration->markers[i].layer;

        bool different=true;
        for(size_t j=0;j<acceptedLayers.size();j++)
        {
            if(layer == acceptedLayers[j])
            {
                different=false;
                break;
            }
        }
        if(different)
            acceptedLayers.push_back(layer);
    }



    for(size_t i=0;i<m_items.size();i++)
    {
        bool isAssociated=false;
        for(size_t j=0;j<acceptedLayers.size();j++)
        {
            if(m_items[i]->type==ITEM_COMPONENT)
            {
                Component* component = (Component*)m_items[i];

                if(component->layer == acceptedLayers[j]) //Good layer for this image
                {
                    isAssociated=true;
                    break;
                }
            }
            else if(m_items[i]->type==ITEM_COMMENT)
            {
                Comment* comment = (Comment*)m_items[i];

                if(comment->imagePath == calibration->imagePath) //Good image
                {
                    isAssociated=true;
                    break;
                }
            }
            else if(m_items[i]->type==ITEM_MEASUREMENT)
            {
                Measurement* measurement = (Measurement*)m_items[i];

                if(measurement->imagePath == calibration->imagePath) //Good image
                {
                    isAssociated=true;
                    break;
                }
            }
        }
        if(!isAssociated)
            continue;


        SDL_Rect testPos = InspectionReport::convertPickAndPlacetoImage(m_items[i]->posPickAndPlace, calibration->conversionParams);

        if(testPos.x>=0 && testPos.y>=0 && testPos.x<calibration->imageW && testPos.y<calibration->imageH)
        {
            Item* newItem = NULL;
            switch(m_items[i]->type)
            {
                default:
                case ITEM_COMPONENT:
                    newItem = new Component( *( (Component*)m_items[i] ) );
                    break;

                case ITEM_COMMENT:
                    newItem = new Comment( *( (Comment*)m_items[i] ) );
                    break;

                case ITEM_MEASUREMENT:
                    newItem = new Measurement( *( (Measurement*)m_items[i] ) );
                    break;
            }
            newItem->pos = testPos;
            items.push_back(newItem);
        }
    }

    return items;
}



string addExtension(string path, string ext)
{
    ext = string(".")+ext;
    if(path.substr(path.size()-ext.size()) == ext) //Extension is already there
        return path;
    else
        return path+ext;
}

string makePathRelativeTo(string path, string fileRootOfRelative)
{
    string relativePath = fs::relative(fs::u8path(path), fs::u8path(fileRootOfRelative).remove_filename()).string();

    if(fs::path::preferred_separator == '\\') //On windows
        relativePath = Utilities::convertBackSlashesToFront(relativePath);

    return relativePath;
}

void InspectionReport::saveFormatVersion1(string savePath)
{
    ofstream outFile(fs::u8path(savePath), ofstream::trunc);

    if(m_pickAndPlace==NULL)
    {
        outFile <<"The current session does not contain a pick and place file" <<endl;
        outFile.close();
        return;
    }
    if(m_imageCalibrations.size()==0)
    {
        outFile <<"The current session does not contain any images" <<endl;
        outFile.close();
        return;
    }


    const string sep="\t";
    //Writing a Byte-Order-Mark to specify a UTF-8 encoding to Excel does not work :(
    //It can understand the UTF-8 encoding but it still won't understand that the file is Tab-separated
    //This means it is simply best to


    //Save the version so we can update the format
    outFile <<"Version" <<sep <<1 <<endl;

    //Save the paths
    outFile <<"PickAndPlace" <<sep <<makePathRelativeTo(m_pickAndPlace->getPath(), savePath) <<endl;
    for(size_t i=0;i<m_imageCalibrations.size();i++)
    {
        outFile <<"Image" <<sep <<makePathRelativeTo(m_imageCalibrations[i].imagePath, savePath) <<endl;
    }
    for(size_t i=0;i<m_imageCalibrations.size();i++)
    {
        for(size_t j=0;j<m_imageCalibrations[i].markers.size();j++)
        {
            outFile <<"Calibration" <<sep <<i <<sep <<m_imageCalibrations[i].markers[j].designator
                                              <<sep <<m_imageCalibrations[i].markers[j].pos.x
                                              <<sep <<m_imageCalibrations[i].markers[j].pos.y <<endl;
        }
    }
    outFile <<"CommentCount" <<sep <<m_numberOfComments <<endl;
    outFile <<"MeasurementCount" <<sep <<m_numberOfMeasurements <<endl;

    outFile <<endl;

    outFile <<"Components" <<endl;
    for(size_t i=0;i<m_items.size();i++)
    {
        if(m_items[i]->type != ITEM_COMPONENT)
            continue;

        Component* component = (Component*)m_items[i];

        outFile <<component->designator <<sep;
        switch(component->state)
        {
            case COMP_UNDEFINED:
                outFile <<"Undefined";
                break;

            case COMP_FITTED:
                outFile <<"Fitted";
                break;

            case COMP_NOT_FITTED:
                outFile <<"Not Fitted";
                break;

            case COMP_ERROR:
                outFile <<"Error";
                break;

            default:
                outFile <<"Unknown";
                break;
        }

        if(component->comment!="")
            outFile <<sep <<component->comment;
        outFile <<endl;
    }
    outFile <<endl;

    if(m_numberOfComments>0)
    {
        outFile <<"Comments" <<endl;
        for(size_t i=0;i<m_items.size();i++)
        {
            if(m_items[i]->type != ITEM_COMMENT)
                continue;

            Comment* comment = (Comment*)m_items[i];

            outFile <<comment->designator.substr(5) <<sep <<comment->comment <<sep <<comment->pos.x <<sep <<comment->pos.y <<sep <<makePathRelativeTo(comment->imagePath, savePath) <<endl;
        }

        outFile <<endl;
    }


    if(m_numberOfMeasurements>0)
    {
        outFile <<"Measurements" <<endl;
        for(size_t i=0;i<m_items.size();i++)
        {
            if(m_items[i]->type != ITEM_MEASUREMENT)
                continue;

            Measurement* measurement = (Measurement*)m_items[i];
            outFile <<measurement->designator.substr(5) <<sep <<setprecision(4) <<measurement->length <<sep <<InspectionReport::getUnits() <<sep <<measurement->angle <<sep <<measurement->comment <<sep \
                    <<measurement->pos.x <<sep <<measurement->pos.y <<sep <<measurement->pos2.x <<sep <<measurement->pos2.y <<sep \
                    <<makePathRelativeTo(measurement->imagePath, savePath) <<endl;
        }
    }


    outFile.close();
}

bool InspectionReport::openFormatVersion1(string filePath)
{
    ifstream reportFile(fs::u8path(filePath));

    fs::path absoluteFilePath(fs::u8path(filePath));
    absoluteFilePath = absoluteFilePath.remove_filename();

    string line;

    enum ReadState{STATE_HEADER,STATE_COMPONENT,STATE_COMMENT,STATE_MEASUREMENT};

    ReadState readState=STATE_HEADER;
    while(getline(reportFile, line))
    {
        vector<string> lineVector = Utilities::csvLineToCells(line,'\t');

        if(lineVector.size()==0)
        {
            continue;
        }
        else if(lineVector.size()==1)
        {
            if(lineVector[0]=="Components")
                readState=STATE_COMPONENT;
            else if(lineVector[0]=="Comments")
                readState=STATE_COMMENT;
            else if(lineVector[0]=="Measurements")
                readState=STATE_MEASUREMENT;

            continue;
        }

        if(readState==STATE_HEADER)
        {
            if(lineVector[0]=="Version")
            {
                continue;
            }
            else if(lineVector[0]=="PickAndPlace" && lineVector.size()>=2)
            {
                if(m_callbackLoadPickAndPlace!=NULL)
                {
                    string absoluteFilePathString = absoluteFilePath.string() + lineVector[1];
                    (*m_callbackLoadPickAndPlace)(m_callbackPickAndPlaceObjPtr, (void*)absoluteFilePathString.c_str());
                }
                continue;
            }
            else if(lineVector[0]=="Image" && lineVector.size()>=2)
            {
                if(m_callbackLoadImage!=NULL)
                {
                    string absoluteFilePathString = absoluteFilePath.string() + lineVector[1];
                    (*m_callbackLoadImage)(m_callbackImageObjPtr, (void*)absoluteFilePathString.c_str());
                }
                continue;
            }
            else if(lineVector[0]=="Calibration" && lineVector.size()>=5)
            {
                if(m_imageCalibrations.size() >= stoul(lineVector[1])+1 && m_pickAndPlace!=NULL)
                {
                    size_t index = stoul(lineVector[1]);

                    if(m_imageCalibrations[index].markers.size()>=3)//Empty the old ones for these fresh ones
                        m_imageCalibrations[index].markers.clear();

                    Component marker = m_pickAndPlace->getComponent(lineVector[2]);
                    marker.pos.x = stoi(lineVector[3]);
                    marker.pos.y = stoi(lineVector[4]);
                    m_imageCalibrations[index].markers.push_back(marker);

                    if(m_imageCalibrations[index].markers.size()>=3)//We added all the markers for this image
                        InspectionReport::computeCalibration(&(m_imageCalibrations[index]));
                }
                continue;
            }
            else if(lineVector[0]=="CommentCount" && lineVector.size()>=2)
            {
                m_numberOfComments = stoi(lineVector[1]);
            }
            else if(lineVector[0]=="MeasurementCount" && lineVector.size()>=2)
            {
                m_numberOfMeasurements = stoi(lineVector[1]);
            }
            else
            {
                cerr <<"Unidentified element in the report file: \"" <<lineVector[0] <<"\"" <<endl;
            }
        }
        else if(readState==STATE_COMPONENT)
        {
            CompState state;

            if(lineVector[1] == "Undefined")
                state = COMP_UNDEFINED;
            else if(lineVector[1] == "Fitted")
                state = COMP_FITTED;
            else if(lineVector[1] == "Not Fitted")
                state = COMP_NOT_FITTED;
            else if(lineVector[1] == "Error")
                state = COMP_ERROR;
            else if(lineVector[1] == "Unknown")
                state = COMP_UNDEFINED;
            else
                state = COMP_UNDEFINED;

            InspectionReport::setComponentState(lineVector[0],state);
            if(lineVector.size()>=3)
                InspectionReport::setItemComment(lineVector[0],lineVector[2]);
        }
        else if(readState==STATE_COMMENT)
        {
            if(lineVector.size()<5)
                continue;

            SDL_Rect position;
            position.x = stoi(lineVector[2]);
            position.y = stoi(lineVector[3]);

            string absoluteFilePathString = absoluteFilePath.string() + lineVector[4];

            InspectionReport::addComment("VISE_"+lineVector[0], lineVector[1], position, absoluteFilePathString);
        }
        else if(readState==STATE_MEASUREMENT)
        {
            if(lineVector.size()<7)
                continue;

            Measurement measurement;

            measurement.designator = "VISE_"+lineVector[0];
            //lineVector[1->3] are length, units, and angle. They can be ignored because they will be recalculated
            measurement.comment = lineVector[4];

            string absoluteFilePathString = absoluteFilePath.string() + lineVector[9];
            measurement.imagePath = absoluteFilePathString;

            measurement.pos.x = stoi(lineVector[5]);
            measurement.pos.y = stoi(lineVector[6]);

            measurement.pos2.x = stoi(lineVector[7]);
            measurement.pos2.y = stoi(lineVector[8]);

            InspectionReport::addMeasurement(measurement);
        }
    }

    return true;
}


string InspectionReport::saveInspectionFile()
{
    nfdfilteritem_t filters[1] = { {"Inspection File", "csvvise"} };
    char* savePathChar = NULL;
    string savePath;
    if(NFD_OKAY == NFD_SaveDialog(&savePathChar, filters, 1, NULL, NULL))
    {
        savePath = savePathChar;
        delete savePathChar;
    }
    else
        return "";

    savePath = addExtension(savePath,"csvvise");

    return saveInspectionFile(savePath);
}

string InspectionReport::saveInspectionFile(string savePath)
{
    if(savePath=="")
    {
        savePath = InspectionReport::saveInspectionFile();
        if(savePath=="")
            return "";
    }

    if(CSVVISE_FORMAT_VERSION==1)
    {
        InspectionReport::saveFormatVersion1(savePath);
        return savePath;
    }
    else
    {
        cerr <<"Cannot save file. Format version " <<CSVVISE_FORMAT_VERSION <<" has not been defined" <<endl;
        return "";
    }
}

bool InspectionReport::openInspectionFile()
{
    nfdfilteritem_t filters[1] = { {"Inspection File", "csvvise"} };
    char* openPathChar = NULL;
    string openPath;
    if(NFD_OKAY == NFD_OpenDialog(&openPathChar, filters, 1, NULL))
    {
        openPath = openPathChar;
        delete openPathChar;
    }
    else
        return false;

    return InspectionReport::openInspectionFile(openPath);
}

bool InspectionReport::openInspectionFile(string path)
{
    //Extract the version string to decide which format version to use
    ifstream inputFile(fs::u8path(path));
    string versionLine;
    getline(inputFile, versionLine);

    size_t index = versionLine.find_first_of('\t');
    if(index==string::npos)//did not find separator
    {
        cerr <<"File is corrupted or not the appropriate type. Could not find version string" <<endl;
        return false;
    }
    string versionString=versionLine.substr(0,index);
    string numberString=versionLine.substr(index+1);
    int versionNumber = stoi(numberString);

    if(versionString != "Version" || versionNumber<1)//"Version" was not the first word or the number is incorrect
    {
        cerr <<"File is corrupted or not the appropriate type. Could not find version string" <<endl;
        return false;
    }

    inputFile.close();


    if(m_callbackResetWorkspace!=NULL)
    {
        string inspectionFileName = Utilities::getFileName(path);
        (*m_callbackResetWorkspace)(m_callbackResetWorkspaceObjPtr, (void*)&inspectionFileName);
    }
    InspectionReport::resetInspectionReport();
    switch(versionNumber)
    {
        case 1:
            return InspectionReport::openFormatVersion1(path);
            break;

        default:
            cerr <<"File format version (" <<versionNumber <<") is higher than the maximum format known to this current software version (" <<CSVVISE_FORMAT_VERSION <<")" <<endl;
            break;
    }

    return false;
}

void InspectionReport::resetInspectionReport()
{
    for(size_t j=0;j<m_imageCalibrations.size();j++)
    {
        for(size_t i=0;i<m_imageCalibrations[j].items.size();i++)
        {
            delete m_imageCalibrations[j].items[i];
        }
    }
    m_imageCalibrations.clear();

    for(size_t i=0;i<m_items.size();i++)
    {
        delete m_items[i];
    }
    m_items.clear();
}


InspectionReport::~InspectionReport()
{
    InspectionReport::resetInspectionReport();
}
