/*
 * main.cpp
 *
 *  Created on: 2016Äê5ÔÂ8ÈÕ
 *      Author: LeoBrilliant
 */


#include <netdb.h>
#include <errno.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "InitServer.h"

#define QLEN 10

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

//extern int initserver(int, struct sockaddr*, socklen_t, int);

void serve(int sockfd)
{
	int clfd, status;
	pid_t pid;
	while(true)
	{
		clfd = accept(sockfd, NULL, NULL);
		if(clfd < 0)
		{
			syslog(LOG_ERR, "ruptimed: accept error: %s", strerror(errno));
			exit(1);
		}

		if((pid = fork()) < 0)
		{
			syslog(LOG_ERR, "ruptimed: fork error: %s", strerror(errno));
			exit(1);
		}
		else if(pid == 0) //child
		{
			if(dup2(clfd, STDOUT_FILENO) != STDOUT_FILENO ||
					dup2(clfd, STDERR_FILENO) != STDERR_FILENO)
			{
				syslog(LOG_ERR, "ruptimed: unexpected error");
				exit(1);
			}
			close(clfd);
			execl("/usr/bin/date", "uptime", (char*)0);
			syslog(LOG_ERR, "ruptimed: unexpected return from exec: %s",
					strerror(errno));
		}
		else //parent
		{
			close(clfd);
			waitpid(pid, &status, 0);
		}
	}
}

#define SERVER_PORT 20000
int main()
{
	int sockfd;
	struct sockaddr_in server_addr;
	char server_ip[] = "192.168.31.134";

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	inet_aton(server_ip, &server_addr.sin_addr);
	server_addr.sin_port = htons(SERVER_PORT);

	if((sockfd = initserver(SOCK_STREAM, (sockaddr*)&server_addr, sizeof(server_addr), QLEN)) >= 0)
	{
		serve(sockfd);
		exit(0);
	}

	return 0;
}
