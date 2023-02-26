#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cstdio>
#include <unistd.h>

#include "ContentDaemonFunction.h"

void *ConnectionHandler::run()
{
    // Remove 1 item at a time and process it. Blocks if no items are
    // available to process.
    for (int i = 0;; i++)
    {
        // printf("thread %lu, loop %d - waiting for item...\n", (long unsigned int)self(), i);
        WorkItem *item = m_queue.remove();
        // printf("thread %lu, loop %d - got one item\n", (long unsigned int)self(), i);
        TCPStream *stream = item->getStream();

        // Echo messages back the client until the connection is
        // closed
        LPHEADER pHeader = new HEADER;
        memset(pHeader, 0x00, HEADER_SIZE);

        int len;
        while ((len = stream->receive((char *)pHeader, HEADER_SIZE) > 0))
        {
            printf("thread %lu, get header size from client : %ld\n", (long unsigned int)self(), HEADER_SIZE);
            printf("thread %lu, nCmd: %d\n", (long unsigned int)self(), pHeader->nCmd);
            printf("thread %lu, nStatus: %d\n", (long unsigned int)self(), pHeader->nStatus);
            printf("thread %lu, nDataCnt: %d\n", (long unsigned int)self(), pHeader->nDataCnt);
            printf("thread %lu, nDataSize: %d\n", (long unsigned int)self(), pHeader->nDataSize);
            printf("thread %lu, nErrorCode: %d\n", (long unsigned int)self(), pHeader->nErrorCode);
            printf("thread %lu, cMsg: %s\n", (long unsigned int)self(), pHeader->cMsg);

            if (processCmd(stream, pHeader->nCmd, pHeader->nStatus, pHeader->nDataCnt, pHeader->nDataSize, pHeader->nErrorCode, pHeader->cMsg) == 0)
            {
                printf("thread %lu, process end\n", (long unsigned int)self());
                break;
            }
            else
            {
                printf("thread %lu, process error\n", (long unsigned int)self());
                break;
            }

            printf("thread %lu, one cycle end\n", (long unsigned int)self());
        }
        printf("thread %lu, end\n", (long unsigned int)self());
        delete item;
    }

    // Should never get here
    return NULL;
}

void ConnectionHandler::printHeader(LPFILEINFO fileinfo)
{
    printf("\n\nSTEP 3] printHeader Start\n");
    printf("nType : %d\n", fileinfo->nType);
    printf("nContId : %ld\n", fileinfo->nContId);
    printf("nContSeq : %ld\n", fileinfo->nContSeq);
    printf("cServerIP : %s\n", fileinfo->cServerIP);
    printf("lServerPort : %ld\n", fileinfo->lServerPort);
    printf("cFolderName : %s\n", fileinfo->cFolderName);
    printf("cFileName : %s\n", fileinfo->cFileName);
    printf("dFileSize : %f\n", fileinfo->dFileSize);
    printf("dFileDownSize : %f\n", fileinfo->dFileDownSize);
    printf("cEncdFolderPath : %s\n", fileinfo->cEncdFolderPath);
    printf("cEncdFileName : %s\n", fileinfo->cEncdFileName);
    printf("cTempData : %s\n", fileinfo->cTempData);
    printf("cTempData2 : %s\n", fileinfo->cTempData2);
    printf("cTempData3 : %s\n", fileinfo->cTempData3);
    printf("STEP 3] printHeader End\n\n");
}

int ConnectionHandler::processCmd(TCPStream *stream, int cmd, int status, int dataCnt, int dataSize, int errorCode, char *msg)
{
    switch (cmd)
    {
    case FILE_TRANSFER_CMD_START: // FILE TRANSFER
        processPutFile(stream, dataCnt, dataSize);
        break;

    default:
        printf("CMD NOT FOUND\n");
        return -1;
        break;
    }

    return 0;
}

int ConnectionHandler::processPutFile(TCPStream *stream, int dataCnt, int dataSize)
{

    int functionResult = STATUS_OK;
    int functionStep = 0;

    long dataLength = dataCnt * dataSize;
    char *pRecvData = NULL;
    pRecvData = new char[dataLength];
    memset(pRecvData, 0x00, dataLength);

    int len;
    while ((len = stream->receive(pRecvData, dataLength) > 0))
    {

        LPFILEINFO fileinfo = (LPFILEINFO)pRecvData;

        functionStep = 3;
        ConnectionHandler::printHeader(fileinfo);
        // STEP3 : 헤더 정보 출력
        //실 사용 시 주석처리 해야 할 필요성?

        functionStep = 4;
        FileController *fileController = new FileController(fileinfo->cFolderName, fileinfo->cFileName);
        functionResult = ConnectionHandler::processPutFile_FileExistCheck(fileController);
        // STEP4 : 실제 파일이 있는지 검수
        // return 1 : 파일 존재함
        // return -1 : 파일 없음

        functionStep = 5;
        functionResult = ConnectionHandler::processPutFile_FileExistReturn(stream, functionResult);
        // STEP5 : 실제 파일이 있는지 / 없는지에 대한 결과값을 인코딩 서버에 리턴
        // return 1 : 다음 스탭으로 진행
        // return -1 : 전송 오류 발생. 스탭 중지

        functionStep = 8;
        functionResult = ConnectionHandler::processPutFile_TransferReady(stream, fileController);
        // STEP8 : 파일을 받을 준비가 되었다고 인코딩 서버로부터 전달받음
        // return 1 : 다음 스탭으로 진행
        // return -1 : 오류 발생. 스탭 중지
        // return 99 : 스탭 중지

        functionStep = 9;
        functionResult = ConnectionHandler::processPutFile_File_Transfer(stream, fileController, fileinfo->dFileSize, fileinfo->dFileDownSize);
        // STEP9 : 인코딩 서버로 파일 전송 시작

        functionStep = 11;
        functionResult = ConnectionHandler::processPutFile_TransferEnd(stream, fileController, fileinfo->dFileSize);
        // STEP11 : 파일 전송 완료

        break;
    }

    printf("\n\nALL PROCESS END : %d\n", functionStep);

    return 1;
}

int ConnectionHandler::processPutFile_FileExistCheck(FileController *fileController)
{
    printf("\n\nSTEP 4] processPutFile_FileExistCheck Start\n");

    int fileExist;
    if (fileController->fileExist())
    {
        fileExist = STATUS_OK;
    }
    else
    {
        fileExist = STATUS_ERR;
    }

    printf("STEP 4] processPutFile_FileExistCheck End : %d\n\n", fileExist);
    return fileExist;
}

int ConnectionHandler::processPutFile_FileExistReturn(TCPStream *stream, int fileExist)
{
    printf("\n\nSTEP 5] processPutFile_FileExistReturn Start\n");
    HEADER header;
    header = SocketFunction::makeHeader(FILE_TRANSFER_CMD_EXIST_CHECK, fileExist);

    char *send_data = NULL;
    SocketFunction::makeData(send_data, header);
    printf("makeData End\n");

    printf("send FILE EXIST CHECK start\n");
    int len;
    len = stream->send(send_data, HEADER_SIZE);
    printf("send FILE EXIST CHECK end\n");

    printf("STEP 5] processPutFile_FileExistReturn End : %d\n\n", len);
    if (len > 0)
    {
        return STATUS_OK;
    }
    else
    {
        return STATUS_ERR;
    }
}

int ConnectionHandler::processPutFile_TransferReady(TCPStream *stream, FileController *fileController)
{
    printf("\n\nSTEP 8] processPutFile_TransferReady Start\n");

    LPHEADER pHeader = new HEADER;
    memset(pHeader, 0x00, HEADER_SIZE);
    int len;
    int return_value = STATUS_ERR;

    while ((len = stream->receive((char *)pHeader, HEADER_SIZE, 60) > 0))
    {

        printf("received, nCmd: %d\n", pHeader->nCmd);
        printf("received, nStatus: %d\n", pHeader->nStatus);
        printf("received, nDataCnt: %d\n", pHeader->nDataCnt);
        printf("received, nDataSize: %d\n", pHeader->nDataSize);
        printf("received, nErrorCode: %d\n", pHeader->nErrorCode);
        printf("received, cMsg: %s\n", pHeader->cMsg);

        if (pHeader->nCmd == FILE_TRANSFER_CMD_TRANSFER_READY)
        {
            printf("FILE_TRANSFER_CMD_TRANSFER_READY : %d\n", pHeader->nStatus);
            return_value = pHeader->nStatus;
            break;
        }
        else
        {
            printf("FILE_TRANSFER_CMD_TRANSFER_READY CMD ERROR : %d\n", pHeader->nCmd);
            return_value = STATUS_ERR;
            break;
        }
    }

    printf("STEP 8] processPutFile_TransferReady End : %d\n\n", return_value);
    return return_value;
}

int ConnectionHandler::processPutFile_File_Transfer(TCPStream *stream, FileController *fileController, double totalSize, double startSize)
{
    printf("\n\nSTEP 9] processPutFile_File_Transfer Start\n");
    int return_value = STATUS_OK;
    int len;

    FILE *file;

    printf("FILE DATA : %s\n", fileController->getFilePath());
    printf("FILE Total Size : %f\n", totalSize);
    printf("FILE Start Size : %f\n", startSize);
    file = fopen64(fileController->getFilePath(), "rb");
    fseeko64(file, startSize, SEEK_SET);

    double sizeLeft = totalSize - startSize;
    double sizeDoing = 0;
    printf("TRANSFER SIZE : %f - %f = %f\n", totalSize, startSize, sizeLeft);

    char *fileBuffer = new char[FILE_BUF_SIZE];
    memset(fileBuffer, 0x00, FILE_BUF_SIZE);
    double transferSize = (double)FILE_BUF_SIZE;
    if (sizeLeft < transferSize)
    {
        transferSize = sizeLeft;
        //남은 용량이 버퍼보다 작다면, 남은 용량 크기까지만 복사하도록
    }
    sleep(3);
    //시작 전 10초 대기

    while (sizeLeft > 0)
    {
        //남은 파일 사이즈가 0이 될때까지 while로 전송
        // printf("FILE TRANSFER SIZE LEFT : %f\n", sizeLeft);
        fread((void *)fileBuffer, 1, transferSize, file);
        printf("FILE TRANSFER READING\n");

        len = stream->send(fileBuffer, transferSize);

        if (len < 0)
        {
            printf("FILE TRANSFER PUT ERROR : %d\n", len);
            break;
            return_value = STATUS_ERR;
        }
        else
        {
            printf("FILE TRANSFER PUT DOING : %d\n", len);
        }

        printf("FILE TRANSFER PUT LEFT : %f = %f - %f\n", (sizeLeft - transferSize), sizeLeft, transferSize);
        sizeLeft = sizeLeft - transferSize;
        sizeDoing = sizeDoing + transferSize;
        memset(fileBuffer, 0x00, FILE_BUF_SIZE);
        transferSize = (double)FILE_BUF_SIZE;

        if (sizeLeft < transferSize)
        {
            transferSize = sizeLeft;
            //남은 용량이 버퍼보다 작다면, 남은 용량 크기까지만 복사하도록
        }
        usleep(10);
        //한 사이클 대기
    }

    fclose(file);
    file = NULL;

    printf("STEP 9] processPutFile_File_Transfer End : %f / %d\n\n", sizeDoing, return_value);
    return return_value;
}

int ConnectionHandler::processPutFile_TransferEnd(TCPStream *stream, FileController *fileController, double totalSize)
{
    printf("\n\nSTEP 11] processPutFile_TransferEnd Start\n");
    HEADER header;
    header = SocketFunction::makeHeader(FILE_TRANSFER_CMD_TRANSFER_END, 1);

    char *send_data = NULL;
    SocketFunction::makeData(send_data, header);
    printf("makeData End : %d / %d\n", header.nCmd, header.nStatus);
    printf("send FILE EXIST CHECK start\n");
    int len;
    len = stream->send(send_data, HEADER_SIZE);
    printf("send FILE EXIST CHECK end\n");

    printf("STEP 11] processPutFile_TransferEnd End : %d\n\n", len);
    if (len > 0)
    {
        return STATUS_OK;
    }
    else
    {
        return STATUS_ERR;
    }
}
