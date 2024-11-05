#ifndef FILE_HEADER
#define FILE_HEADER

#include <string>

enum FileType{FILE_IMAGE,FILE_PICKANDPLACE};

class File
{
    public:
        File(FileType fileType);

        virtual int createFile(std::string path)=0;
        virtual bool loaded()=0;

        std::string getPath();
        FileType getType();

        virtual ~File()=0;

    protected:
        std::string m_path;
        FileType m_fileType;
};

#endif //#ifndef FILE_HEADER

