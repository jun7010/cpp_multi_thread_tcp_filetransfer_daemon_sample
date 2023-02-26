#ifndef __ENCODE_DAEMON_FUNCTION_h__
#define __ENCODE_DAEMON_FUNCTION_h__

#include <string>
#include <stdio.h>
#include <cstdio>
#include "tcpconnector.h"
#include "FDstructDefine.h"
#include "FileControl.h"
#include "FunctionNameDefine.h"
#include "SocketFunction.h"

using namespace std;

void serverStart(int workers, int port, string ip);

class EncdConnectionHandler : public Thread
{
    wqueue<WorkItem *> &m_queue;

public:
    EncdConnectionHandler(wqueue<WorkItem *> &queue) : m_queue(queue){};
    void *run();
    static void FileTransferFunction(TCPStream *stream);
    static void FileTransferFunction(TCPStream *stream, FILEINFO fileinfo);
    static FILEINFO testFileinfoMake();
    static void FileTransferTEST(TCPStream *stream);

private:
    void printData(LPFILEINFO fileinfo);
    int processCmd(TCPStream *stream, int cmd, int status, int dataCnt, int dataSize, int errorCode, char *msg);
    int processGetFile(TCPStream *stream, int dataCnt, int dataSize);
    static int processGetFile_START(TCPStream *stream, FILEINFO fileinfo);
    static int processGetFile_Get_Exist_Data(TCPStream *stream);
    static int processGetFile_TRANSFER_READY(TCPStream *stream);
    static int processGetFile_FILE_TRANSFER(TCPStream *stream, FileController *fileController, double totalSize);
    static int processGetFile_TRANSFER_END(TCPStream *stream, FileController *fileController, double totalSize);
};

#endif
