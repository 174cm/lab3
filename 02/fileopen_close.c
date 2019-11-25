/*
title: System programing lab3 number 2
file name: fileopen_close.c
breif: Program to open and close files
author: github_174cm
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

void main(int argc, char *argv[])
{
	int fd;
	if(argc<2){
		fprintf(stderr, "Usage: file_check filename\n");
		exit(1);
	}	
	if((fd = open(argv[1], O_RDONLY)) == -1){
		perror("open");	
		exit(1);
	}
	printf("File \"%s\" found...\n",argv[1]);
	close(fd);
}

