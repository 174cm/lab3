/*
title: System programing lab3 number 4
file name: typing_practice.c
breif: typing pracitce program
author: github_174cm
*/

#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#define PASSWORDSIZE 12

int main(void)
{
    int fd;
    time_t tStart, tEnd;
    char text[4][100] = {{"Sofware Development Agreement"}, {"Software Reseller Agreement"}, {"System Integration Agreement"}, {"Manufacturing Agreement"}};
    struct termios init_attr, new_attr;
    int randnum;
    fd = open(ttyname(fileno(stdin)), O_RDWR);
    tcgetattr(fd, &init_attr);
    new_attr = init_attr;
    new_attr.c_lflag &= ~ICANON;
    new_attr.c_lflag &= ~ECHO;
    new_attr.c_cc[VMIN] = 1;
    new_attr.c_cc[VTIME] = 0;

    while(1)
    {
        if (tcsetattr(fd, TCSANOW, &new_attr) != 0){
            fprintf(stderr, "터미널 속성을 설정할 수 없음.\n");
        }
        randnum = rand()%4;
        int nread, cnt = 0, errcnt = 0;
        char ch;
        printf("\n다음 문장을 그대로 입력하시오.\n%s\n", text[randnum]);
        time(&tStart);
        while ((nread=read(fd,&ch,1)) > 0 && ch != '\n'){
            if(ch == text[randnum][cnt++])
                write(fd, &ch, 1);
            else {
                write(fd, "*", 1);
                errcnt++;
            }
        }
        printf("\n\n타이핑 오류의 횟수는 %d회 입니다.\n", errcnt);
        time(&tEnd);
        printf("시간 %f초\n", difftime(tEnd,tStart));
        printf("타자 %f타\n", (cnt-errcnt)*60/difftime(tEnd,tStart));
        tcsetattr(fd, TCSANOW, &init_attr);
    }
    close(fd);
}