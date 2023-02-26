#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include "SocketFunction.h"

using namespace std;

SocketFunction::SocketFunction()
{
}

HEADER SocketFunction::makeHeader(int nCmd, int nStatus)
{
    printf("makeHeader Start\n");
    HEADER header;
    memset(&header, 0x00, sizeof(HEADER));
    header.nCmd = nCmd;
    header.nStatus = nStatus;
    header.nDataCnt = 0;
    header.nDataSize = 0;
    header.nErrorCode = 0;
    sprintf(header.cMsg, "null");
    printf("makeHeader Return\n");
    return header;
}

HEADER SocketFunction::makeHeader(int nCmd, int nStatus, int nDataCnt, int nDataSize, int nErrorCode, const char *cMsg)
{
    printf("makeHeader2 Start\n");
    HEADER header;
    memset(&header, 0x00, sizeof(HEADER));
    header.nCmd = nCmd;
    header.nStatus = nStatus;
    header.nDataCnt = nDataCnt;
    header.nDataSize = nDataSize;
    header.nErrorCode = nErrorCode;
    sprintf(header.cMsg, cMsg);
    printf("makeHeader2 Return\n");
    return header;
}

void SocketFunction::makeData(char *&send_data, HEADER header)
{
    printf("makeData Start\n");
    send_data = new char[HEADER_SIZE];
    memset(send_data, 0x00, (HEADER_SIZE));
    memcpy(send_data, &header, HEADER_SIZE);
    printf("makeData Return\n");
}

void SocketFunction::makeData(char *&send_data, HEADER header, int nDataCnt, char *dataList)
{
    printf("makeData2 Start\n");
    send_data = new char[HEADER_SIZE + (sizeof(FILEINFO) * nDataCnt)];
    memset(send_data, 0x00, (HEADER_SIZE + (sizeof(FILEINFO) * nDataCnt)));
    memcpy(send_data, &header, HEADER_SIZE);
    memcpy(send_data + HEADER_SIZE, dataList, (sizeof(FILEINFO) * nDataCnt));
    printf("makeData2 Return\n");
}

void SocketFunction::makeData(char *&send_data, HEADER header, FILEINFO fileinfo)
{

    printf("makeData3 Start\n");
    send_data = new char[HEADER_SIZE + sizeof(FILEINFO)];
    memset(send_data, 0x00, (HEADER_SIZE + sizeof(FILEINFO)));
    memcpy(send_data, &header, HEADER_SIZE);
    memcpy(send_data + HEADER_SIZE, &fileinfo, sizeof(FILEINFO));
    printf("makeData3 Return\n");
}

FILEINFO SocketFunction::makeFILEINFOfromPointer(LPFILEINFO pFileinfo)
{

    FILEINFO fileinfo;
    memset(&fileinfo, 0x00, sizeof(FILEINFO));

    fileinfo.nType = pFileinfo->nType;
    fileinfo.nContId = pFileinfo->nContId;
    fileinfo.nContSeq = pFileinfo->nContSeq;
    sprintf(fileinfo.cServerIP, pFileinfo->cServerIP);
    fileinfo.lServerPort = pFileinfo->lServerPort;
    sprintf(fileinfo.cFolderName, pFileinfo->cFolderName);
    sprintf(fileinfo.cFileName, pFileinfo->cFileName);
    fileinfo.dFileSize = pFileinfo->dFileSize;
    fileinfo.dFileDownSize = pFileinfo->dFileDownSize;
    sprintf(fileinfo.cEncdFolderPath, pFileinfo->cEncdFolderPath);
    sprintf(fileinfo.cEncdFileName, pFileinfo->cEncdFileName);
    sprintf(fileinfo.cTempData, pFileinfo->cTempData);
    sprintf(fileinfo.cTempData2, pFileinfo->cTempData2);
    sprintf(fileinfo.cTempData3, pFileinfo->cTempData3);

    return fileinfo;
}