#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include "util.h"

#define LIMIT 256	// 명령에 대한 최대 토큰 수
#define MAXLINE 1024 // 사용자 입력의 최대 문자 수

void init()
{
	
	GBSH_PID = getpid();

	GBSH_IS_INTERACTIVE = isatty(STDIN_FILENO);

	if (GBSH_IS_INTERACTIVE)
	{
	
		while (tcgetpgrp(STDIN_FILENO) != (GBSH_PGID = getpgrp()))
			kill(GBSH_PID, SIGTTIN);

		
		act_child.sa_handler = signalHandler_child;
		act_int.sa_handler = signalHandler_int;

		sigaction(SIGCHLD, &act_child, 0);
		sigaction(SIGINT, &act_int, 0);

		setpgid(GBSH_PID, GBSH_PID); 
		GBSH_PGID = getpgrp();
		if (GBSH_PID != GBSH_PGID)
		{
			printf("Error, the shell is not process group leader");
			exit(EXIT_FAILURE);
		}
		// 터미널 제어
		tcsetpgrp(STDIN_FILENO, GBSH_PGID);

		// 쉘의 기본 터미널 속성 저장
		tcgetattr(STDIN_FILENO, &GBSH_TMODES);

		currentDirectory = (char *)calloc(1024, sizeof(char));
	}
	else
	{
		printf("Could not make the shell interactive.\n");
		exit(EXIT_FAILURE);
	}
}

/**
 * 쉘 시작시 생성되는 코멘트.
 */
void welcomeScreen()
{
	printf("\n\t============================================\n");
	printf("\t                 Welcome!\n");
	printf("\n");
	printf("\t               SungHo Shell\n");
	printf("\t============================================\n");
	printf("\n\n");
}

/**
 * 시그널 핸들러
 */

/**
 * SIGCHLD를 위한 시그널 핸들러
 */
void signalHandler_child(int p)
{
	
	while (waitpid(-1, NULL, WNOHANG) > 0)
	{
	}
	printf("\n");
}

/**
 * SIGINT를 위한 시그널 핸들러
 */
void signalHandler_int(int p)
{
	if (kill(pid, SIGTERM) == 0)
	{
		printf("\n프로세스 %d 는 SIGINT 시그널을 받았습니다.\n", pid);
		no_reprint_prmpt = 1;
	}
	else
	{
		printf("\n");
	}
}

/**
 *	"<user>@<host> <cwd>> 형식으로 프롬프트를 출력"
 */
void shellPrompt()
{

	char hostn[1204] = "";
	gethostname(hostn, sizeof(hostn));
	printf("%s@%s %s > ", getenv("LOGNAME"), hostn, getcwd(currentDirectory, 1024));
}

/**
 * 디렉토리 변경
 */
int changeDirectory(char *args[])
{

	if (args[1] == NULL)
	{
		chdir(getenv("HOME"));
		return 1;
	}

	else
	{
		if (chdir(args[1]) == -1)
		{
			printf(" %s: 해당 디렉토리가 존재하지 않음.\n", args[1]);
			return -1;
		}
	}
	return 0;
}

/**
* 프로그램을 시작하는 방법. 백그라운드에서 계속 실행됨.
*/
void launchProg(char **args, int background)
{
	int err = -1;

	if ((pid = fork()) == -1)
	{
		printf("자식 프로세스를 만들 수 없습니다.\n");
		return;
	}

	if (pid == 0)
	{
		signal(SIGINT, SIG_IGN);

		setenv("parent", getcwd(currentDirectory, 1024), 1);

		// 존재하지 않는 명령을 실행시, 프로세스 종료.
		if (execvp(args[0], args) == err)
		{
			printf(" 해당 명령을 찾을 수 없습니다.");
			kill(getpid(), SIGTERM);
		}
	}

	if (background == 0)
	{
		waitpid(pid, NULL, 0);
	}
	else
	{
		printf("Process created with PID: %d\n", pid);
	}
}

/**
* 입출력 리디렉션 관리.
*/
void fileIO(char *args[], char *inputFile, char *outputFile, int option)
{

	int err = -1;

	int fileDescriptor; 
	// 출력 또는 입력 파일을 설명 (0~19)

	if ((pid = fork()) == -1)
	{
		printf("자식 프로세스를 만들 수 없습니다.\n");
		return;
	}
	if (pid == 0)
	{
		// 옵션 0: 출력 리디렉션
		if (option == 0)
		{
			// 쓰기 전용으로 열기
			fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
			dup2(fileDescriptor, STDOUT_FILENO);
			close(fileDescriptor);
			// Option 1: 입력 및 출력 리디렉션
		}
		else if (option == 1)
		{
			// 읽기 전용으로 열기
			fileDescriptor = open(inputFile, O_RDONLY, 0600);
			dup2(fileDescriptor, STDIN_FILENO);
			close(fileDescriptor);

			fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
			dup2(fileDescriptor, STDOUT_FILENO);
			close(fileDescriptor);
		}

		setenv("parent", getcwd(currentDirectory, 1024), 1);

		if (execvp(args[0], args) == err)
		{
			printf("err");
			kill(getpid(), SIGTERM);
		}
	}
	waitpid(pid, NULL, 0);
}

/**
* 파이프 사용.
*/
void pipeHandler(char *args[])
{
	
	int filedes[2]; 
	int filedes2[2];

	int num_cmds = 0;

	char *command[256];

	pid_t pid;

	int err = -1;
	int end = 0;

	// 다른 루프에 사용되는 변수
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;

	// 명령 수를 계산.
	while (args[l] != NULL)
	{
		if (strcmp(args[l], "|") == 0)
		{
			num_cmds++;
		}
		l++;
	}
	num_cmds++;

	
	while (args[j] != NULL && end != 1)
	{
		k = 0;
		
		while (strcmp(args[j], "|") != 0)
		{
			command[k] = args[j];
			j++;
			if (args[j] == NULL)
			{
				end = 1;
				k++;
				break;
			}
			k++;
		}
		
		command[k] = NULL;
		j++;

		if (i % 2 != 0)
		{
			pipe(filedes); 
		}
		else
		{
			pipe(filedes2); 
		}

		pid = fork();

		if (pid == -1)
		{
			if (i != num_cmds - 1)
			{
				if (i % 2 != 0)
				{
					close(filedes[1]); 
				}
				else
				{
					close(filedes2[1]); 
				}
			}
			printf("자식 프로세스를 만들 수 없습니다.\n");
			return;
		}
		if (pid == 0)
		{
			
			if (i == 0)
			{
				dup2(filedes2[1], STDOUT_FILENO);
			}

			else if (i == num_cmds - 1)
			{
				if (num_cmds % 2 != 0)
				{ 
					dup2(filedes[0], STDIN_FILENO);
				}
				else
				{ 
					dup2(filedes2[0], STDIN_FILENO);
				}
			}
			else
			{ 
				if (i % 2 != 0)
				{
					dup2(filedes2[0], STDIN_FILENO);
					dup2(filedes[1], STDOUT_FILENO);
				}
				else
				{ 
					dup2(filedes[0], STDIN_FILENO);
					dup2(filedes2[1], STDOUT_FILENO);
				}
			}

			if (execvp(command[0], command) == err)
			{
				kill(getpid(), SIGTERM);
			}
		}

		
		if (i == 0)
		{
			close(filedes2[1]);
		}
		else if (i == num_cmds - 1)
		{
			if (num_cmds % 2 != 0)
			{
				close(filedes[0]);
			}
			else
			{
				close(filedes2[0]);
			}
		}
		else
		{
			if (i % 2 != 0)
			{
				close(filedes2[0]);
				close(filedes[1]);
			}
			else
			{
				close(filedes[0]);
				close(filedes2[1]);
			}
		}

		waitpid(pid, NULL, 0);

		i++;
	}
}

/**
* 표준입력을 통해 입력된 명령을 처리
*/
int commandHandler(char *args[])
{
	int i = 0;
	int j = 0;

	int fileDescriptor;
	int standardOut;

	int aux;
	int background = 0;

	char *args_aux[256];

	while (args[j] != NULL)
	{
		if ((strcmp(args[j], ">") == 0) || (strcmp(args[j], "<") == 0) || (strcmp(args[j], "&") == 0))
		{
			break;
		}
		args_aux[j] = args[j];
		j++;
	}

	// 'exit' 입력시, 쉘을 종료함.
	if (strcmp(args[0], "exit") == 0)
		exit(0);
	// 'pwd' 입력시, 현재 디렉토리를 출력함.
	else if (strcmp(args[0], "pwd") == 0)
	{
		if (args[j] != NULL)
		{

			if ((strcmp(args[j], ">") == 0) && (args[j + 1] != NULL))
			{
				fileDescriptor = open(args[j + 1], O_CREAT | O_TRUNC | O_WRONLY, 0600);

				standardOut = dup(STDOUT_FILENO);
				dup2(fileDescriptor, STDOUT_FILENO);
				close(fileDescriptor);
				printf("%s\n", getcwd(currentDirectory, 1024));
				dup2(standardOut, STDOUT_FILENO);
			}
		}
		else
		{
			printf("%s\n", getcwd(currentDirectory, 1024));
		}
	}
	// 'clear' 입력시, 화면을 지움.
	else if (strcmp(args[0], "clear") == 0)
		system("clear");
	// 'cd' 입력시, 디렉토리를 변경함.
	else if (strcmp(args[0], "cd") == 0)
		changeDirectory(args);

	else
	{

		// '&' 입력시, 명령을 백그라운드로 전환.
		while (args[i] != NULL && background == 0)
		{
			if (strcmp(args[i], "&") == 0)
			{
				background = 1;
				// '|' 입력시, 파이프 명령을 수행.
			}
			else if (strcmp(args[i], "|") == 0)
			{
				pipeHandler(args);
				return 1;
				// '<' 입력시, 입 출력 리디렉션 수행
			}
			else if (strcmp(args[i], "<") == 0)
			{
				aux = i + 1;
				if (args[aux] == NULL || args[aux + 1] == NULL || args[aux + 2] == NULL)
				{
					printf(" 입력 인수가 충분하지 않음.\n");
					return -1;
				}
				else
				{
					if (strcmp(args[aux + 1], ">") != 0)
					{
						printf("Usage: Expected '>' and found %s\n", args[aux + 1]);
						return -2;
					}
				}
				fileIO(args_aux, args[i + 1], args[i + 3], 1);
				return 1;
			}

			// '>' 입력시, 출력 리디렉션 수행
			else if (strcmp(args[i], ">") == 0)
			{
				if (args[i + 1] == NULL)
				{
					printf(" 입력 인수가 충분하지 않음.\n");
					return -1;
				}
				fileIO(args_aux, NULL, args[i + 1], 0);
				return 1;
			}
			i++;
		}
		args_aux[i] = NULL;
		launchProg(args_aux, background);
	}
	return 1;
}

/**
* 메인 쉘 부분
*/
int main(int argc, char *argv[], char **envp)
{
	char line[MAXLINE];  // 사용자 입력을 위한 버퍼
	char *tokens[LIMIT]; // 명령에서 다른 토큰에 대한 배열
	int numTokens;

	no_reprint_prmpt = 0; 
	pid = -10;			 

	// 초기화 방법과 시작 화면을 호출.
	init();
	welcomeScreen();

//	environ = envp;

//	setenv("shell", getcwd(currentDirectory, 1024), 1);

	while (TRUE)
	{
		if (no_reprint_prmpt == 0)
			shellPrompt();
		no_reprint_prmpt = 0;

		memset(line, '\0', MAXLINE);

		// 사용자 입력을 기다림.
		fgets(line, MAXLINE, stdin);

		// 아무것도 쓰지 않을 시, 루프가 실행.
		if ((tokens[0] = strtok(line, " \n\t")) == NULL)
			continue;

		numTokens = 1;
		while ((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL)
			numTokens++;

		commandHandler(tokens);
	}

	exit(0);
}