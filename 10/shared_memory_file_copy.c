/*
title: System programing lab3 number 10
file name: shared_memory_file_copy.c
breif: Program to copy to another file using shared memory
author: github_174cm
*/

#include <sys/types.h> // include types infomation
#include <sys/stat.h> // use mode flag, open option
#include <sys/mman.h> // use shm_open 
#include <sys/wait.h> // use wait
#include <sys/ipc.h> // interprocess communication access structure
#include <sys/sem.h>  // semaphore
#include <sys/shm.h> // shared memory
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <fcntl.h> //use mode flag, open option
#define SHMSIZE 1024
#define SHMNAME "/my_shm"
#define BUFSIZE 512
#define SEMKEY (key_t)0111


void p(int semid)
{
    struct sembuf pbuf;
    pbuf.sem_num = 0;
    pbuf.sem_op = -1;
    pbuf.sem_flg = SEM_UNDO;
    if (semop(semid, &pbuf, 1) == -1)
    {
        perror("semop failed");
        exit(1);
    }
}

void v(int semid)
{
    struct sembuf vbuf;
    vbuf.sem_num = 0;
    vbuf.sem_op = 1;
    vbuf.sem_flg = SEM_UNDO;
    if (semop(semid, &vbuf, 1) == -1)
    {
        perror("semop failed");
        exit(1);
    }
}

int main()
{
    union senum 
    //control semaphore
    {
        int value;
        struct semid_ds *buf;
        unsigned short int *array;
    } arg;
    int shmd, len;
    int fd;
    int total;
    char buffer[BUFSIZE];
    void *shmaddr;
    int filedes;
    int nread;
    int semid;
    pid_t pid;

    if ((semid = semget(SEMKEY, 1, IPC_CREAT | 0666)) == -1)
    {
        perror("semget failed");
        exit(1);
    }
    arg.value = 1;

    if (semctl(semid, 0, SETVAL, arg) == -1)
    {
        perror("semctl failed");
        exit(1);
    }
    pid = fork();

    if (pid == 0)
    //child process start
    {
        sleep(3);
        p(semid);

        if ((shmd = shm_open(SHMNAME, O_RDWR, 0666)) == -1)
        {
            perror("shm_open failed");
            exit(1);
        }

        if ((shmaddr = mmap(0, SHMSIZE, PROT_READ, MAP_SHARED, shmd, 0)) == MAP_FAILED)
        {
            perror("mmap failed");
            exit(1);
        }

        if (mlock(shmaddr, SHMSIZE) != 0)
        {
            perror("mlock failed");
            exit(1);
        }

        printf("0\n");
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

        if ((filedes = open("test2.txt", O_WRONLY | O_CREAT, mode)) == -1)
        {
            printf("error in opening another file \n");
            exit(1);
        }
        strcpy(buffer, (char *)shmaddr);
        write(filedes, buffer, strlen(buffer));

        if (munmap(shmaddr, SHMSIZE) == -1)
        {
            perror("munmap failed");
            exit(1);
        }
        close(shmd);

        if (shm_unlink(SHMNAME) == -1)
        {
            perror("shm_unlink failed");
            exit(1);
        }
        v(semid);
    }

    else if (pid > 0)
    // Parent process start
    {
        p(semid);
        sleep(6); //check semaphore

        if ((filedes = open("test.txt", O_RDONLY)) == -1)
        {
            printf("error in opening another file \n");
            exit(1);
        }

        while ((nread = read(filedes, buffer, BUFSIZE)) > 0)
        {
            total += nread;
            buffer[nread - 1] = '\0';
        }

        if ((shmd = shm_open(SHMNAME, O_CREAT | O_RDWR, 0666)) == -1)
        {
            perror("shm_open failed");
            exit(1);
        }

        if (ftruncate(shmd, SHMSIZE) != 0)
        {
            perror("ftruncate failed");
            exit(1);
        }

        if ((shmaddr = mmap(0, SHMSIZE, PROT_WRITE, MAP_SHARED, shmd, 0)) == MAP_FAILED)
        {
            perror("mmap failed");
            exit(1);
        }

        if (mlock(shmaddr, SHMSIZE) != 0)
        {
            perror("mlock failed");
            exit(1);
        }

        strcpy((char *)shmaddr, buffer);
        if (munmap(shmaddr, SHMSIZE) == -1)
        {
            perror("munmap failed");
            exit(1);
        }

        printf("%s", buffer);
        close(shmd);
        v(semid);
        wait((int *) 0);
    }

    else
        printf("process create failed\n");
}


