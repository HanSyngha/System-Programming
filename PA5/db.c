/*-----------------------------------------------------------
 *
 * SSE2033: System Software Experiment 2 (Fall 2019)
 *
 * Skeleton code for PA #1
 * 
 * CSI Lab, Sungkyunkwan University
 *
 * Student Id   : 2016311821
 * Student Name : Han Syngha
 *
 *-----------------------------------------------------------
 */

#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <malloc.h>

int dbsize;
int wordcount;
int new_file_name;
void recovery(db_t *db)
{
	int i = 1;
	int rd,wr = 0;
	char buf[BUF_SIZE];
	char key[1024];
	char val[10];
	while(1)
	{
		lseek(log_fd,-i*BUF_SIZE,SEEK_END);
		if(read(log_fd,log_buf,BUF_SIZE));
		if(!strncmp((char*)log_buf,"OPEN",4)) break;
		if(!strncmp((char*)log_buf,"CHECKPOINT",10)) break;
		i++;
	}	
	while(i>0)
	{
		lseek(log_fd,-i*BUF_SIZE,SEEK_END);
		if(read(log_fd,log_buf,BUF_SIZE));
		if(!strncmp((char*)log_buf,"PUT$",4))
		{
			rd = 0;
			wr = 0;
			memcpy(buf,log_buf,sizeof(char)*BUF_SIZE);
			while(rd<BUF_SIZE) if(buf[rd++] == '$') break;
			while(rd<BUF_SIZE)
			{
				if(buf[rd] == '$') break;
				key[wr++] = buf[rd++];
			}
			rd++;
			key[wr] = '\0';
			wr = 0;
			while(rd<BUF_SIZE)
			{
				if(buf[rd] == '$') break;
				val[wr++] = buf[rd++];
			}
			val[wr] = '\0';
			db_put_with_no_log(db,key,strlen(key),val,strlen(val));
		}
		i--;
	}
}
int findw(db_t *db,char* key,int idx)
{
	db_t *find;
	if(key[0]>db[idx].word[0]) find = db[idx].right;
	else find = db[idx].left;
	while(find != NULL)
	{
		if(strcmp(key,find->word) == 0)
		{
			return 1;
		}
		if(key[0]>find->word[0]) find = find->right;
		else find = find->left;
	}
	return 0;
}
void save_and_free(db_t *db,int idx)
{
	if(db->left != NULL) save_and_free(db->left,idx);
	if(db->right != NULL) save_and_free(db->right,idx);
	char* filename = (char*)malloc(sizeof(char)*20);
	char* filenum = (char*)malloc(sizeof(char)*10);
	char* num = (char*)malloc(sizeof(char)*10);
	sprintf(num,"%d",db->num);
	strcat(num,"\n");
	strcpy(filename,"./db/");
	sprintf(filenum,"%d",(new_file_name*dbsize)+idx);
	strcat(filename,filenum);
	free(filenum);
	int file = open(filename,O_CREAT|O_WRONLY,0777);
	lseek(file,0,SEEK_END);
	if(write(file,db->word,strlen(db->word)));
	if(write(file," ",1));
	if(write(file,num,strlen(num)));
	free(filename);
	free(num);
	free(db->word);
	free(db);
	fsync(file);
	close(file);
}

db_t *db_open(int size)
{	
	int i,idx = 0;
	char* buffer = (char*)malloc(sizeof(char));
	char* filenum = (char*)malloc(sizeof(char)*10);
	dbsize = size;
	wordcount = 0;
	new_file_name = 0;
	if(fork() == 0) execl("/bin/mkdir","mkdir","-p","db",NULL);
	else wait(NULL);
	int get_newname = open("./db/new_file_name",O_RDONLY);
	if(get_newname != -1)
	{
		idx = 0;
		while(read(get_newname,buffer,1))
		{
			if(*buffer == '$') break;
			filenum[idx++] = *buffer; 
		}
		filenum[idx] = '\0';
		new_file_name = atoi(filenum);
	}
	close(get_newname);
	free(buffer);
	free(filenum);
	db_t *db = (db_t*)malloc(sizeof(db_t)*size);
	for(i=0;i<size;i++)
	{
		db[i].word = (char*)malloc(sizeof(char));
		*db[i].word = (char)94;
		db[i].left = NULL;
		db[i].right = NULL;
	}
	if(posix_memalign(&log_buf,SECTOR_SIZE,BUF_SIZE)) perror("ALLIGN ERROR\n");
	return db;
}

void db_close(db_t *db)
{
	int i;
	char* checkpoint = (char*)malloc(sizeof(char)*12);
	for(i=0;i<dbsize;i++)
	{
		if(db[i].left != NULL) save_and_free(db[i].left,i);
		if(db[i].right != NULL) save_and_free(db[i].right,i);
	}
	new_file_name++;
	int get_newname = open("./db/new_file_name",O_CREAT|O_RDWR,0777);
	char* filenum = (char*)malloc(sizeof(char)*10);
	sprintf(filenum,"%d",new_file_name);
	if(write(get_newname,filenum,strlen(filenum)));
	if(write(get_newname,"$",1));
	fsync(get_newname);
	close(get_newname);
	strcpy(checkpoint,"CHECKPOINT\n");
	memcpy(log_buf,checkpoint,sizeof(char)*11);
	if(write(log_fd,log_buf,BUF_SIZE));
	fsync(log_fd);
	free(checkpoint);
	free(db);
}

void db_put(db_t *db, char *key, int keylen, char *val, int vallen)
{
	int i;
	int idx =0;
	int value = atoi(val);
	char* key_buf = (char*)malloc(sizeof(char)*1024);
	strcpy(key_buf,"PUT$");
	strcat(key_buf,key);
	strcat(key_buf,"$");
	strcat(key_buf,val);
	strcat(key_buf,"$\n");
	memcpy(log_buf,key_buf,sizeof(char)*strlen(key_buf));
	if(write(log_fd,log_buf,BUF_SIZE));
	fsync(log_fd);
	free(key_buf);
	for(i=0;i<keylen;i++) idx = idx + key[i];
	idx = idx%dbsize;
	if(wordcount == 100) //flush when there is 100 key-value pair
	{
		for(i=0;i<dbsize;i++)
		{
			if(db[i].left != NULL) save_and_free(db[i].left,i);
			if(db[i].right != NULL) save_and_free(db[i].right,i);
			db[i].left = NULL;
			db[i].right = NULL;
		}
		new_file_name++;
		int get_newname = open("./db/new_file_name",O_CREAT|O_RDWR,0777);
		char* filenum = (char*)malloc(sizeof(char)*10);
		sprintf(filenum,"%d",new_file_name);
		if(write(get_newname,filenum,strlen(filenum)));
		if(write(get_newname,"$",1));
		fsync(get_newname);
		close(get_newname);
		char* checkpoint = (char*)malloc(sizeof(char)*12);
		strcpy(checkpoint,"CHECKPOINT\n");
		memcpy(log_buf,checkpoint,sizeof(char)*11);
		if(write(log_fd,log_buf,BUF_SIZE));
		fsync(log_fd);
		free(checkpoint);
		wordcount = 0;
	}
	int isthere = findw(db,key,idx);
	if(isthere == 0)
	{
		wordcount++;
		db_t *insert = (db_t*)malloc(sizeof(db_t));
		insert->word = (char*)malloc(sizeof(char)*(keylen+1));
		for(i=0;i<keylen;i++)
		{
			insert->word[i] = key[i];
		}
		insert->word[keylen] = '\0';
		insert->num = value;
		insert->left = NULL;
		insert->right = NULL;
		db_t *find = NULL;
		if(key[0]>db[idx].word[0])
		{
			if(db[idx].right == NULL)
			{
				db[idx].right = insert;
				return;
			}
			else find = db[idx].right;
		}
		else
		{
			if(db[idx].left == NULL)
			{
				db[idx].left = insert;
				return;
			}
			else find = db[idx].left;
		}
		while(1)
		{
			if(insert->word[0]>find->word[0])
			{
				if(find->right != NULL) find = find->right;
				else{
					find->right = insert;
					return;
				}
			}
			else
			{
				if(find->left != NULL) find = find->left;
				else{
					find->left = insert;
					return;
				}
			}
		}
	}
	else
	{	
		db_t *find;
		if(key[0]>db[idx].word[0]) find = db[idx].right;
		else find = db[idx].left;
		while(find != NULL)
		{
			if(strcmp(key,find->word) == 0)
			{
				find->num = value;
			}
			if(key[0]>find->word[0]) find = find->right;
			else find = find->left;
		}
	}
}

void db_put_with_no_log(db_t *db, char *key, int keylen, char *val, int vallen)
{
	int i;
	int idx =0;
	int value = atoi(val);
	for(i=0;i<keylen;i++) idx = idx + key[i];
	idx = idx%dbsize;
	if(wordcount == 100) //flush when there is 100 key-value pair
	{
		for(i=0;i<dbsize;i++)
		{
			if(db[i].left != NULL) save_and_free(db[i].left,i);
			if(db[i].right != NULL) save_and_free(db[i].right,i);
			db[i].left = NULL;
			db[i].right = NULL;
		}
		new_file_name++;
		int get_newname = open("./db/new_file_name",O_CREAT|O_RDWR,0777);
		char* filenum = (char*)malloc(sizeof(char)*10);
		sprintf(filenum,"%d",new_file_name);
		if(write(get_newname,filenum,strlen(filenum)));
		if(write(get_newname,"$",1));
		fsync(get_newname);
		close(get_newname);
		char* checkpoint = (char*)malloc(sizeof(char)*12);
		strcpy(checkpoint,"CHECKPOINT\n");
		memcpy(log_buf,checkpoint,sizeof(char)*11);
		if(write(log_fd,log_buf,BUF_SIZE));
		fsync(log_fd);
		free(checkpoint);
		wordcount = 0;
	}
	int isthere = findw(db,key,idx);
	if(isthere == 0)
	{
		wordcount++;
		db_t *insert = (db_t*)malloc(sizeof(db_t));
		insert->word = (char*)malloc(sizeof(char)*(keylen+1));
		for(i=0;i<keylen;i++)
		{
			insert->word[i] = key[i];
		}
		insert->word[keylen] = '\0';
		insert->num = value;
		insert->left = NULL;
		insert->right = NULL;
		db_t *find = NULL;
		if(key[0]>db[idx].word[0])
		{
			if(db[idx].right == NULL)
			{
				db[idx].right = insert;
				return;
			}
			else find = db[idx].right;
		}
		else
		{
			if(db[idx].left == NULL)
			{
				db[idx].left = insert;
				return;
			}
			else find = db[idx].left;
		}
		while(1)
		{
			if(insert->word[0]>find->word[0])
			{
				if(find->right != NULL) find = find->right;
				else{
					find->right = insert;
					return;
				}
			}
			else
			{
				if(find->left != NULL) find = find->left;
				else{
					find->left = insert;
					return;
				}
			}
		}
	}
	else
	{	
		db_t *find;
		if(key[0]>db[idx].word[0]) find = db[idx].right;
		else find = db[idx].left;
		while(find != NULL)
		{
			if(strcmp(key,find->word) == 0)
			{
				find->num = value;
			}
			if(key[0]>find->word[0]) find = find->right;
			else find = find->left;
		}
	}
}
/* Returns NULL if not found. A malloc()ed array otherwise.
 * Stores the length of the array in *vallen */
char *db_get(db_t *db, char *key, int keylen, int *vallen)
{
	char *value = NULL;
	int idx = 0;
	char* rword = (char*)malloc(sizeof(char)*(keylen+1));
	char* num = (char*)malloc(sizeof(char)*8);
	char buffer[1];
	int i,j,tmp = 0;
	int found;
	db_t *find;
	for(i=0;i<keylen;i++) idx = idx + key[i];
	idx = idx%dbsize;
	if(key[0]>db[idx].word[0]) find = db[idx].right;
	else find = db[idx].left;
	while(find != NULL)
	{
		if(strcmp(key,find->word) == 0)
		{
			value = (char*)malloc(sizeof(int));
			memcpy((void*)value,(void*)(&find->num),sizeof(int));
		}
		if(key[0]>find->word[0]) find = find->right;
		else find = find->left;
	}
	found = 0;
	for(i=new_file_name-1;i>=0;i--)
	{
		if(found) break;
		j = 0;
		if(value != NULL) break;
		char* filename = (char*)malloc(sizeof(char)*20);
		char* filenum = (char*)malloc(sizeof(char)*10);
		int searchf;
		strcpy(filename,"./db/");
		sprintf(filenum,"%d",(i*dbsize)+idx);
		strcat(filename,filenum);
		searchf = open(filename,O_RDONLY);
		if(searchf == -1){
			free(filename);
			free(filenum);
			continue;
		}
		while(read(searchf,buffer,1))
		{
			for(j=0;j<keylen+1;j++)
			{	
				if(buffer[0] == ' '){
					 break;
				}
				rword[j] = buffer[0];
				if(read(searchf,buffer,1));
			}
			rword[j] = '\0';
			if(strcmp(rword,key) == 0 && j != keylen+1)
			{
				for(j = 0;j<100;j++)
				{
					if(read(searchf,buffer,1));
					if(buffer[0] == '\n') break;
					num[j] = buffer[0];
				}
				num[j] = '\0';
				tmp = atoi(num);
				value = (char*)malloc(sizeof(int));
				memcpy((void*)value,(void*)(&tmp),sizeof(int));
				found = 1;
				break;
			}
			else while(buffer[0] != '\n') if(read(searchf,buffer,1));
		}
		for(j=0;j<keylen;j++)
		{
			rword[j] = '\0';
		}
		free(filename);
		free(filenum);
		close(searchf);
	}
	free(num);
	free(rword);
	return value;
}



