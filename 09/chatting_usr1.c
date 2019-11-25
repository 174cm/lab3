/*
title: System programing lab3 number 9
file name: chatting_usr1.c
brief: Simple chat program using message queue, user1.
author: github_174cm
*/
#include <sys/types.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#define BUFSIZE 500
#define QNAME "/my_queue"
#define QNAME2 "/my_queue2"
#define PRIORITY 1

char send_data[BUFSIZE];
char recv_data[BUFSIZE];
mqd_t qd, qd2;

void *send(void *arg)
{
    while(1){
        scanf("%s", send_data);
        fflush(stdin);
        sleep(1);

        if (mq_send(qd, send_data, strlen(send_data), PRIORITY) == -1) {
            perror ("mq_send failed");
            exit(1);
        }
        printf("%s send\n", send_data);
    }
}

void *receive(void *arg)
{
    while(1) {
        if (mq_receive(qd2, recv_data, BUFSIZE, 0) == -1) {
                perror ("mq_send failed");
                exit(1);
        }
        printf("%s receive\n", recv_data);

        for(int i = 0; i < BUFSIZE; i++)
        {
            recv_data[i] = '\0';
        }
    }
}

int main()
{
    struct mq_attr q_attr;
    pthread_t st, rt;
    q_attr.mq_maxmsg = 5;
    q_attr.mq_msgsize = BUFSIZE;
    if ((qd = mq_open(QNAME, O_CREAT | O_RDWR, 0600, & q_attr)) == -1 ) {
        perror("mq_open failed");
        exit(1);
    }

    if((qd2 = mq_open(QNAME2, O_CREAT|O_RDWR,0600,& q_attr )) == -1) {
        perror("mq_open failed");
        exit(1);
    }
    pthread_create(&st,NULL,send,NULL);
    pthread_create(&rt,NULL,receive,NULL);

    pthread_join(st,NULL);
    pthread_join(rt,NULL);
}