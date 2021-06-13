#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h> 
#include <string.h>


void type_prompt() {
	char hostname[256] = { '\0' };
	gethostname(hostname, sizeof(hostname));

	char curpath[256] = { '\0' };
	getcwd(curpath, 256);

	char prompt = '$';
	if (geteuid() == 0) {
		prompt = '#';
	}

	printf("%s@%s:%s%c", getpwuid(getuid())->pw_name, hostname, curpath, prompt);
}

char** read_command() {
	char cmd[256];
	fgets(cmd, 256, stdin);

	const char *delim = " ";
	int num = 0;
	char copy[256];
	strcpy(copy, cmd);
	char *p = strtok(copy, delim);
	while (p != NULL) {
		p = strtok(NULL, delim);
		num++;
	}
	
	char **argv = (char**)malloc((num+1) * sizeof(char*));
	for (int i = 0; i < num; i++) {
		argv[i] = (char*)malloc(256 * sizeof(char));
	}

	num = 0;
	strcpy(copy, cmd);
	p = strtok(copy, delim);
	while (p != NULL) {
		strcpy(argv[num], p);
		p = strtok(NULL, delim);
		num++;
	}
	argv[num] = (char *)0;

	return argv;
}

int main()
{
	
	while (1)
	{
		type_prompt();
		char* env[] = { "PATH=/bin",0 };
		char** argv= read_command();
		if (fork() != 0) {
			waitpid(-1, NULL, 0);
		}
		else {
			execve("/bin/ls", argv, env);
		}
	}
	return 0;
}
