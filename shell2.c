/*執行方法：
./myShell
>> ls -R /
ctr-c
*/
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <setjmp.h>
#include <sys/resource.h> 
/*
全域變數，放解析過後的使用者指令（字串陣列）
*/
char* argVect[200];
char* cmd;
//印出提示訊息
void printSigMask(){
    sigset_t oldsigset;
    sigprocmask(SIG_SETMASK,NULL,&oldsigset);
    for(int i=0;i<SIGRTMAX;i++){
        if(sigismember(&oldsigset,i)==1){
            printf("Signal ""%s"" is blocked\n",sys_siglist[i]);
        }
    }  
}
double getCurTime(){
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC,&now);
    double sec=now.tv_sec;
    double nano_sec=now.tv_nsec;
    return sec+nano_sec*10E-9;
}
/*
parseString：將使用者傳進的命令轉換成字串陣列
str：使用者傳進的命令
cmd：回傳執行檔
*/
void parseString(char* str, char** cmd) {
    int idx=0;
    char* retPtr;
    //printf("%s\n", str);
    retPtr=strtok(str, " \n");
    while(retPtr != NULL) {
        //printf("token =%s\n", retPtr);
        //if(strlen(retPtr)==0) continue;
        argVect[idx++] = retPtr;
        if (idx==1)
            *cmd = retPtr;
        retPtr=strtok(NULL, " \n");
    }
    argVect[idx]=NULL;
}
void ungets_ctr_c() {
	ungetc('\n', stdin);
	ungetc('c', stdin);
	ungetc('^', stdin);
}
sigjmp_buf jmpbuf;
int hasChild ;
pid_t childPid;
/*signal handler專門用來處理ctr-c*/
void ctr_c(int signum) {
    printf("\nctr-c:");
    printSigMask();
    printf("\n end of the report of sigmask\n");
    if (hasChild) {
        kill(childPid, SIGINT);
        hasChild = 0;
    } 
    else {
        /*確認main function並不是剛好在處理字串，這裡使用一個隱含的同步方法*/
        /*藉由確認是否argVect[0]（即執行檔）是否為NULL保證main function不是在處理字串*/
        /*主程式的控制迴圈必須在一開始的地方將argVect[0]設為NULL*/
        if (argVect[0] == NULL) {
            ungets_ctr_c();
            siglongjmp(jmpbuf, 1);
        } 
    }
}
int main (int argc, char** argv) {
    char* exeName;
    pid_t pid;
    char cmdLine[256];
    /*底下程式碼註冊signal的處理方式*/
    signal(SIGINT, ctr_c);
    /*無窮迴圈直到使用者輸入exit*/
    while(1) {
        //設定化hasChild, argVect[0]，避免發生race condtion
        hasChild = 0;
        argVect[0]=NULL;
        printSigMask();
        printf("myshell >");
        //設定返回地點，如果使用者按下ctr-c會從sigsetjmp的下一行開始執行
        sigsetjmp(jmpbuf, 1);
        /*
        接收使用者命令，除了cd, exit以外，其他指令呼叫對應的執行檔
        */
        fgets(cmdLine, 256, stdin);
        printf("the return of gets if %s\n",cmdLine);
        parseString(cmdLine,&exeName);
        if (strcmp(cmdLine, "^c") == 0) {   //使用者按下control-c，^c是由signal handler放入
            //printf("ctr-c \n");
            printf("\n");
            continue;
        }
        pid = fork();   //除了exit, cd，其餘為外部指令
        if (pid>0)
        {
            /*
            parent(myShell)等待child完成命令
            完成命令後，parent將child的結束回傳值印出來
            */
            int tmp;
            childPid = pid;/*通知singal handler，如果使用者按下ctr-c時，要處理這個child*/
            hasChild = 1;/*通知singal handler，正在處理child*/
            wait(&tmp);
        }
        else {
            /*
            產生一個child執行使用者的指令
            */
            printf("child execute the command %s\n",cmdLine);
            if (execvp(exeName, argVect)==-1) {
                perror("myShell");
                exit(errno*-1);
            }
        } 
    }
}