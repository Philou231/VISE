#ifndef COMMDDE_HEADER
#define COMMDDE_HEADER

#include <functional>
#include <string>
#include <windows.h>



class CommDDE{

    public:
        CommDDE();

        void setCallback(std::function<int (std::string,std::string)>* callbackFunction);
        static HDDEDATA CALLBACK callbackDDE(UINT uType, UINT uFmt, HCONV hconv, HSZ hString1, HSZ hString2, HDDEDATA hData, DWORD dwData1, DWORD dwData2);

        ~CommDDE();

    public:
        static DWORD m_instanceID;
        static std::function<int (std::string,std::string)>* m_callbackFunction;
};



#endif // COMMDDE_HEADER
