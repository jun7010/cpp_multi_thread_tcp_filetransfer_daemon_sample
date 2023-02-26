
# Cpp Multi Thread TCP Filetransfer Daemon

쓰레드 방식으로 다중 접속을 할 수 있고, TCP 소켓 연결을 통해 파일 전송이 가능한 서버, 클라이언트 데몬 샘플 프로젝트입니다.


  

## [Requirements]

사용 언어 : C++

OS : Centos 7

개발 환경 : GCC / G++ 4.8.5에서 테스트 되었음

  
  

## [Usage]

make 명령어 사용 시 client, client_test / server 파일 생성됨.

server [queue_worker_size] [ip] [port]

client [queue_worker_size] [ip] [port]

client_test [server_ip] [server_port]

  

client_test는 src/lib.server/EncodeDaemonFunction.cpp의 EncdConnectionHandler::testFileinfoMake 함수의 데이터를 서버로 전송하여 임시 데이터를 통해 테스트 가능.

  
  

## [Project Documents]

server 파일이 파일을 받아올 서버에 / client 파일이 파일을 전송할 서버에서 실행된다고 가정한다.

외부 API 서버는 서버 데몬에 클라이언트 데몬이 있는 서버의 IP / 포트 / 파일 정보가 담긴 헤더 데이터를 전송하여 파일 전송이 실행된다.

  
  
---
FTP Server daemon

외부 API 서버로부터 받아올 파일에 대한 정보를 받아온 뒤, client 데몬에 연결하여 해당 파일에 대한 정보를 송신하고 파일을 받아온다

  

FTP Client daemon

서버로부터 정보를 받아온 파일을 전송한다

  ---
  

파일 전송 단계

1. 외부 API에서 server 데몬으로 받아올 파일 정보를 구조체 헤더에 넣어 전달한다. 이 때, 전달하는 정보는 파일이 존재하는 client 데몬 서버의 정보가 포함되어 있다.

2. server 데몬은 받아온 파일 정보를 토대로 자신이 해당 파일을 가지고 있는지 판단한다. 이 때, 파일이 없으면 새로 받아올 파일 / 파일이 없으면 file_size 비교를 통해 이어 받기를 해야 하는지 판단한다. 이후, 받아와야하는 남은 파일 사이즈를 계산하여 clinet에 파일 정보를 전달한다.

3. client 데몬은 서버로부터 전송된 파일 정보를 토대로 자신에게 해당 파일이 실제로 있는지 검사하여 server 데몬에 결과값을 리턴한다.

4. server 데몬은 client로부터 OK 신호를 받으면, 다운로드 준비가 되었다는 OK 신호를 답변한다.

5. client 데몬은 server 데몬으로부터 OK 신호를 받으면 파일 전송을 시작한다.

6. server 데몬은 받아온 데이터를 저장한다.

7. client 데몬은 모든 전송이 끝나면 전송 종료 신호를 server 데몬에 전달한다.

8. server 데몬은 파일 전송이 완료 되었을 경우, client 데몬과의 연결을 종료한다.

  
  

## [etc]

참조 오픈소스

https://github.com/vichargrave/mtserver

  
  

## [Licenses]

Apache License Version 2.0