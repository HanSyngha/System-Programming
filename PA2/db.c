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
int dbsize;
int wordcount;
int new_file_name;

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
	close(file);
}

db_t *db_open(int size)
{	
	int i,tmp,idx = 0;
	char* buffer = (char*)malloc(sizeof(char));
	char* filenum = (char*)malloc(sizeof(char)*10);
	dbsize = size;
	wordcount = 0;
	new_file_name = 0;
	if(fork() == 0) execl("/bin/mkdir","mkdir","-p","db",NULL);
	else wait(NULL);
	if(fork() == 0) 
	{	
		int lslist = open("lslist.txt",O_CREAT|O_RDWR,0777);
		dup2(lslist,1);
		execl("/bin/ls","ls","db",NULL);
	}
	else wait(NULL);
	int lslist = open("lslist.txt",O_CREAT|O_RDWR,0777);
	while(read(lslist,buffer,1))
	{
		if(*buffer != '\n') filenum[idx++] = *buffer;
		else
		{
			tmp = atoi(filenum);
			if(tmp>new_file_name) new_file_name = tmp;
			for(i=0;i<10;i++) filenum[i] = 0;
			idx = 0;
		}
	}
	new_file_name = (new_file_name/dbsize) + 1;
	free(buffer);
	free(filenum);
	close(lslist);
	db_t *db = (db_t*)malloc(sizeof(db_t)*size);
	for(i=0;i<size;i++)
	{
		db[i].word = (char*)malloc(sizeof(char));
		*db[i].word = (char)94;
		db[i].left = NULL;
		db[i].right = NULL;
	}
	return db;
}

void db_close(db_t *db)
{
	int i;
	for(i=0;i<dbsize;i++)
	{
		if(db[i].left != NULL) save_and_free(db[i].left,i);
		if(db[i].right != NULL) save_and_free(db[i].right,i);
	}
	free(db);
	if(fork() == 0) execl("/bin/rm","rm","lslist.txt",NULL);
}

void db_put(db_t *db, char *key, int key_len,
			char *val, int val_len)
{
	int i;
	int idx =0;
	for(i=0;i<key_len;i++) idx = idx + key[i];
	idx = idx%dbsize;
	if(wordcount == dbsize)
	{
		for(i=0;i<dbsize;i++)
		{
			if(db[i].left != NULL) save_and_free(db[i].left,i);
			if(db[i].right != NULL) save_and_free(db[i].right,i);
			db[i].left = NULL;
			db[i].right = NULL;
		}
		new_file_name++;
		wordcount = 0;
	}
	int isthere = findw(db,key,idx);
	if(isthere == 0)
	{
		wordcount++;
		db_t *insert = (db_t*)malloc(sizeof(db_t));
		insert->word = (char*)malloc(sizeof(char)*(key_len+1));
		for(i=0;i<key_len;i++)
		{
			insert->word[i] = key[i];
		}
		insert->word[key_len] = '\0';
		insert->num = *(int*)val;
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
				find->num = *(int*)val;
			}
			if(key[0]>find->word[0]) find = find->right;
			else find = find->left;
		}
	}
}

/* Returns NULL if not found. A malloc()ed array otherwise.
 * Stores the length of the array in *vallen */
char *db_get(db_t *db, char *key, int key_len,
			int *val_len)
{
	char *value = NULL;
	int idx = 0;
	char* rword = (char*)malloc(sizeof(char)*(key_len+1));
	char* num = (char*)malloc(sizeof(char)*8);
	char buffer[1];
	int i,j,tmp = 0;
	int found;
	db_t *find;
	for(i=0;i<key_len;i++) idx = idx + key[i];
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
	for(i=new_file_name-1;i>0;i--)
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
			for(j=0;j<key_len+1;j++)
			{	
				if(buffer[0] == ' '){
					 break;
				}
				rword[j] = buffer[0];
				if(read(searchf,buffer,1));
			}
			rword[j] = '\0';
			if(strcmp(rword,key) == 0 && j != key_len+1)
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
		for(j=0;j<key_len;j++)
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



