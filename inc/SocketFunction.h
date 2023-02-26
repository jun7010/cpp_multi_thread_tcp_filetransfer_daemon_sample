#ifndef __SOCKET_FUNCTION_h__
#define __SOCKET_FUNCTION_h__

#include "FDstructDefine.h"
#include "thread.h"
#include "wqueue.h"
#include "tcpacceptor.h"
#include "tcpconnector.h"

class SocketFunction
{

public:
    SocketFunction();
    static HEADER makeHeader(int nCmd, int nStatus);
    static HEADER makeHeader(int nCmd, int nStatus, int nDataCnt, int nDataSize, int nErrorCode, const char *cMsg);
    static void makeData(char *&send_data, HEADER header);
    static void makeData(char *&send_data, HEADER header, int nDataCnt, char *dataList);
    static void makeData(char *&send_data, HEADER header, FILEINFO fileinfo);
    static FILEINFO makeFILEINFOfromPointer(LPFILEINFO pFileinfo);
};

class WorkItem
{
    TCPStream *m_stream;

public:
    WorkItem(TCPStream *stream) : m_stream(stream) {}
    ~WorkItem() { delete m_stream; }

    TCPStream *getStream() { return m_stream; }
};

#define FILE_BUF_SIZE 1024 * 32
//파일 전송 버퍼 크기

#endif