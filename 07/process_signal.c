/*
title: System programing lab3 number 7 
file name: precess_signal.c
breif: Program that allows signal handlers to receive interrupt signals
author: github_174cm 
*/
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void handler(int signo);
void main()
{
    struct sigaction act;
    int i = 0;
    act.sa_handler = handler;
    sigfillset(&(act.sa_mask));
    sigaction(SIGINT, &act, NULL);
    printf("SIGINT on\n");

    while(60){
        sleep(1);
        printf("sleep for %d sec(s).\n", ++i);
    }
}

void handler(int signo)
{
    printf("handler: signo = %d\n", signo);
}
