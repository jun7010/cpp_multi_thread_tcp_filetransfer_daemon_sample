#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <signal.h>
#include "EncodeDaemonFunction.h"

using namespace std;

int main(int argc, char **argv)
{
    // Process command line arguments
    if (argc < 3 || argc > 4)
    {
        printf("usage: %s <workers> <port> <ip>\n", argv[0]);
        exit(-1);
    }
    int workers = atoi(argv[1]);
    int port = atoi(argv[2]);
    string ip;
    if (argc == 4)
    {
        ip = argv[3];
    }

    signal(SIGPIPE, SIG_IGN);

    serverStart(workers, port, ip);

    // Should never get here
    exit(0);
}

void serverStart(int workers, int port, string ip)
{
    // Create the queue and consumer (worker) threads
    wqueue<WorkItem *> queue;
    for (int i = 0; i < workers; i++)
    {
        EncdConnectionHandler *handler = new EncdConnectionHandler(queue);
        if (!handler)
        {
            printf("Could not create ConnectionHandler %d\n", i);
            exit(1);
        }
        handler->start();
    }

    // Create an acceptor then start listening for connections
    WorkItem *item;
    TCPAcceptor *connectionAcceptor;
    if (ip.length() > 0)
    {
        connectionAcceptor = new TCPAcceptor(port, (char *)ip.c_str());
    }
    else
    {
        connectionAcceptor = new TCPAcceptor(port);
    }
    if (!connectionAcceptor || connectionAcceptor->start() != 0)
    {
        printf("Could not create an connection acceptor\n");
        exit(1);
    }

    // Add a work item to the queue for each connection
    while (1)
    {
        TCPStream *connection = connectionAcceptor->accept();
        if (!connection)
        {
            printf("Could not accept a connection\n");
            continue;
        }
        item = new WorkItem(connection);
        if (!item)
        {
            printf("Could not create work item a connection\n");
            continue;
        }
        queue.add(item);
    }
}

int main_bck(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("usage: %s <port> <ip>\n", argv[0]);
        exit(1);
    }

    TCPConnector *connector = new TCPConnector();
    TCPStream *stream = connector->connect(argv[2], atoi(argv[1]));

    EncdConnectionHandler::FileTransferFunction(stream);

    printf("ALL PROCESS END!!!!\n");
    exit(0);
}