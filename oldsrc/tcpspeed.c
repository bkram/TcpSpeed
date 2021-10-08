#ifdef AMIGA
#include <proto/socket.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#ifndef AMIGA
#include <sys/times.h>
#endif

/* $$$ comment out the following line if your OS uses old-style
       (BSD 4.2) sockaddr structures
   $$$ */
#define HASSINLEN

#define PORTNR 37215
#define BUFLEN 32768
#define BUFNUM 640

unsigned char buf[BUFLEN];

static void usage(char *pname) {
	fprintf(stderr,"Usage: \"%s s\" / \"%s c hostname\"\n",pname,pname);
	exit(0);
}

clock_t getelapsed(void) {
#ifdef AMIGA
	return clock();
#else
	struct tms tms;
	return times(&tms);
#endif
}

void main(int argc,char *argv[]) {
	char mode;
	int sock;
	int len,totallen;
	struct sockaddr_in sin;

	if((argc<2)||(argc>3))
		usage(argv[0]);
	mode=tolower(argv[1][0]);
	if((mode!='c')&&(mode!='s'))
		usage(argv[0]);
	if(((mode=='c')&&(argc!=3))||((mode=='s')&&(argc!=2)))
		usage(argv[0]);
	if(0>(sock=socket(AF_INET,SOCK_STREAM,0))) {
		fprintf(stderr,"Unable to create socket\n");
		exit(0);
	}
	if(mode=='c') {
		struct hostent *he;
		clock_t starttime,endtime;

		if(!(he=gethostbyname(argv[2]))) {
			fprintf(stderr,"Host not found\n");
			exit(0);
		}
		sin.sin_family=AF_INET;
#ifdef HASSINLEN
		sin.sin_len=sizeof(sin),
#endif
		memcpy(&sin.sin_addr,he->h_addr,he->h_length);
		sin.sin_port=htons(PORTNR);
		if(0>connect(sock,(struct sockaddr *)&sin,sizeof(sin))) {
			fprintf(stderr,"Unable to connect\n");
			exit(0);
		}
		totallen=0;
		starttime=getelapsed();
		while(0<(len=recv(sock,buf,BUFLEN,0)))
			totallen+=len;
		endtime=getelapsed();
#ifdef AMIGA
		CloseSocket(sock);
#else
		close(sock);
#endif
		printf("%ld bytes read, %d kB/s\n",
		 totallen,(((totallen/1024)*CLOCKS_PER_SEC)/(endtime-starttime)));
	} else {
		int sock2,i;
		long on=1;

		for(i=0;i<BUFLEN;i++)
			buf[i]=i;
#ifdef SO_REUSEADDR
		setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char *)&on,
		 sizeof(on));
#endif
		memset(&sin,0,sizeof(sin));
#ifdef HASSINLEN
		sin.sin_len=sizeof(sin);
#endif
		sin.sin_family=AF_INET;
		sin.sin_addr.s_addr=htonl(INADDR_ANY);
		sin.sin_port=htons(PORTNR);
		if(0>bind(sock,(struct sockaddr *)&sin,sizeof(sin))) {
			fprintf(stderr,"Unable to bind socket\n");
			exit(0);
		}
		listen(sock,5);
		if(0>(sock2=accept(sock,0,0))) {
			fprintf(stderr,"Unable to accept\n");
			exit(0);
		}
		for(i=0;i<BUFNUM;i++)
			if(0>=send(sock2,buf,BUFLEN,0))
				break;
#ifdef AMIGA
		CloseSocket(sock2);
		CloseSocket(sock);
#else
		close(sock2);
		close(sock);
#endif
	}
}
