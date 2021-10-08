#ifdef AMIGA
#include <proto/socket.h>
#endif
#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#if !defined(AMIGA) && !defined(WIN32)
#include <sys/times.h>
#endif

#ifndef WIN32
#define HASSINLEN
#endif

#ifdef LINUX
#undef HASSINLEN
#include <fcntl.h>
#include <unistd.h>
#endif

#define PORTNR 37215
#define BUFLEN 32768
#define BUFNUM 640

float perc;
float MAX = BUFNUM;

unsigned char buf[BUFLEN];

static void usage(char *pname)
{
    fprintf(stderr, "Usage: \"%s s\" / \"%s c hostname\"\n", pname, pname);
    exit(0);
}

clock_t getelapsed(void)
{
#if defined(AMIGA) || defined(WIN32)
    return clock();
#else
    struct tms tms;
    return times(&tms);
#endif
}

void main(int argc, char *argv[])
{
    char mode;
    int sock;
    int len, totallen;
    struct sockaddr_in sin;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

#ifdef WIN32
    WORD VersionRequested = MAKEWORD(2, 0);
    WSADATA WsaData;

    WSAStartup(VersionRequested, &WsaData);
#endif

    if ((argc < 2) || (argc > 3))
        usage(argv[0]);
    mode = tolower(argv[1][0]);
    if ((mode != 'c') && (mode != 's'))
        usage(argv[0]);
    if (((mode == 'c') && (argc != 3)) || ((mode == 's') && (argc != 2)))
        usage(argv[0]);
    if (0 > (sock = socket(AF_INET, SOCK_STREAM, 0)))
    {
        fprintf(stderr, "Unable to create socket\n");
        exit(0);
    }
    if (mode == 'c')
    {
        struct hostent *he;
        clock_t starttime, endtime;

        if (!(he = gethostbyname(argv[2])))
        {
            fprintf(stderr, "Host not found\n");
            exit(0);
        }
        sin.sin_family = AF_INET;
#ifdef HASSINLEN
        sin.sin_len = sizeof(sin),
#endif
        memcpy(&sin.sin_addr, he->h_addr, he->h_length);
        sin.sin_port = htons(PORTNR);
        if (0 > connect(sock, (struct sockaddr *)&sin, sizeof(sin)))
        {
            fprintf(stderr, "Unable to connect\n");
            exit(0);
        }
        totallen = 0;
        starttime = getelapsed();
        while (0 < (len = recv(sock, buf, BUFLEN, 0)))
            totallen += len;
        endtime = getelapsed();

#ifdef AMIGA
        CloseSocket(sock);
#elif defined(WIN32)
        closesocket(sock);
#else
        close(sock);
#endif
        printf("%d bytes read, %lu kB/s\n",
               totallen, (((totallen / 1024) * CLOCKS_PER_SEC) / (endtime - starttime)));
    }
    else
    {
        int sock2, i;
        long on = 1;

        for (i = 0; i < BUFLEN; i++)
            buf[i] = i;
#ifdef SO_REUSEADDR
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on,
                   sizeof(on));
#endif
        memset(&sin, 0, sizeof(sin));
#ifdef HASSINLEN
        sin.sin_len = sizeof(sin);
#endif
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = htonl(INADDR_ANY);
        sin.sin_port = htons(PORTNR);
        if (0 > bind(sock, (struct sockaddr *)&sin, sizeof(sin)))
        {
            fprintf(stderr, "Unable to bind socket\n");
            exit(0);
        }
        else
        {
            printf("Waiting for a connection\n");
        }
        listen(sock, 5);


        sock2 = accept(sock, (struct sockaddr *)&address,
                       (socklen_t *)&addrlen);

        if (0 > sock2)
        {
            fprintf(stderr, "Unable to accept\n");
            exit(0);
        }

        printf("New connection from ip: %s\n", inet_ntoa(address.sin_addr));

        printf("Starting sending buffers\n");

        for (i = 0; i < BUFNUM + 1; i++)
            if (0 >= send(sock2, buf, BUFLEN, 0))
            {
                break;
            }
            else if (i > 0)
            {
                perc = i / MAX * 100;
                printf("Sending %6.2f%%\n", perc);
                if (i != BUFNUM)
                {
                    printf("\033[A\033[2K");
                }
            }

#ifdef AMIGA
        CloseSocket(sock2);
        CloseSocket(sock);
#elif defined(WIN32)
        closesocket(sock2);
        closesocket(sock);
#else
        close(sock2);
        close(sock);
#endif
    }

#ifdef WIN32
    WSACleanup();
#endif
}
