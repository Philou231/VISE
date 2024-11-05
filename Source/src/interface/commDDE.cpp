//Helpful links:
//https://learn.microsoft.com/en-us/windows/win32/dataxchg/basic-concepts
//https://mskb.pkisolutions.com/kb/279721

#include "CommDDE.h"

#include <ddeml.h>

using namespace std;



std::string handleStringToString(HSZ handleString);
string handleDataToString(HDDEDATA hData, bool truncate=false);



DWORD CommDDE::m_instanceID=0;
std::function<int (string,string)>* CommDDE::m_callbackFunction=NULL;




CommDDE::CommDDE()
{
    m_instanceID=0;
    m_callbackFunction=NULL;

    DdeInitialize(&m_instanceID, (PFNCALLBACK) &CommDDE::callbackDDE, 0, 0);


    //Create DDE service named "VISE"
    HSZ m_hServiceName = DdeCreateStringHandle(m_instanceID, "VISE", CP_WINANSI);
    DdeNameService(m_instanceID, m_hServiceName, 0L, DNS_REGISTER);
    DdeFreeStringHandle(m_instanceID, m_hServiceName);
}


void CommDDE::setCallback(std::function<int (string,string)>* callbackFunction)
{
    m_callbackFunction = callbackFunction;
}

HDDEDATA CALLBACK CommDDE::callbackDDE(UINT uType, UINT uFmt, HCONV hconv, HSZ hString1, HSZ hString2, HDDEDATA hData, DWORD dwData1, DWORD dwData2)
{
    //cout <<"DDE Callback (" <<uType <<"): ";
    switch (uType)
    {
        case XTYP_CONNECT:
            //cout <<"Connection" <<endl;
            if(handleStringToString(hString1)=="Find")
                return (HDDEDATA) TRUE;
            else
                return (HDDEDATA) FALSE;

        case XTYP_CONNECT_CONFIRM:
            //cout <<"Connection confirmed" <<endl;
            return (HDDEDATA) NULL;

        case XTYP_REGISTER:
            //cout <<"Registered" <<endl;
            return (HDDEDATA) NULL;

        case XTYP_UNREGISTER:
            //cout <<"Unregistered" <<endl;
            return (HDDEDATA) NULL;

        case XTYP_EXECUTE:
            //cout <<"Execute " <<handleStringToString(hString1) <<endl;
            //cout <<"Data: " <<handleDataToString(hData) <<endl;
            if(m_callbackFunction!=NULL)
            {
                int succeeded = (*m_callbackFunction)(handleStringToString(hString1),handleDataToString(hData));
                if(succeeded==1)
                    return (HDDEDATA) TRUE;
                else
                    return (HDDEDATA) FALSE;
            }
            return (HDDEDATA) FALSE;

        case XTYP_POKE:
            //cout <<"Poke" <<endl;
            return (HDDEDATA) DDE_FACK;

        case XTYP_ADVDATA:
            //cout <<"Advertise Data" <<endl;
            //cout <<"Cell " <<handleStringToString(hString2) <<" of " <<handleStringToString(hString1) <<" now contains: \"" <<handleDataToString(hData) <<"\"" <<endl;
            return (HDDEDATA) DDE_FACK;

        case XTYP_XACT_COMPLETE:
            //cout <<"Asynchronous transaction complete" <<endl;
            return (HDDEDATA) NULL;

        case XTYP_DISCONNECT:
            //cout <<"Disconnected!" <<endl;
            return (HDDEDATA) NULL;

        default:
            //cout <<"Unknown" <<endl;
            return (HDDEDATA) NULL;
    }
}


CommDDE::~CommDDE()
{
    DdeUninitialize(m_instanceID);
    m_instanceID=0;

    m_callbackFunction=NULL;
}




std::string handleStringToString(HSZ handleString)
{
    DWORD dataSize = 1 + DdeQueryStringA(CommDDE::m_instanceID, handleString, NULL, 0, CP_WINANSI);
    char resultArray[dataSize];

    DdeQueryStringA(CommDDE::m_instanceID, handleString, resultArray, dataSize, CP_WINANSI);

    return (string)resultArray;
}

string handleDataToString(HDDEDATA hData, bool truncate)
{
    DWORD dataSize = DdeGetData(hData, NULL, 0, 0);
    if(truncate)
        dataSize = (dataSize-2<255?dataSize-2:255);
    else
        dataSize = (dataSize<255?dataSize:255);
    char resultArray[dataSize];

    DdeGetData(hData, (unsigned char *)resultArray, dataSize, 0);
    if(truncate)
        resultArray[dataSize-1]='\0';//The response is truncated because it finishes with \r\n\0. \r gets replaced by \0

    return (string)resultArray;
}


/* For DDE client
void DDEExecute(DWORD idInst, HCONV hConv, char* szCommand)
{
    HDDEDATA hData = DdeCreateDataHandle(idInst, (LPBYTE)szCommand,
                               lstrlen(szCommand)+1, 0, NULL, CF_TEXT, 0);
    if (hData==NULL)   {
        printf("Command failed: %s\n", szCommand);
    }
    else    {
        DdeClientTransaction((LPBYTE)hData, 0xFFFFFFFF, hConv, 0L, 0,
                             XTYP_EXECUTE, TIMEOUT_ASYNC, NULL);
    }
}

void DDEPoke(DWORD idInst, HCONV hConv, char* szItem, char* szData)
{
    HSZ hszItem = DdeCreateStringHandle(idInst, szItem, 0);
	DdeClientTransaction((LPBYTE)szData, (DWORD)(lstrlen(szData)+1),
                          hConv, hszItem, CF_TEXT,
                          XTYP_POKE, 3000, NULL);
    DdeFreeStringHandle(idInst, hszItem);
}




m_hServiceName = DdeCreateStringHandle(m_instanceID, "Excel", CP_UTF8);
m_hConversation = DdeConnect(m_instanceID, m_hServiceName, NULL, (PCONVCONTEXT) NULL);
DdeFreeStringHandle(m_instanceID, m_hServiceName);

if(m_hConversation == NULL)
    cout <<"DDE Server unavailable..." <<endl;
else
{
    cout <<"Connected to Excel's DDE Server!" <<endl;


    //Write Data
    char testString[] = "Test!";
    HSZ hItemName1 = DdeCreateStringHandle(m_instanceID, "R2C2", 0);

    DdeClientTransaction((LPBYTE)testString, (DWORD)(lstrlen(testString)+1), m_hConversation, hItemName1, CF_TEXT, XTYP_POKE, 1000, NULL);
    DdeFreeStringHandle(m_instanceID, hItemName1);


    //Read Data
    HSZ hItemName2 = DdeCreateStringHandle(m_instanceID, "R3C2", 0);
    HDDEDATA hDataFromExcel = DdeClientTransaction(NULL, 0, m_hConversation, hItemName2, CF_TEXT, XTYP_REQUEST, 1000, NULL);
    if(hDataFromExcel==NULL)
        cout <<"Request failed..." <<endl;
    else
        cout <<"Data in cell B3: \"" <<handleDataToString(hDataFromExcel) <<"\"" <<endl;
    DdeFreeStringHandle(m_instanceID, hItemName2);


    //Subscribe to updates
    HSZ hItemName3 = DdeCreateStringHandle(m_instanceID, "R4C2", 0);

    DdeClientTransaction(NULL, 0, m_hConversation, hItemName1, CF_TEXT, XTYP_ADVSTART, 1000, NULL);
    DdeFreeStringHandle(m_instanceID, hItemName3);
}
*/
