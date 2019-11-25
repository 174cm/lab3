/*
title: System programing lab3 number 5
file name: atexitprocess.c
breif: Using fork function program
author: github_174cm
*/

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void main()
{
	pid_t pid;
	printf("Calling fork \n");
	pid = fork();

	if (pid == 0)
		printf("child process call\n");
	else if (pid > 0)
		printf("parent process call\n");
	else
		printf("fork failed\n");
}
