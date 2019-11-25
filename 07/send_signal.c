/*
title: System programing lab3 number 7 
file name: send_signal.c
author: github_174cm 
*/
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>

int i = 0;
void p_handler (int), c_handler (int);

void main(int argc, char** argv)
{
    pid_t pid, ppid;
    struct sigaction act;
    pid = fork();
    
    if (pid ==0){
        act.sa_handler = c_handler;
        sigaction (SIGUSR1, &act, NULL);
        ppid = getppid();
        
        while(1) {
            sleep(1);
            kill (ppid, SIGUSR1);
            pause();
        }
    }
    else if (pid > 0) {
        act.sa_handler = p_handler;
        sigaction(SIGUSR1, &act, NULL);
        
        while(1) {
            pause();
            sleep(1);
            kill (pid, SIGUSR1);
        }
    }
    else perror ("Error");
}
void p_handler(int singno)
{
    printf("Parent handler: call %d times.\n", ++i);
}

void c_handler(int signo)
{
    printf("Child handler: call %d times.\n", ++i);
}
