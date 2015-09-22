#include "process.h"
#include "shell.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>

/**
 * Executes the process p.
 * If the shell is in interactive mode and the process is a foreground process,
 * then p should take control of the terminal.
 */

void launch_process(process* proc){

	int pid = fork();

	if(pid == 0)
	{
		tok_t* path;
		extern char** environ;
  		path = getToks(environ[9]);
		int no_of_paths = 1, i = 1;

		while(path[no_of_paths] != NULL)
		{
			no_of_paths++;
		}
		no_of_paths--;
		
		dup2(proc->std_in, 0);
		dup2(proc->std_out, 1);
		
		char p[500];
		
		do
		{
			strcpy(p, path[i]);
			strcat(p, "/");
			strcat(p, proc->argv[0]);
			i++;
		}
		while(fopen(p, "r") == NULL && i <= no_of_paths);


		if(i > no_of_paths)
		{
			execv(proc->argv[0], proc->argv);
		}
		else
		{
			execv(p, proc->argv);
		}

		exit(20);
	}
	else if(pid > 0)
	{
		proc->pid = pid;
		waitpid(pid, &(proc->status), 0);
	}
	else
	{
		printf("Could not create process :(");
	}
}

