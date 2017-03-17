/*
This is a very simple HTTP server. Default port is 8888 and ROOT for the server is your current working directory..

You can provide command line arguments like:- $./server -p [port] -r [path]

for ex. 
$./server -p 50000 -r /home/
to start a server at port 50000 with root directory as "/home"

$./server -r /home/shadyabhi
starts the server at port 10000 with ROOT as /home/shadyabhi
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>

#define CONNMAX 1000
#define BYTES 1024
#define DEFAULT_PORT "8888"

char *ROOT;

int listenfd, clients[CONNMAX];

void error(char *);
void startServer(char *);
void respond(int);

int main(int argc, char *argv[])
{
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char c;

    //Default Values PATH=./ and PORT=10000
    char PORT[6];
    ROOT = getenv("PWD");
    strcpy(PORT, DEFAULT_PORT);

    

    //Parsing the command line arguments
    while ((c = getopt(argc, argv, "p:r:")) != -1)
        switch (c)
        {
            case 'r':
                ROOT = malloc(strlen(optarg));
                strcpy(ROOT, optarg);
                break;
            case 'p':
                strcpy(PORT, optarg);
                break;
            case '?':
                fprintf(stderr, "Unrecognized option.\n");
            default:
                exit(1);
        }

    printf("Server started at port no. %s%s%s with root directory as %s%s%s\n", 
            "\033[92m", PORT, "\033[0m", "\033[92m", ROOT, "\033[0m");
    
    // Setting all elements to -1: signifies there is no client connected
    for (int i = 0; i < CONNMAX; i++) 
    {
        clients[i] = -1;
    }
    
    startServer(PORT);

    int slot = 0;

    // ACCEPT connections
    while (1)
    {
        addrlen = sizeof(clientaddr);
        clients[slot] = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

        if (clients[slot] < 0)
            error("accept() error");
        else
        {
            if (fork() == 0)
            {
                respond(slot);
                exit(0);
            }
        }

        while (clients[slot] != -1)
            slot = (slot + 1) % CONNMAX;
    }

    return 0;
}

//start server
void startServer(char *port)
{
    struct addrinfo hints, *res, *p;

    // getaddrinfo for host
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &res) != 0)
    {
        perror("getaddrinfo() error");
        exit(1);
    }
    
    // socket and bind
    for (p = res; p != NULL; p = p->ai_next)
    {
        listenfd = socket(p->ai_family, p->ai_socktype, 0);
        if (listenfd == -1)
            continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
    }
    if (p == NULL)
    {
        perror("socket() or bind()");
        exit(1);
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if (listen(listenfd, 1000000) != 0)
    {
        perror("listen() error");
        exit(1);
    }
}

//client connection
void respond(int n)
{
    char msg[99999], *reqline[3], data_to_send[BYTES], path[99999];
    int rcvd, fd, bytes_read;

    memset((void *)msg, (int)'\0', 99999);

    rcvd = recv(clients[n], msg, 99999, 0);

    if (rcvd < 0) // receive error
        fprintf(stderr, ("recv() error\n"));
    else if (rcvd == 0) // receive socket closed
        fprintf(stderr, "Client disconnected upexpectedly.\n");
    else // message received
    {
        printf("%s", msg);
        reqline[0] = strtok(msg, " \t\n");
        if (strncmp(reqline[0], "GET\0", 4) == 0)
        {
            reqline[1] = strtok(NULL, " \t");
            reqline[2] = strtok(NULL, " \t\n");
            if (strncmp(reqline[2], "HTTP/1.0", 8) != 0 && strncmp(reqline[2], "HTTP/1.1", 8) != 0)
            {
                write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
            }
            else
            {
                if (strncmp(reqline[1], "/\0", 2) == 0)
                    reqline[1] = "/index.html"; //Because if no file is specified, index.html will be opened by default

                strcpy(path, ROOT);
                strcpy(&path[strlen(ROOT)], reqline[1]);
                printf("file: %s\n", path);

                if ((fd = open(path, O_RDONLY)) != -1) //FILE FOUND
                {
                    send(clients[n], "HTTP/1.0 200 OK\n\n", 17, 0);
                    while ((bytes_read = read(fd, data_to_send, BYTES)) > 0)
                        write(clients[n], data_to_send, bytes_read);
                }
                else
                    write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
            }
        }
    }

    //Closing SOCKET
    shutdown(clients[n], SHUT_RDWR); //All further send and recieve operations are DISABLED...
    close(clients[n]);
    clients[n] = -1;
}