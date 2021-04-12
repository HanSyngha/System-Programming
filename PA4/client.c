#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unistd.h"
#include <pthread.h>

#define MAXLINE 80
int cfd;

int main (int argc, char *argv[]) {
	int n,rd,gp_rd,i;
	struct hostent *h;
	struct sockaddr_in saddr;
	char* buf = (char*)malloc(sizeof(char)*MAXLINE);
	char *gp_detect = (char*)malloc(sizeof(char)*4);
	char *read_buf = (char*)malloc(sizeof(char));
	char *host = argv[1];
	int port = atoi(argv[2]);

	if ((cfd= socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket() failed.\n");
		exit(1);
	}
	if ((h = gethostbyname(host)) == NULL) {
		printf("invalid hostname %s\n", host);
		exit(2);
	}

	bzero((char *)&saddr, sizeof(saddr));
	saddr.sin_family= AF_INET;
	bcopy((char *)h->h_addr, (char *)&saddr.sin_addr.s_addr, h->h_length);
	saddr.sin_port= htons(port);

	if (connect(cfd,(struct sockaddr*)&saddr,sizeof(saddr)) < 0) {
		printf("connect() failed.\n");
		exit(3);
	}
	while ((n = read(cfd, buf, 1)) > 0) {
		if(*buf == 'F')
		{
			printf("TOO MANY CLIENTS!! ( CONNECTION DENIDED ) \n");
			exit(0);
		}
		else if(*buf == 'C') break;
	}
	int stops = 0;
	while(1)
	{
		rd = 0;
		while(read(0,read_buf,1))
		{
			buf[rd++] = read_buf[0];
			if(*read_buf == '\n') break;
		}
		buf[rd] = '\0';
		if(!strcmp(buf,"CONNECT\n")) stops = 1;
		write(cfd,buf,rd);
		rd = 0;
		while(read(cfd,read_buf,1))
		{
			buf[rd++] = read_buf[0];
			if(*read_buf == '\n') break;
		}
		write(1,buf,rd);
		if(stops) break;
	}
	rd = 0;
	while(1)
	{
		for(i=0;i<MAXLINE;i++) buf[i] = '\0';
		rd = 0;
		gp_rd = 0;
		while(read(0,read_buf,1))
		{
			buf[rd++] = read_buf[0];
			if(gp_rd<3) gp_detect[gp_rd++] = read_buf[0];
			if(*read_buf == '\n') break;
		}
		buf[rd] = '\0';
		gp_detect[gp_rd] = '\0';
		if(!strcmp(buf,"DISCONNECT\n"))
		{
			write(cfd,"DISCONNECT\n",12);
			break;
		}
		else if(!strcmp(buf,"DISCONNECT"))
		{
			write(cfd,"DISCONNECT\n",12);
			break;
		}
		if(!strcmp(gp_detect,"GET"))
		{
			write(cfd,buf,strlen(buf));
			rd = 0;
			while(read(cfd,read_buf,1))
			{
				buf[rd++] = read_buf[0];
				if(*read_buf == '\n') break;
			}
			buf[rd] = '\0';
			write(1,buf,rd);
		}
		else if(!strcmp(gp_detect,"PUT"))
		{
			write(cfd,buf,strlen(buf));
			rd = 0;
			while(read(cfd,read_buf,1))
			{
				buf[rd++] = read_buf[0];
				if(*read_buf == '\n') break;
			}
			buf[rd] = '\0';
			write(1,buf,rd);
		}
		else
		{
			write(cfd,buf,strlen(buf));
			rd = 0;
			while(read(cfd,read_buf,1))
			{
				buf[rd++] = read_buf[0];
				if(*read_buf == '\n') break;
			}
			write(1,buf,rd);
		}
	}
	rd = 0;
	while(read(cfd,read_buf,1))
	{
		buf[rd++] = read_buf[0];
		if(*read_buf == '\n') break;
	}
	write(1,buf,rd);
	free(buf);
	free(gp_detect);
	free(read_buf);
	close(cfd);
}


