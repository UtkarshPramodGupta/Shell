#include "shell.h"
#include <signal.h>
#include <stdio.h>

void handler(int a){
}

int main(int argc, char* argv[])
{
	signal(SIGSTOP, handler);
	signal(SIGKILL, handler);
  return shell(argc, argv);
}
