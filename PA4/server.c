#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_KEYLEN 1024
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netdb.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <strings.h>
 #include <arpa/inet.h>
 #include "unistd.h"
 #include <signal.h>
 #include <sys/wait.h>
 #include <time.h>
#include <signal.h>
#include <pthread.h>

 #define MAXLINE 80
typedef struct thread_argvss
{
	pthread_mutex_t lock;
	int t_num;
}thread_argvs;
int connfd;
thread_argvs *thread_argv = NULL;
void *thread(void *thread_argv);
	
db_t *DB;
int max_connection;

void handler(int SIG)
{
	db_close(DB);
	exit(0);
}

int main ( int argc , char * argv []) 
{
	signal(SIGINT, handler);
   	signal(SIGTSTP, handler);
	int i;
	thread_argv = (thread_argvs*)malloc(sizeof(thread_argvs));
	pthread_mutex_init (&thread_argv->lock,(pthread_mutexattr_t *)NULL);
	thread_argv->t_num = 0;

	int listenfd, caddrlen ;
	struct hostent *h;
	struct sockaddr_in saddr, caddr ;
	int port =atoi(argv [1]);
	max_connection = atoi(argv[2]);
	int size = atoi(argv[3]);
	DB = db_open(size);


	srand(time(NULL));

	if (( listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf ("socket() failed.\n");
		exit(1);
	}

	bzero ((char *)& saddr, sizeof ( saddr ));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr =	htonl (INADDR_ANY);
	saddr.sin_port =htons (port);
	
	if (bind( listenfd , ( struct sockaddr *)& saddr, sizeof ( saddr )) < 0) {
		printf ("bind() failed.\n");
		exit(2);
	}


	if (listen( listenfd ,max_connection ) < 0) {
		printf ("listen() failed.\n");
		exit(3);
	}
	pthread_t tid;
	 while (1) {
		caddrlen = sizeof ( caddr );
		if (( connfd = accept( listenfd , ( struct sockaddr *)& caddr , & caddrlen )) < 0) {
			printf ("accept() failed.\n");
			continue;
		}
		pthread_mutex_lock(&thread_argv->lock);
		pthread_create(&tid, NULL, thread, thread_argv);
		pthread_detach (tid); 
	}
}

void *thread(void *thread_argvs) /* Thread routine */
{
	int n;
	int my_id = connfd;
	if (thread_argv->t_num == max_connection)
	{
		if(write(my_id , "F",1));
		pthread_mutex_unlock(&thread_argv->lock);
		pthread_exit(NULL);
	}
	else if(write(my_id, "C",1));
	thread_argv->t_num++;
	pthread_mutex_unlock(&thread_argv->lock);
	char *buf = (char*)malloc(sizeof(char)*MAXLINE);
	char *valbuf = (char*)malloc(sizeof(char)*MAXLINE);
	char *valuebuf = (char*)malloc(sizeof(char)*10);
	char *read_buf = (char*)malloc(sizeof(char));
	char *gp_detect = (char*)malloc(sizeof(char)*4);
	int rd,gp_rd;
	int key_len;
	int i;
	int val;
	rd = 0;
	while(read(my_id,read_buf,1))
	{	
		buf[rd++] = *read_buf;
		if(*read_buf == '\n')
		{
			if(!strncmp(buf,"CONNECT",7))
			{
				if(write(my_id , "CONNECT_OK\n" ,11));
				break;
			}
			else 
			{
				if(write(my_id , "UNDEFINED PROTOCOL\n" ,19));
				rd = 0;
			}
		}
	}
	while(1)
	{
		rd = 0;
		gp_rd = 0;
		while(read(my_id,read_buf,1))
		{
			if(*read_buf == '\n') break;
			buf[rd++] = read_buf[0];
			if(gp_rd<3) gp_detect[gp_rd++] = read_buf[0];
		}
		buf[rd] = '\0';
		gp_detect[gp_rd] = '\0';
		if(!strcmp(buf,"DISCONNECT"))
		{
			if(write(my_id ,"BYE\n" ,4));
			break;
		}
		if(!strcmp(gp_detect,"GET"))
		{
			rd = 0;
			i = 0;
			while(buf[i] != '[') i++;
			i++;
			while(buf[i] != ']')
			{
				valbuf[rd++] = buf[i];
				i++;
			}
			valbuf[rd] = '\0';
			key_len = strlen(valbuf);
			val = db_get(DB, valbuf, key_len);
			if (val == 0)
			{
				if(write(my_id,"GETIN\n",6));
			}
			else
			{
				strcpy(buf,"GETOK [");
				strcat(buf,valbuf);
				strcat(buf,"] ");
				sprintf(valbuf,"[%d]\n",val);
				strcat(buf,valbuf);
				strcat(buf,"\0");
				if(write(my_id,buf,strlen(buf)));
				for(i=0;i<MAXLINE;i++) buf[i] = '\0';
			}
		}
		else if(!strcmp(gp_detect,"PUT"))
		{
			rd = 0;
			i = 0;
			while(buf[i] != '[') i++;
			i++;
			while(buf[i] != ']')
			{
				valbuf[rd++] = buf[i];
				i++;
			}
			valbuf[rd] = '\0';
			key_len = strlen(valbuf);
			rd = 0;
			while(buf[i] != '[') i++;
			i++;
			while(buf[i] != ']')
			{
				valuebuf[rd++] = buf[i];
				i++;
			}
			valuebuf[rd] = '\0';
			val = atoi(valuebuf);
			db_put(DB,valbuf,key_len,val);
			if(write(my_id,"PUTOK\n",6));
			for(i=0;i<10;i++) valuebuf[i] = '\0';
			for(i=0;i<MAXLINE;i++) buf[i] = '\0';
			for(i=0;i<MAXLINE;i++) valbuf[i] = '\0';
		}
		else 
		{
			if(write(my_id, "UNDEFINED PROTOCOL\n" ,20));
		}
	}
	free(buf);
	free(valbuf);
	free(valuebuf);
	free(read_buf);
	free(gp_detect);
	pthread_mutex_lock(&thread_argv->lock);
	thread_argv->t_num--;
	close(my_id);
	pthread_mutex_unlock(&thread_argv->lock);
	pthread_exit(NULL);
}



