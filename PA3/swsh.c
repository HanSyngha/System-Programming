//-----------------------------------------------------------
//
// SSE2033 : System Software Experiment 2 (Fall 2019)
//
// Skeleton Code for PA#3
//
// Oct 28, 2019.
// CSI, Sungkyunkwan University
//
// Forked by http://csapp.cs.cmu.edu/public/ics2/code/ecf/shellex.c
//-----------------------------------------------------------


/* $begin shellmain */
#define MAXARGS   128
#define MAXLINE	  256
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 
void swsh_head(char **argv);
void swsh_tail(char **argv);
void swsh_cat(char **argv);
void swsh_cp(char **argv);
void swsh_mv(char **argv);
void swsh_rm(char **argv);
void swsh_cd(char **argv);
void swsh_pwd();
void swsh_exit(char **argv);
void close_pipe();

/* global variables */
int pipes[10][2];
pid_t pgid = 0;

void close_pipe()
{
	int idx;
	for(idx=0;idx<10;idx++)
	{
		close(pipes[idx][0]);
		close(pipes[idx][1]);
	}
}

int main() 
{ 
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    char cmdline[MAXLINE]; /* Command line */
	char* ret;
    while (1) {
	/* Read */
	printf("swsh> ");                   
	ret = fgets(cmdline, MAXLINE, stdin); 
	if (feof(stdin) || ret == NULL)
	    exit(0);

	/* Evaluate */
	eval(cmdline);
    } 
}
/* $end shellmain */
  
/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; /* Argument list execve() */
	int pipe_argv[MAXARGS];
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    int argc = 0;
	int idx,out_ptr = 0;
	int in_ptr =0;
	int is_in = 0;
	int is_out = 0;
	int pipe_wr = 0;
	int pipe_num = 0;
	int remain_pipe;
	int status;
	int is_child = 0;
    pid_t pid = 0;           /* Process id */

	for(idx=0;idx<10;idx++)
	{
		if(pipe(pipes[idx]) <0){
				perror("pipe error");
				exit(-1);
		}
	}
    strcpy(buf, cmdline);
    bg = parseline(buf, argv); 
    if (argv[0] == NULL)  
	return;   /* Ignore empty lines */
	else{}
	pipe_argv[pipe_wr++] = 0;
	while(argv[argc] != NULL)
	{
		if(strcmp("|",argv[argc]) == 0)
		{
			pipe_argv[pipe_wr++] = argc+1;
		}
		argc++;
	}
	pipe_argv[pipe_wr] = argc+1;
	pipe_num = pipe_wr;
	remain_pipe = pipe_num;
    if (!builtin_command(argv)) { 
	if(pipe_num != 1)
	{	
		while(remain_pipe != 0)
		{
			pgid = getpgrp();
			if ((pid = fork()) == 0) 
			{
				is_child = 1;
				setpgid(getpid(),pgid);
				remain_pipe--;
			}
			else
			{
				if(is_child == 0) close_pipe();
				break;
			}				
		}
		if(remain_pipe == 0)
		{
			dup2(pipes[0][1],1);
			close_pipe();
		}
		if(remain_pipe == pipe_num-1)
		{
			dup2(pipes[pipe_num-2][0],0);
			close_pipe();
		}
		for(idx=1;idx<pipe_num-1;idx++)
		{
			if(idx == remain_pipe)
			{
				dup2(pipes[idx][1],1);
				dup2(pipes[idx-1][0],0);
				close_pipe();
			}
		}
		if(pid != 0 && is_child) 
		{
			if (waitpid(pid, &status, WUNTRACED) < 0) printf("waitfg: waitpid error");
			if(WIFSTOPPED(status)) kill(-1,SIGKILL);
		}
	}
	if(pipe_num == 1)
	{
		pgid = getpgrp();
		if ((pid = fork()) == 0)
		{
			setpgid(getpid(),pgid);
			is_child = 1;
		}
	}
	int wr = 0;
	if (is_child) {   /* Child runs user job */
		if(pipe_num != 1)
		{
			for(idx=pipe_argv[remain_pipe];idx<pipe_argv[remain_pipe+1]-1;idx++) argv[wr++] = argv[idx];
			for(idx=wr;idx<argc;idx++) argv[idx] = NULL;
			argc = wr;
		}
		idx = 0;
		while(argv[idx] != NULL)
		{
			if(strcmp("<",argv[idx]) == 0)
			{
				is_in = 1;
				in_ptr = idx;
			}
			if(strcmp(">",argv[idx]) == 0)
			{
				is_out = 1;
				out_ptr = idx;
			}
			if(strcmp(">>",argv[idx]) == 0)
			{
				is_out = 2;
				out_ptr = idx;
			}
			idx++;
		}
		//redirection
		if(argc>1){
			//input "<"
			if(is_in)
			{
				int redirection_input = open(argv[in_ptr+1],O_RDWR,0775);
				if(redirection_input == -1)
				{
					perror("swsh");
					exit(errno);
				}
				dup2(redirection_input,0);
				for(idx=in_ptr;idx<argc;idx++)
				{
					argv[idx] = argv[idx+2];
				}
			}
			//output ">", ">>"
			if(is_out)
			{
				int redirection_output = open(argv[out_ptr+1],O_CREAT|O_RDWR,0775);
				if(is_out == 2) lseek(redirection_output,0,SEEK_END);
				dup2(redirection_output,1);
				for(idx=out_ptr;idx<argc;idx++)
				{
					argv[idx] = argv[idx+2];
				}
			}
		}
		//cmd_type1
		if(strcmp("ls",argv[0]) == 0)	execvp(argv[0],argv);
		else if(strcmp("man",argv[0]) == 0)	execvp(argv[0],argv);
		else if(strcmp("grep",argv[0]) == 0) execvp(argv[0],argv);
		else if(strcmp("sort",argv[0]) == 0) execvp(argv[0],argv);
		else if(strcmp("awk",argv[0]) == 0)	execvp(argv[0],argv);
		else if(strcmp("bc",argv[0]) == 0)	execvp(argv[0],argv);
		else if(strncmp("./",argv[0],2) == 0)	execv(argv[0],argv);
		//cmd_type2
		else if(strcmp("head",argv[0]) == 0)	swsh_head(argv);
		else if(strcmp("tail",argv[0]) == 0)	swsh_tail(argv);
		else if(strcmp("cat",argv[0]) == 0)	swsh_cat(argv);
		else if(strcmp("cp",argv[0]) == 0)	swsh_cp(argv);
		//cmd_type3
		else if(strcmp("mv",argv[0]) == 0)	swsh_mv(argv);
		else if(strcmp("rm",argv[0]) == 0)	swsh_rm(argv);
		//cmd_type4
		else if(strcmp("pwd",argv[0]) == 0)	swsh_pwd();
		else{
			fprintf(stderr,"%s: Command not found.\n", argv[0]);
			exit(0);
		}
		exit(0);
	}
	/* Parent waits for foreground job to terminate */
	if (!bg) {
	    if (waitpid(pid, &status, WUNTRACED) < 0)
			printf("waitfg: waitpid error");
	}
	else
	    printf("%d %s", pid, cmdline);
    }
    if(WIFSTOPPED(status)) kill(-1,SIGKILL);
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    	if (!strcmp(argv[0], "quit")) exit(0);  
	else if(strcmp("cd",argv[0]) == 0)
	{
		swsh_cd(argv);
		return 1;
	}
	else if(strcmp("exit",argv[0]) == 0)	swsh_exit(argv);
	else{}
   	if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
		return 1;
   	return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* Ignore spaces */
	       buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
	return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	argv[--argc] = NULL;

    return bg;
}
/* $end parseline */

void swsh_head(char **argv)
{
	int is_stdin = 0;
	int remain_lines = 10;
	int fd_head = 0;
	char buf[1];
	if(argv[1] != NULL && strcmp(argv[1],"-n") == 0)
	{
		remain_lines = atoi(argv[2]);
		if(argv[3] == NULL) is_stdin = 1;
		else fd_head = open(argv[3], O_RDONLY, 0644);
	}
	else
	{
		if(argv[1] == NULL) is_stdin = 1;
		else fd_head = open(argv[1], O_RDONLY, 0644);
	}
	if(is_stdin)
	{
		while(read(0,buf,1))
		{
			if(*buf == EOF) return;
			if(*buf == '\n') remain_lines--;
			if(remain_lines == 0)
			{
				if(write(1,"\n",1));
				break;
			}
			if(write(1,buf,1));
		}
	}
	else{
		while(remain_lines>0)
		{
			if(read(fd_head, buf, 1) <= 0) return;
			if(*buf == '\n') remain_lines--;
			if(write(1,buf,1));
		}
		close(fd_head);
	}
	return;
}

void swsh_tail(char **argv)
{
	int idx = 0;
	int remain_lines = 10;
	int total_word = 0;
	int *word_for_last_lines;
	int fd_head;
	int erase = 0;
	int is_stdin = 0;
	char buf[1];
	if(argv[1] != NULL && strcmp(argv[1],"-n") == 0)
	{
		word_for_last_lines = (int*)malloc(sizeof(int)*remain_lines);
		remain_lines = atoi(argv[2]);
		if(argv[3] == NULL) is_stdin = 1;
		else fd_head = open(argv[3], O_RDONLY, 0644);
	}
	else
	{
		word_for_last_lines = (int*)malloc(sizeof(int)*remain_lines);
		if(argv[1] == NULL) is_stdin = 1;
		else fd_head = open(argv[1], O_RDONLY, 0644);
	}
	if(is_stdin)
	{
		fd_head = open("temp.txt",O_CREAT|O_RDWR,0775);
		while(read(0,buf,1)) if(write(fd_head,buf,1));
		lseek(fd_head,0,SEEK_SET);
	}
	while(read(fd_head, buf, 1))
	{
		if(erase)
		{
			erase = 0;
			word_for_last_lines[idx] = 0;
		}
		word_for_last_lines[idx]++;
		if(*buf == '\n')
		{
			idx = (idx+1)%remain_lines;
			erase = 1;
		}
	}
	for(idx=0;idx<remain_lines;idx++) total_word = total_word - word_for_last_lines[idx];
	lseek(fd_head,total_word,SEEK_END);
	while(read(fd_head,buf,1)) if(write(1,buf,1));
	free(word_for_last_lines);
	close(fd_head);
	if(is_stdin)
	{
		if(fork() == 0) execl("/bin/rm","rm","temp.txt",NULL);
	}
	return;
}

void swsh_cat(char **argv)
{
	char buf[1];
	if(argv[1] == NULL)
	{
		while(1)
		{
			if(read(0,buf,1) == 0) break;
			if(*buf == EOF) return;
			if(write(1,buf,1));
		}
	}	
	else
	{
		int fd_head = open(argv[1], O_RDONLY, 0644);
		while(read(fd_head,buf,1)) if(write(1,buf,1));
		close(fd_head);
	}
}

void swsh_cp(char **argv)
{
	char buf[1];
	int fd_src = open(argv[1], O_RDONLY, 0644);
	int fd_dest = open(argv[2], O_CREAT|O_WRONLY, 0664);
	while(read(fd_src,buf,1)) if(write(fd_dest,buf,1));
	close(fd_src);
	close(fd_dest);
}

void swsh_mv(char **argv)
{
	char* src = (char*)malloc(sizeof(char)*(strlen(argv[1])+3));
	char* dest = (char*)malloc(sizeof(char)*(strlen(argv[2])+3));
	strcpy(src,"./");
	strcpy(dest,"./");
	strcat(src,argv[1]);
	strcat(dest,argv[2]);
	strcat(src,"\0");
	strcat(dest,"\0");
	int result = rename(src,dest);
	free(src);
	free(dest);
	if(result != 0) perror("mv");
	
}

void swsh_rm(char **argv)
{
	char* src = (char*)malloc(sizeof(char)*(strlen(argv[1])+3));
	strcpy(src,"./");
	strcat(src,argv[1]);
	strcat(src,"\0");
	int result = unlink(src);
	free(src);
	if(result != 0) perror("unlink");
}

void swsh_cd(char **argv)
{
	char* src = (char*)malloc(sizeof(char)*(strlen(argv[1])+3));
	strcpy(src,"./");
	strcat(src,argv[1]);
	strcat(src,"\0");
	int result = chdir(src);
	free(src);
	if(result != 0) perror("cd");
}

void swsh_pwd()
{
	char pwd_buf[1024];
	char* result = getcwd(pwd_buf,sizeof(pwd_buf));
	if(result == NULL) perror("pwd");
	else printf("%s\n",pwd_buf);
}

void swsh_exit(char **argv)
{
	int errorno = 0;
	if(argv[1] != NULL) errorno = atoi(argv[1]);
	if(write(2,"exit\n",5));
	exit(errorno);
}
//1
//2
//3
//4
//5
//6
//7
//8
//9
//10
