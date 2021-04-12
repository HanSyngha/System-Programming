#define _GNU_SOURCE

#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#define MAX_KEYLEN 1024

int main(int argc, char *argv[])
{
	db_t *DB;
	char key[MAX_KEYLEN];
	char bf_pars[2*MAX_KEYLEN];
	char buf[1];
	char *val;
	char value[MAX_KEYLEN];
	int size;
	int key_len, val_len;
	int key_wr,wr,val_wr;
	int i;
	int end = 0;

	if (argc != 2) {
		printf("Usage: %s size\n", argv[0]);
		return -1;
	}

	size = atoi(argv[1]);
	DB = db_open(size);
	if (DB == NULL) {
		printf("DB not opened\n");
		return -1;
	}
	printf("DB opened\n");

	if ((log_fd = open("./db_log", O_RDWR|O_CREAT|O_DIRECT, 0755)) < 0) {
		perror("open db_log");
		exit(1);
	}
	posix_fallocate(log_fd, 0, 4096*256*256);  // 256MB
	printf("DB log file opened\n");

	char* openstr = (char*)malloc(sizeof(char)*6);
	strcpy(openstr,"OPEN\n");
	memcpy(log_buf,openstr,sizeof(char)*5);
	if(write(log_fd,log_buf,BUF_SIZE));
	fsync(log_fd);
	free(openstr);
	recovery(DB);
	while(1)
	{
		if(end) break;
		wr = 0;
		while(read(0,buf,1))
		{
			if(*buf == '\n') break;
			bf_pars[wr++] = *buf;
		}
		if(!strncmp(bf_pars,"GET",3))
		{
			key_wr = 0;
			for(i=0;i<wr;i++) if(bf_pars[i] == '[') break;
			i++;
			while(i<=wr)
			{
				if(bf_pars[i] == ']') break;
				key[key_wr++] = bf_pars[i++];
			}
			key[key_wr] = '\0';
			key_len = strlen(key);
			val = db_get(DB,key,key_len,&val_len);
			if (val == NULL)
			{
				printf("GETOK [%s] [NULL]\n",key);
			}
			else
			{
				printf("GETOK [%s] [%d]\n",key,*(int*)val);
				free(val);
			}
		}
		else if(!strncmp(bf_pars,"PUT",3))
		{
			key_wr = 0;
			val_wr = 0;
			for(i=0;i<wr;i++) if(bf_pars[i] == '[') break;
			i++;
			while(i<=wr)
			{
				if(bf_pars[i] == ']') break;
				key[key_wr++] = bf_pars[i++];
			}
			key[key_wr] = '\0';
			for(;i<wr;i++) if(bf_pars[i] == '[') break;
			i++;
			while(i<=wr)
			{
				if(bf_pars[i] == ']') break;
				value[val_wr++] = bf_pars[i++];
			}
			value[val_wr] = '\0';
			key_len = strlen(key);
			val_len = strlen(value);
			printf("PUTOK\n");
			db_put(DB,key,key_len,value,val_len);
		}
		else if(!strncmp(bf_pars,"DB_CLOSE",8)) break;
	}

	db_close(DB);
	close(log_fd);
	printf("DB closed\n");
	return 0;
}
