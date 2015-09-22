#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>

#define FALSE 0
#define TRUE 1
#define INPUT_STRING_SIZE 80

#include "io.h"
#include "parse.h"
#include "process.h"
#include "shell.h"

process* first_process = NULL; //pointer to the first process that is launched */
process* last_process = NULL; //pointer to the first process that is launched */


int cmd_help(tok_t arg[]);
int cmd_quit(tok_t arg[]);
int cmd_cd(tok_t arg[]);


/* Command Lookup table */
typedef int cmd_fun_t (tok_t args[]); /* cmd functions take token array and return int */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_quit, "quit", "quit the command shell"},
  {cmd_cd, "cd", "change the current working directory"},
};

int cmd_help(tok_t arg[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    printf("%s - %s\n",cmd_table[i].cmd, cmd_table[i].doc);
  }
  return 1;
}

int cmd_quit(tok_t arg[]) {
  printf("Bye\n");
  exit(0);
  return 1;
}

int cmd_cd(tok_t arg[]) {
	return chdir(arg[0]);
}

int lookup(char cmd[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0)) return i;
  }
  return -1;
}

void init_shell()
{
  /* Check if we are running interactively */
  shell_terminal = STDIN_FILENO;

  /** Note that we cannot take control of the terminal if the shell
      is not interactive */
  shell_is_interactive = isatty(shell_terminal);

  if(shell_is_interactive){

    /* force into foreground */
    while(tcgetpgrp (shell_terminal) != (shell_pgid = getpgrp()))
      kill( - shell_pgid, SIGTTIN);

    shell_pgid = getpid();
    /* Put shell in its own process group */
    if(setpgid(shell_pgid, shell_pgid) < 0){
      perror("Couldn't put the shell in its own process group");
      exit(1);
    }

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);
    tcgetattr(shell_terminal, &shell_tmodes);
  }
  /** YOUR CODE HERE */
}

/**
 * Add a process to our process list
 */
void add_process(process* p)
{
	if(first_process==NULL)
	{
		first_process = p;
	}
	else
	{
		last_process->next = p;
		p->prev = last_process;
	}
	last_process = p;
}

/**
 * Creates a process given the inputString from stdin
 */
process* create_process(tok_t* t)
{
	process* proc = malloc(sizeof(process));

	int i, argc = 0, k = 1;

	proc->next = NULL;
	proc->prev = NULL;
	proc->std_in = 0;
	proc->std_out = 1;
	proc->std_err = 2;
	
	while(t[k] != NULL)
	{
		if(strcmp(t[k], "<") == 0)
		{
			proc->std_in = fileno(fopen(t[k + 1], "r"));
			argc = argc == 0 ? k : argc;
		}
		if(strcmp(t[k], ">") == 0)
		{
			proc->std_out = fileno(fopen(t[k + 1], "a"));
			argc = argc == 0 ? k : argc;
		}
		k++;
	}
	argc = argc == 0 ? k : argc;

	char** argv = (char**) malloc((argc+1)*sizeof(char*));

	for(i = 0; i < argc; i++)
	{
		argv[i] = t[i];
	}
	argv[argc] = NULL;

	proc->argc = argc;
	proc->argv = argv;

	add_process(proc);
  
	return proc;
}

int shell (int argc, char *argv[]) {
  char *s = malloc(INPUT_STRING_SIZE+1);			/* user input string */
  tok_t *t;			/* tokens parsed from input */
  int lineNum = 0;
  int fundex = -1;
  char* cwd = NULL;
  pid_t pid = getpid();		/* get current processes PID */
  pid_t ppid = getppid();	/* get parents PID */
  pid_t cpid, tcpid, cpgid;
  int stat = 50;
  process* proc;

  init_shell();


  printf("%s running as PID %d under %d\n",argv[0],pid,ppid);

  lineNum=0;
  cwd = getcwd(NULL, 50);

  fprintf(stdout, "%d:%s: ", lineNum, cwd);

  while ((s = freadln(stdin))){
    t = getToks(s); /* break the line into tokens */
    fundex = lookup(t[0]); /* Is first token a shell literal */
    if(fundex >= 0) cmd_table[fundex].fun(&t[1]);
    else 
    {
	   	proc = create_process(t);
		launch_process(proc);
		stat = proc->status;
    }

    cwd = getcwd(NULL, 50);
    fprintf(stdout, "%d:%d:%s: ", lineNum, stat, cwd);
  }
  return 0;
}
