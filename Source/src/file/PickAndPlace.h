#ifndef PICKANDPLACE_HEADER
#define PICKANDPLACE_HEADER

#include <vector>
#include "misc/Utilities.h"
#include "file/File.h"
#include "inspection/Item.h"



class PickAndPlace: public File
{
    public:
        PickAndPlace();

        virtual int createFile(std::string path);
        virtual bool loaded();
        size_t count();

        std::vector<Component> getAllComponents();
        Component getComponent(std::string designator);
        std::vector<std::string> getDesignatorList();

        std::string getUnits();

        static std::string toString(CompState state);
        void printComponents();

        ~PickAndPlace();

    private:
        std::string m_units;
        std::vector<Component> m_components;
};

#endif //#ifndef PICKANDPLACE_HEADER
