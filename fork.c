#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
int main(){
    pid_t pid=fork();
    int status;
    switch (pid){
        case -1:
            perror("fork()");
            exit(-1);
        case 0:
            printf("I'm a Child process\n");
            printf("Child's PID is %d\n",getpid());
            printf("Enter a number:\n");
            scanf("%d",&status);
            int st=status*status;
            printf("Its square is ");
            printf("%d\n",st);
            break;
        default:
            wait(&status);
            sleep(3);
            printf("I'm a Parent process\n");
            printf("Parent's PID is %d\n",getpid());
    }
    return 0;
}