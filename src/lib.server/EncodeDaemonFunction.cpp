#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cstdio>

#include "EncodeDaemonFunction.h"


void EncdConnectionHandler::FileTransferFunction(TCPStream *stream, FILEINFO fileinfo)
{
    string message;

    int functionResult = STATUS_OK;
    int functionStep = 0;

    // data test
    functionStep = 1;
    // FILEINFO fileinfo;
    FileController *fileController;
    //파일 저장용 파일 컨트롤 클래스 변수 생성
    // fileinfo = (FILEINFO)fileinfo_from;
    // STEP1 : 테스트 파일 정보 생성
    //실 사용 시 여기서 만드는 대신에 솔루션 서버로부터 받아온 정보를 토대로 실행해야함

    fileController = new FileController(fileinfo.cEncdFolderPath, fileinfo.cEncdFileName);

    // fileinfo.dFileDownSize = fileController->getFileSizeExist();
    // 1-1 이어받기 가능하도록

    fileController->appendFileSizeSet(0);
    fileinfo.dFileDownSize = 0;
    // 1-2 무조건 새로받기

    functionStep = 2;
    functionResult = processGetFile_START(stream, fileinfo);
    // STEP2 : 파일 정보 컨텐츠 서버로 전송
    // return 1 == 전송 성공
    // return -1 == 전송 실패

    functionStep = 6;
    functionResult = processGetFile_Get_Exist_Data(stream);
    // STEP6 : 컨텐츠 서버로부터 실제 파일이 해당 서버에 있는지 데이터를 받아온다
    // return 1 == 파일 있음
    // return 99 == 파일 없음
    // return -1 == 통신 에러

    if (functionResult == 99)
    {
        //파일 없음. 종료처리
        return;
    }
    else if (functionResult == -1)
    {
        //통신 에러. 종료처리
        return;
    }
    else
    {
        functionStep = 7;
        //파일 있음. 다음 스텝 진행
    }

    functionResult = processGetFile_TRANSFER_READY(stream);
    // STEP7 : 컨텐츠 서버에게 파일 전송 준비가 완료되었다고 신호를 보냄

    functionStep = 10;
    functionResult = processGetFile_FILE_TRANSFER(stream, fileController, fileinfo.dFileSize);
    // STEP10 : 컨텐츠 서버에게 실제 파일 받아옴

    functionStep = 12;
    functionResult = processGetFile_TRANSFER_END(stream, fileController, fileinfo.dFileSize);
    // STEP12 : 파일 받아오기 완료

    printf("\n\nALL PROCESS END : %d\n", functionStep);
}

int EncdConnectionHandler::processGetFile_START(TCPStream *stream, FILEINFO fileinfo)
{
    printf("\n\nSTEP 2] processGetFile_START Start\n");
    HEADER header;
    header = SocketFunction::makeHeader(FILE_TRANSFER_CMD_START, 1, 1, sizeof(FILEINFO), 0, "test msg");
    char *send_data = NULL;
    SocketFunction::makeData(send_data, header, fileinfo);

    int len;
    int return_value;
    if (stream)
    {
        len = stream->send(send_data, sizeof(FILEINFO) + HEADER_SIZE);
        if (len > 0)
        {
            printf("processGetFile_START Send OK : %d / %ld\n", len, HEADER_SIZE);
            return_value = STATUS_OK;
        }
        else
        {
            printf("processGetFile_START send data error");
            return_value = STATUS_ERR;
        }
    }
    else
    {
        printf("processGetFile_START stream error");
        return_value = STATUS_ERR;
    }

    printf("STEP 2] processGetFile_START End\n\n");
    return return_value;
}

int EncdConnectionHandler::processGetFile_Get_Exist_Data(TCPStream *stream)
{
    printf("\n\nSTEP 6] processGetFile_Get_Exist_Data Start\n");

    printf("receive data start\n");
    LPHEADER pHeader = new HEADER;
    memset(pHeader, 0x00, HEADER_SIZE);
    printf("header init\n");

    int len;
    int return_value;

    while ((len = stream->receive((char *)pHeader, HEADER_SIZE) > 0))
    {
        printf("received, nCmd: %d\n", pHeader->nCmd);
        printf("received, nStatus: %d\n", pHeader->nStatus);
        printf("received, nDataCnt: %d\n", pHeader->nDataCnt);
        printf("received, nDataSize: %d\n", pHeader->nDataSize);
        printf("received, nErrorCode: %d\n", pHeader->nErrorCode);
        printf("received, cMsg: %s\n", pHeader->cMsg);

        printf("receive data end : %d\n", len);

        if (pHeader->nCmd == FILE_TRANSFER_CMD_EXIST_CHECK)
        {
            printf("FILE_TRANSFER_CMD_EXIST_CHECK : %d\n", pHeader->nStatus);
            if (pHeader->nStatus == -1)
            {
                printf("FILE_TRANSFER_CMD_EXIST_CHECK, FILE NOT EXIST: %d\n", pHeader->nStatus);
                return_value = STATUS_END;
            }
            else
            {
                printf("FILE_TRANSFER_CMD_EXIST_CHECK, FILE EXIST: %d\n", pHeader->nStatus);
                return_value = STATUS_OK;
            }
        }
        else
        {
            printf("FILE_TRANSFER_CMD_EXIST_CHECK, CMD ERROR: %d\n", pHeader->nCmd);
            return_value = STATUS_ERR;
        }

        break;
    }

    printf("STEP 6] processGetFile_Get_Exist_Data End : %d\n\n", return_value);
    return return_value;
}

int EncdConnectionHandler::processGetFile_TRANSFER_READY(TCPStream *stream)
{
    printf("\n\nSTEP 7] processGetFile_TRANSFER_READY Start\n");

    HEADER header;
    header = SocketFunction::makeHeader(FILE_TRANSFER_CMD_TRANSFER_READY, 1);
    char *send_data = NULL;
    SocketFunction::makeData(send_data, header);

    int len = stream->send(send_data, HEADER_SIZE);
    int return_value;

    if (len > 0)
    {
        return_value = STATUS_OK;
    }
    else
    {
        return_value = STATUS_ERR;
    }

    printf("STEP 7] processGetFile_TRANSFER_READY End : %d\n\n", return_value);
    return return_value;
}

int EncdConnectionHandler::processGetFile_FILE_TRANSFER(TCPStream *stream, FileController *fileController, double totalSize)
{
    printf("\n\nSTEP 10] processGetFile_FILE_TRANSFER Start\n");
    int return_value = STATUS_OK;
    int len;

    FILE *file;

    if (fileController->getFileSizeExist())
    {
        printf("FILE Append mode\n");
        file = fopen64(fileController->getFilePath(), "wb+");
    }
    else
    {
        printf("FILE Write mode\n");
        file = fopen64(fileController->getFilePath(), "wb+");
    }

    printf("FILE PATH : %s\n", fileController->getFilePath());

    fseeko64(file, fileController->getFileSizeExist(), SEEK_SET);
    // fseeko64(file, 0, SEEK_SET);

    printf("FILE SEEK TO : %zu\n", fileController->getFileSizeExist());

    double sizeLeft = totalSize - fileController->getFileSizeExist();
    double sizeDoing = 0;

    printf("EXIST SIZE : %f = %f - %zu\n", sizeLeft, totalSize, fileController->getFileSizeExist());

    char *fileBuffer = new char[FILE_BUF_SIZE];
    memset(fileBuffer, 0x00, FILE_BUF_SIZE);

    double transferSize = (double)FILE_BUF_SIZE;
    if (sizeLeft < transferSize)
    {
        transferSize = sizeLeft;
        //남은 용량이 버퍼보다 작다면, 남은 용량 크기까지만 복사하도록
    }

    while (sizeLeft > 0)
    {
        //남은 파일 사이즈가 0이 될때까지 while로 전송

        len = stream->receive((char *)fileBuffer, transferSize);
        if (len <= 0)
        {
            //전송 에러
            printf("FILE TRANSFER GET ERROR : %d\n", len);
            return_value = STATUS_ERR;
            break;
        }
        else if (len < transferSize)
        {
            // FILE_BUF_SIZE보다 적은 데이터가 전달됨
            printf("FILE WRITE GET LESS LENGTH : %d\n", len);
            printf("FILE TRANSFER GET DATA SIZE MISMATCH : %f = %f - %d\n", (transferSize - len), transferSize, len);
            transferSize = (double)len;
            len = fwrite((void *)fileBuffer, 1, len, file);

            if (len < 0)
            {
                printf("FILE WRITE GET ERROR : %d\n", len);
                return_value = STATUS_ERR;
                break;
            }

            usleep(10);
        }
        else
        {
            // FILE_BUF_SIZE길이의 데이터가 전달됨
            printf("FILE WRITE GET LENGTH : %d\n", len);
            len = fwrite((void *)fileBuffer, 1, transferSize, file);

            if (len < 0)
            {
                printf("FILE WRITE GET ERROR : %d\n", len);
                return_value = STATUS_ERR;
                break;
            }
        }

        printf("FILE TRANSFER GET LEFT : %f = %f - %f\n", (sizeLeft - transferSize), sizeLeft, transferSize);
        sizeLeft = sizeLeft - transferSize;
        sizeDoing = sizeDoing + transferSize;

        // next transfer init
        memset(fileBuffer, 0x00, FILE_BUF_SIZE);
        transferSize = (double)FILE_BUF_SIZE;

        if (sizeLeft < transferSize)
        {
            transferSize = sizeLeft;
            //남은 용량이 버퍼보다 작다면, 남은 용량 크기까지만 복사하도록
        }
    }

    fclose(file);
    file = NULL;

    printf("STEP 10] processGetFile_FILE_TRANSFER End : %f / %d\n\n", sizeDoing, return_value);

    return return_value;
}

int EncdConnectionHandler::processGetFile_TRANSFER_END(TCPStream *stream, FileController *fileController, double totalSize)
{
    printf("\n\nSTEP 12] processGetFile_TRANSFER_END Start\n");

    printf("receive data start\n");
    LPHEADER pHeader = new HEADER;
    memset(pHeader, 0x00, HEADER_SIZE);
    char *pHeaderPointer = (char *)pHeader;
    printf("header init\n");

    int len;
    int return_value;

    while ((len = stream->receive(pHeaderPointer, HEADER_SIZE) > 0))
    {

        printf("received, nCmd: %d\n", pHeader->nCmd);
        printf("received, nStatus: %d\n", pHeader->nStatus);
        printf("received, nDataCnt: %d\n", pHeader->nDataCnt);
        printf("received, nDataSize: %d\n", pHeader->nDataSize);
        printf("received, nErrorCode: %d\n", pHeader->nErrorCode);
        printf("received, cMsg: %s\n", pHeader->cMsg);

        printf("receive data end : %d\n", len);

        if (pHeader->nCmd == FILE_TRANSFER_CMD_TRANSFER_END)
        {
            printf("FILE_TRANSFER_CMD_TRANSFER_END : %d\n", pHeader->nStatus);
            return_value = STATUS_END;
        }
        else
        {
            printf("FILE_TRANSFER_CMD_TRANSFER_END, CMD ERROR: %d\n", pHeader->nCmd);
            return_value = STATUS_ERR;
        }

        break;
    }

    printf("STEP 12] processGetFile_TRANSFER_END End : %d\n\n", return_value);

    return return_value;
}

//이 밑부터 Server Side
void *EncdConnectionHandler::run()
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

        printf("thread %lu, start\n", (long unsigned int)self());

        char *pRecvHeaderData = NULL;
        pRecvHeaderData = new char[HEADER_SIZE];
        memset(pRecvHeaderData, 0x00, HEADER_SIZE);
        int header_length = HEADER_SIZE;

        LPHEADER pHeader = new HEADER;
        memset(pHeader, 0x00, HEADER_SIZE);

        char *pRecvData = NULL;
        pRecvData = new char[FILEINFO_SIZE];
        memset(pRecvData, 0x00, FILEINFO_SIZE);
        int data_length = FILEINFO_SIZE;

        LPFILEINFO pData = new FILEINFO;
        memset(pData, 0x00, FILEINFO_SIZE);

        int len;

        len = stream->receive(pRecvHeaderData, header_length);

        if (len < 0)
        {
            printf("len error : %d\n", len);
        }
        else
        {
            printf("thread %lu, get header end\n", (long unsigned int)self());
            pHeader = (LPHEADER)pRecvHeaderData;
            printf("thread %lu, get header size from client : %ld\n", (long unsigned int)self(), HEADER_SIZE);
            printf("thread %lu, nCmd: %d\n", (long unsigned int)self(), pHeader->nCmd);
            printf("thread %lu, nStatus: %d\n", (long unsigned int)self(), pHeader->nStatus);
            printf("thread %lu, nDataCnt: %d\n", (long unsigned int)self(), pHeader->nDataCnt);
            printf("thread %lu, nDataSize: %d\n", (long unsigned int)self(), pHeader->nDataSize);
            printf("thread %lu, nErrorCode: %d\n", (long unsigned int)self(), pHeader->nErrorCode);
            printf("thread %lu, cMsg: %s\n", (long unsigned int)self(), pHeader->cMsg);
        }

        len = stream->receive(pRecvData, data_length);
        if (len < 0)
        {
            printf("len error : %d\n", len);
        }
        else
        {
            printf("thread %lu, get data end\n", (long unsigned int)self());
            pData = (LPFILEINFO)pRecvData;
            EncdConnectionHandler::printData(pData);

            TCPConnector *connector = new TCPConnector();
            TCPStream *stream = connector->connect(pData->cServerIP, pData->lServerPort);
            printf("FileTransferFunction start\n");
            FILEINFO fileinfo = SocketFunction::makeFILEINFOfromPointer(pData);
            EncdConnectionHandler::FileTransferFunction(stream, fileinfo);
        }

        printf("thread %lu, end\n", (long unsigned int)self());
        delete item;
    }

    // Should never get here
    return NULL;
}

void EncdConnectionHandler::printData(LPFILEINFO fileinfo)
{
    printf("\n\nSTEP 1] printData Start\n");
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
    printf("STEP 1] printData End\n\n");
}

int EncdConnectionHandler::processCmd(TCPStream *stream, int cmd, int status, int dataCnt, int dataSize, int errorCode, char *msg)
{
    /*
    //필요시 코딩
    switch (cmd)
    {
    case FILE_TRANSFER_CMD_START: // FILE TRANSFER
        // processPutFile(stream, dataCnt, dataSize);
        break;

    default:
        printf("CMD NOT FOUND\n");
        return -1;
        break;
    }
    */
    printf("######### DEBUG 1 #########\n");
    return 0;
}

int EncdConnectionHandler::processGetFile(TCPStream *stream, int dataCnt, int dataSize)
{
    /*
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
        EncdConnectionHandler::printData(fileinfo);
        // STEP3 : 헤더 정보 출력
    }
    */
    printf("######### DEBUG 2 #########\n");
    return 1;
}

void EncdConnectionHandler::FileTransferTEST(TCPStream *stream)
{
    string message;

    int functionResult = STATUS_OK;
    int functionStep = 0;

    // data test
    functionStep = 1;
    FILEINFO fileinfo;
    FileController *fileController;
    //파일 저장용 파일 컨트롤 클래스 변수 생성
    fileinfo = testFileinfoMake();
    // STEP1 : 테스트 파일 정보 생성
    //실 사용 시 여기서 만드는 대신에 솔루션 서버로부터 받아온 정보를 토대로 실행해야함

    fileController = new FileController(fileinfo.cEncdFolderPath, fileinfo.cEncdFileName);

    // fileinfo.dFileDownSize = fileController->getFileSizeExist();
    // 1-1 이어받기 가능하도록

    fileController->appendFileSizeSet(0);
    fileinfo.dFileDownSize = 0;
    // 1-2 무조건 새로받기

    functionStep = 2;
    functionResult = processGetFile_START(stream, fileinfo);
    // STEP2 : 파일 정보 컨텐츠 서버로 전송
    // return 1 == 전송 성공
    // return -1 == 전송 실패

    functionStep = 6;
    functionResult = processGetFile_Get_Exist_Data(stream);
    // STEP6 : 컨텐츠 서버로부터 실제 파일이 해당 서버에 있는지 데이터를 받아온다
    // return 1 == 파일 있음
    // return 99 == 파일 없음
    // return -1 == 통신 에러

    if (functionResult == 99)
    {
        //파일 없음. 종료처리
        return;
    }
    else if (functionResult == -1)
    {
        //통신 에러. 종료처리
        return;
    }
    else
    {
        functionStep = 7;
        //파일 있음. 다음 스텝 진행
    }


    functionResult = processGetFile_TRANSFER_READY(stream);
    // STEP7 : 컨텐츠 서버에게 파일 전송 준비가 완료되었다고 신호를 보냄


    functionStep = 10;
    functionResult = processGetFile_FILE_TRANSFER(stream, fileController, fileinfo.dFileSize);
    // STEP10 : 컨텐츠 서버에게 실제 파일 받아옴

    functionStep = 12;
    functionResult = processGetFile_TRANSFER_END(stream, fileController, fileinfo.dFileSize);
    // STEP12 : 파일 받아오기 완료

    printf("\n\nALL PROCESS END : %d\n", functionStep);

    // stream->send(send_data, sizeof(FILEINFO) + HEADER_SIZE);
    // printf("send data2\n");
}

FILEINFO EncdConnectionHandler::testFileinfoMake()
{
    printf("\n\nSTEP 1] testFileinfoMake Start\n");

    FILEINFO fileinfo;
    memset(&fileinfo, 0x00, sizeof(FILEINFO));

    /*
        아래 데이터는 테스트 원하는 값으로 바꿔서 테스트
    */

    fileinfo.nType = 0;
    fileinfo.nContId = 12341234;
    fileinfo.nContSeq = 12341234;
    sprintf(fileinfo.cServerIP, "127.0.0.1");
    fileinfo.lServerPort = 9999;
    sprintf(fileinfo.cFolderName, "/home/core/test");
    sprintf(fileinfo.cFileName, "bigfile.iso");
    fileinfo.dFileSize = 10225799168;
    fileinfo.dFileDownSize = 0;
    sprintf(fileinfo.cEncdFolderPath, "/home/core/test/output");
    sprintf(fileinfo.cEncdFileName, "bigfile_output.iso");
    sprintf(fileinfo.cTempData, "temp1");
    sprintf(fileinfo.cTempData2, "temp2");
    sprintf(fileinfo.cTempData3, "temp3");

    printf("STEP 1] testFileinfoMake End\n\n");
    return fileinfo;
}
