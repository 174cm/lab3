#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <memory.h>

int my_shell(char *command){
    int status;

        switch(fork()){
            case 0:
                execl("/bin/bash", "bin/bash", "-c", command, NULL);
                break;
            case 1:
                perror("fork");
                exit(0);
                break;

            default:
                wait(&status);
                break;
        }
        return status;
}

void main(int argc, char** argv){
    int i;
    char temp[100];
    for (i = 0; i < 100; i++)
    {
        temp[i] = '\0';
    }
    memcpy(temp, argv[1], sizeof(argv[1]));
    for (i = 2; i < argc; i++){
        strcat(temp, " ");
        strcat(temp, argv[i]);
    }
    my_shell(temp);
}