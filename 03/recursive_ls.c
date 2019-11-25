/*
title: System programing lab3 number 3
filename: recursive_ls.c
author: github_174cm
*/

#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>

void lsR_func(char *d_path);

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: file_dir dirname\n");
		exit(1);
	}
	lsR_func(argv[1]);
}

void lsR_func(char *argv)
{

DIR *pdir;
struct dirent *pde;
struct stat buf;
int count = 0;
char *temp[1024];
memset(temp, '\0', sizeof(temp));
memset(&pde, '\0', sizeof(pde));
memset(&buf, '\0', sizeof(buf));

printf(":%s\n", argv);
int i = 0;
	
if((pdir = opendir(argv)) < 0) {
	perror("opendir");
	exit(1);
	}
	
chdir(argv);
		while ((pde = readdir(pdir)) != NULL) {
			lstat(pde->d_name, &buf);
			if(S_ISDIR(buf.st_mode)){
				if(strcmp(pde->d_name, "..") && strcmp(pde->d_name, ".")){
					temp[count] = pde-> d_name;
					count++;
					}
				}
			printf("%20s ", pde->d_name);
			if (++i % 3 == 0) printf("\n");
		}
	for(i = 0; i < count; i++)
		lsR_func(temp[i]);
	printf("\n");

closedir(pdir);
chdir("..");
}

