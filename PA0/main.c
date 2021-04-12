//------------------------------------------------------------------
// SSE2033: System Software Experiment 2 (Fall 2019)
//
// Skeleton code for PA #0
//
// Computer Systems and Intelligence Laboratory
// Sungkyunkwan University
//
// Student Name  : Han Syngha
// Student ID No.: 2016311821
//------------------------------------------------------------------
#include <unistd.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct node
{
	char* text;
	int num;
	struct node* left;
	struct node* right;		
}Node;

typedef struct shape
{
	char* newword;
	int len;
}shapeof;


int check(char *text)
{
	if('a'<=*text && *text<='z')
	{
		 return 1;
	}
	else if('A'<=*text && *text<='Z')
	{	
		 return 1;
	}
	else if(*text == '-' || *text == 39)
	{	
		 return 2;
	}
	return 0;
}

int find_big(char* src, char* dest)
{
	int idx = 0;
	while(1)
	{
		if(src[idx] == '\0' && dest[idx] == '\0') return 2;
		if(src[idx] == '\0') return 1;
		else if(dest[idx] == '\0') return 0;
		else
		{
		}
		if(src[idx] == dest[idx]) idx++;
		else
		{
			if(src[idx] == 39) return 1;
			if(dest[idx] == 39) return 0;
			if(src[idx] == '-') return 1;
			if(dest[idx] == '-') return 0;
			if(src[idx]<='Z' && 'a'<=dest[idx])
			{
				if(src[idx] > dest[idx] - 32) return 0;
				if(src[idx] < dest[idx] - 32) return 1;
				if(src[idx] == dest[idx] - 32) return 1;
			}
			if('a'<=src[idx] && dest[idx]<='Z')
			{
				if(src[idx] -32 > dest[idx]) return 0;
				if(src[idx] -32 < dest[idx]) return 1;
				if(src[idx] -32 == dest[idx]) return 0;
			}
			if(src[idx]<dest[idx]) return 1;
			else return 0;
			
		}
	}
	return 0;
}

Node* insert(Node* Head, char* text,int len)
{
	int check;
	int idx = 0;
	Node* temp = (Node*)malloc(sizeof(Node));
	Node* find = Head;
	temp->text = (char*)malloc(sizeof(char)*(len+1));
	temp->num = 1;
	temp->left = NULL;
	temp->right = NULL;
	for(idx=0;idx<len;idx++)
	{
		temp->text[idx] = text[idx];
	}
	temp->text[idx] = '\0';
	while(1)
	{
		check = find_big(find->text,temp->text);
		if(check == 0)
		{
			if(find->left == NULL)
			{
				find->left = temp;
				return Head;
			}
			else find = find->left;
		}
		else if(check == 1)
		{
			if(find->right == NULL)
			{	
				find->right = temp;
					return Head;
			}
			else find = find->right;
		}
		else if(check == 2) 
		{
			find->num++;
			free(temp);
			return Head;
		}
	}
}		


Node* NewNode(char* text,int len)
{
	int idx = 0;
	Node* new = (Node*)malloc(sizeof(Node));
	new->text = (char*)malloc(sizeof(char)*(len+1));
	for(idx=0;idx<len;idx++)
	{
		new->text[idx] = text[idx];
	}
	new->text[idx] = '\0';
	new->num = 1;
	new->left = NULL;
	new->right = NULL;
	return new;
}

void print_all(Node* Head)
{
	char* temp = (char*)malloc(sizeof(char)*(strlen(Head->text)+5));
	char* int_temp = (char*)malloc(sizeof(char)*5);
	if(Head->left != NULL) print_all(Head->left);
	sprintf(int_temp,"%d",Head->num);
	strcpy(temp,Head->text);
	strcat(temp," ");
	strcat(temp,int_temp);
	write(1,temp,strlen(temp));
	write(1,"\n",1);
	if(Head->right != NULL) print_all(Head->right);
}

shapeof* shape(char* text,int len)
{
	shapeof* newshape = (shapeof*)malloc(sizeof(shapeof));
	int i;
	int idx = len+1;
	int checkif;
	char buffer[1];
	while(1)
	{
		buffer[0] = text[idx];
		checkif = check(buffer);
		if(checkif == 1)
		{
			newshape->newword = (char*)malloc(sizeof(char)*(idx+1));
			for(i=0;i<idx+1;i++)
			{
				newshape->newword[i] = text[i];
			}
			newshape->newword[idx+1] = '\0';
			newshape->len = idx+1;
			return newshape;
		}
		else
		{	
			text[idx] = '\0';
			idx--;
		}
	}
}

int main(void)
{
	int idx = 0;
	int i;
	int first = 1;
	int state = 1;
	int checkif;
	char buffer[1];
	char *text = (char*)malloc(sizeof(char)*50);
	Node* Head;
	while(read(0,buffer,1))
	{	
		shapeof *newshape;
		checkif = check(buffer);
		if(checkif)
		{
			if(state == 1 && checkif == 1)
			{	
				text[idx++] = buffer[0];
				state = 0;
			}
			else if(state == 0)
			{
				text[idx++] = buffer[0];
			}
		}
		else
		{
			if(text[0] == '\0');
			else
			{
			state = 1;
			newshape = shape(text,idx);
			if(first) Head = NewNode(newshape->newword,newshape->len);
			else insert(Head,newshape->newword,newshape->len);
			first = 0;
			free(newshape->newword);
			free(newshape);
			for(i=0;i<idx;i++) text[i] = '\0';
			idx = 0;
			}
		}
	}
	print_all(Head);
	return 0;
}
