#ifndef __FDSTRUCT_DEFINE_h__
#define __FDSTRUCT_DEFINE_h__

// TCP 통신에 사용될 구조체 정의 헤더파일

typedef struct _HEADER
{
    int nCmd;       //실행될 커맨드 명
    int nStatus;    //해당 커맨드에서 실행될 process 함수 내부에서 처리될 값
    int nDataCnt;   //헤더 뒤에 들어있는 데이터 갯수
    int nDataSize;  //헤더 뒤에 들어있는 데이터의 1개당 사이즈
    int nErrorCode; //에러코드
    char cMsg[64];  //메시지 내용, 64바이트 제한
} HEADER, *LPHEADER;
#define HEADER_SIZE sizeof(HEADER)
#define _HEADER_SIZE sizeof(_HEADER)

typedef struct _FILEINFO
{
    int nType;                 //파일타입
    unsigned long nContId;     //컨텐츠 번호
    unsigned long nContSeq;    //컨텐츠 번호
    char cServerIP[16];        //전송할 서버 IP
    long lServerPort;          //전송할 서버 PORT
    char cFolderName[512];     //전송할 파일이 있는 폴더 경로
    char cFileName[256];       //전송할 파일이름
    double dFileSize;          //파일크기
    double dFileDownSize;      //전송받기 시작할 파일 위치
    char cEncdFolderPath[512]; //인코딩 서버에 저장될 파일 경로
    char cEncdFileName[256];   //인코딩 서버에 저장될 파일 이름
    char cTempData[32];        //임시 데이터
    char cTempData2[32];       //임시 데이터2
    char cTempData3[32];       //임시 데이터3

} FILEINFO, *LPFILEINFO;
#define FILEINFO_SIZE sizeof(FILEINFO)
#define _FILEINFO_SIZE sizeof(_FILEINFO)
//파일 전송 정보

#endif
