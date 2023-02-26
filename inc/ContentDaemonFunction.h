#ifndef __CONTENT_DAEMON_FUNCTION_h__
#define __CONTENT_DAEMON_FUNCTION_h__

#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <cstdio>
#include "FDstructDefine.h"
#include "FileControl.h"
#include "FunctionNameDefine.h"
#include "SocketFunction.h"

using namespace std;

void serverStart(int workers, int port, string ip);

class ConnectionHandler : public Thread
{
    wqueue<WorkItem *> &m_queue;

public:
    ConnectionHandler(wqueue<WorkItem *> &queue) : m_queue(queue){};
    void *run();

private:
    void printHeader(LPFILEINFO fileinfo);
    int processCmd(TCPStream *stream, int cmd, int status, int dataCnt, int dataSize, int errorCode, char *msg);
    int processPutFile(TCPStream *stream, int dataCnt, int dataSize);
    int processPutFile_FileExistCheck(FileController *fileController);
    int processPutFile_FileExistReturn(TCPStream *stream, int fileExist);
    int processPutFile_TransferReady(TCPStream *stream, FileController *fileController);
    int processPutFile_File_Transfer(TCPStream *stream, FileController *fileController, double totalSize, double startSize);
    int processPutFile_TransferEnd(TCPStream *stream, FileController *fileController, double totalSize);
};
#endif
