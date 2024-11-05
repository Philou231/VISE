#include "file/PickAndPlace.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;


PickAndPlace::PickAndPlace():
    File(FILE_PICKANDPLACE)
{

}

int PickAndPlace::createFile(string path)
{
    m_path = path;
    m_components.clear();

    ifstream pickAndPlaceFile;
    try
    {
        pickAndPlaceFile.open(fs::u8path(path)); //Try the UTF8 encoding (the format outputed by VISE)
    }
    catch(std::exception &e)
    {
        pickAndPlaceFile.open(path); //Open with default page code encoding (the format outputed by Excel)
    }

    int numberOfBlankLines=0;
    bool headerFinished=false;

    int colRefDes=-1;
    int colLayer=-1;
    int colX=-1;
    int colY=-1;
    int colPolarized=-1;

    string line;
    while(getline(pickAndPlaceFile,line))
    {
        vector<string> lineVector = Utilities::csvLineToCells(line,',');

        if(numberOfBlankLines==3 && !headerFinished) //First row of list
        {
            headerFinished=true;

            for(size_t i=0;i<lineVector.size();i++)
            {
                string sub=lineVector[i];
                if(sub.size()==0)
                    continue;

                if(sub[0]=='\"')
                    sub = sub.substr(1); //Remove first quotes
                if(sub[sub.size()-1]=='\"')
                    sub = sub.substr(0,sub.length()-1);//Remove last quotes

                if(sub == "Designator")
                    colRefDes=i;
                else if(sub == "Layer")
                    colLayer=i;
                else if(sub.length()>8 && sub.substr(0,8) == "Center-X")//sub to remove the units
                    colX=i;
                else if(sub.length()>8 && sub.substr(0,8) == "Center-Y")//sub to remove the units
                    colY=i;
                else if(sub == "Polarized")
                    colPolarized=i;
            }
            if(colRefDes==-1 || colLayer==-1 || colX==-1 || colY==-1 || colPolarized==-1)
            {
                cerr <<"To use this software, you need a Pick and Place CSV file with at least the following columns: \"Designator\", \"Layer\", \"Polarized\", \"Center-X and \"Center-Y (with quotes)." <<endl <<endl;

                if(colPolarized==-1)
                    return 5;
                if(colY==-1)
                    return 4;
                if(colX==-1)
                    return 3;
                if(colLayer==-1)
                    return 2;
                if(colRefDes==-1)
                    return 1;

                return 1;
            }
        }
        else if(headerFinished) //List
        {
            Component component;

            for(size_t i=0;i<lineVector.size();i++)
            {
                string sub=lineVector[i];
                if(sub.size()==0)
                    continue;

                if(sub[0]=='\"')
                    sub = sub.substr(1); //Remove first quotes
                if(sub[sub.size()-1]=='\"')
                    sub = sub.substr(0,sub.length()-1);//Remove last quotes

                if((int)i==colRefDes)
                    component.designator=sub;
                else if((int)i==colLayer)
                    component.layer = sub;
                else if((int)i==colX)
                {
                    double coordX;
                    try
                    {
                        coordX = stod(sub);
                    }
                    catch(exception &e)
                    {
                        cerr <<"ERROR: Column Center-X contains non-numerical value" <<endl;
                        return 6;
                    }
                    component.posPickAndPlace.x = coordX;
                }
                else if((int)i==colY)
                {
                    double coordY;
                    try
                    {
                        coordY = stod(sub);
                    }
                    catch(exception &e)
                    {
                        cerr <<"ERROR: Column Center-Y contains non-numerical value" <<endl;
                        return 6;
                    }
                    component.posPickAndPlace.y = coordY;
                }
                else if((int)i==colPolarized)
                    component.polarized = (Utilities::toLower(sub)=="true");
            }
            m_components.push_back(component);
        }
        else //Header
        {
            if(lineVector.size()>=1 && lineVector[0].substr(0,5) == "Units")
            {
                int cut=lineVector[0].find(':');
                string sub=lineVector[0].substr(cut+2);
                m_units = sub;
            }
        }

        if(lineVector.size()==0 || lineVector[0]=="")
            numberOfBlankLines++;
    }

    return 0;
}

bool PickAndPlace::loaded()
{
    return (m_components.size() != 0);
}

size_t PickAndPlace::count()
{
    return m_components.size();
}

vector<Component> PickAndPlace::getAllComponents()
{
    return m_components;
}

Component PickAndPlace::getComponent(string designator)
{
    for(size_t i=0;i<m_components.size();i++)
    {
        if(designator == m_components[i].designator)
            return m_components[i];
    }

    Component component;
    return component;
}

vector<string> PickAndPlace::getDesignatorList()
{
    vector<string> designators;
    for(size_t i=0;i<m_components.size();i++)
    {
        designators.push_back(m_components[i].designator);
    }

    return designators;
}

string PickAndPlace::getUnits()
{
    return m_units;
}

string PickAndPlace::toString(CompState state)
{
    switch(state)
    {
        case COMP_FITTED:
            return "Fitted";

        case COMP_NOT_FITTED:
            return "Not Fitted";

        case COMP_ERROR:
            return "Error";

        case COMP_UNDEFINED:
        default:
            return "Undefined";
    }
}

void PickAndPlace::printComponents()
{
    cout <<"Components:" <<endl;
    for(size_t i=0;i<m_components.size();i++)
    {
        cout <<m_components[i].designator <<" on " <<m_components[i].layer <<" at (" <<m_components[i].posPickAndPlace.x <<", " <<m_components[i].posPickAndPlace.y <<")" <<endl;
    }
    cout <<endl;
}

PickAndPlace::~PickAndPlace()
{

}
