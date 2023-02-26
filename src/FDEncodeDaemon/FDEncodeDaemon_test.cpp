#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <signal.h>
#include "EncodeDaemonFunction.h"

using namespace std;

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("usage: %s <port> <ip>\n", argv[0]);
        exit(1);
    }

    signal(SIGPIPE, SIG_IGN);

    TCPConnector *connector = new TCPConnector();
    TCPStream *stream = connector->connect(argv[2], atoi(argv[1]));

    printf("Connect Start : %s  / %s\n", argv[2], argv[1]);
    EncdConnectionHandler::FileTransferTEST(stream);

    printf("ALL PROCESS END!!!!\n");
    exit(0);
}