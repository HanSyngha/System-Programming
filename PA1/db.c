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
int dbsize;

void freeing(db_t *db)
{
	if(db->left != NULL) freeing(db->left);
	if(db->right != NULL) freeing(db->right);
	free(db->word);
	free(db);
}

db_t *db_open(int size)
{	
	int i;
	dbsize = size;
	db_t *db = (db_t*)malloc(sizeof(db_t)*size);
	for(i=0;i<size;i++)
	{
		db[i].word = (char*)malloc(sizeof(char));
		*db[i].word = (char)94;
	}
	return db;
}

void db_close(db_t *db)
{
	int i;
	for(i=0;i<dbsize;i++)
	{
		if(db[i].left != NULL) freeing(db[i].left);
		if(db[i].right != NULL) freeing(db[i].right);
	}
	free(db);
}

void db_put(db_t *db, char *key, int key_len,
			char *val, int val_len)
{
	int i;
	int idx = key_len % dbsize;
	if(*(int*)val == 1)
	{
		db_t *insert = (db_t*)malloc(sizeof(db_t));
		insert->word = (char*)malloc(sizeof(char)*key_len);
		for(i=0;i<key_len;i++)
		{
			insert->word[i] = key[i];
		}
		insert->num = 1;
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
				find->num++;
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
	int idx = key_len % dbsize;
	db_t *find;
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
	return value;
}



