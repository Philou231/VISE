#include "file/File.h"


File::File(FileType fileType)
{
    m_fileType = fileType;
}

std::string File::getPath()
{
    return m_path;
}

FileType File::getType()
{
    return m_fileType;
}

File::~File()
{

}
