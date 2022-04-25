#include "lsh.h"
#include "pwdStack.h"
#include "system.h"
#include <stdio.h>
#include <string.h>

#define MAX_LINE_LENGTH 256
#define MAX_ARG_COUNT 32

extern void Append(char *unixFile, char *nachosFile, int half);
extern void NAppend(char *nachosFileFrom, char *nachosFileTo);

LibreShell::LibreShell(){
    pwdStack = new PwdStack();
    //把根目录压入工作路径栈
    pwdStack->push(PwdEntry("", 1));
}

PwdStack *LibreShell::getPwdStack()
{
    return pwdStack;
}

void alert(char* raiser,char* msg){
    printf("%s: %s\n",raiser,msg);
}

void getPwdStr(char *res){
    PwdStack *pwdStack = fileSystem->getShell()->getPwdStack();
    PwdStack oldStack=*pwdStack;
    PwdStack stack;
    char tmp[MAX_LINE_LENGTH];
    memset(tmp, 0, sizeof(tmp));

    while (!oldStack.isEmpty())
    {
        stack.push(oldStack.top());
        oldStack.pop();
    }
    
    //逆向路径
    while (!stack.isEmpty())
    {
        // printf("/%s\n",stack.top().name);
        strcat(tmp, stack.top().name);
        strcat(tmp, "/");
        stack.pop();
    }

    //处理烫烫烫
    int ind = 0;
    for(;ind<=strlen(tmp);ind++)
        if(tmp[ind]=='/')break;
    strcpy(res,tmp+ind);
}

void cmd_cd(char* path){
    fileSystem->changeDir(path);
    //  if (!fileSystem->changeDir(path))
    //     alert("cd", "path not found");
}

void cmd_pwd(){
    char pwdStr[MAX_LINE_LENGTH];
    // PwdStack *pwdStack = fileSystem->getShell()->getPwdStack();
    getPwdStr(pwdStr);
    printf("%s\n",pwdStr);
}

void cmd_new(char* name){
    fileSystem->Create(name, 122);
}

void cmd_new_with_content(char *name,char* cont){
    fileSystem->Create(name, strlen(cont)*sizeof(char));
}

void cmd_mkdir(char* name){
    fileSystem->makeDir(name);
}

void cmd_ls(){
    fileSystem->List();
}

void cmd_ap(char *unixFile, char *nachosFile)
{
    Append(unixFile,nachosFile,false);
}

void cmd_nap(char *nachosFileFrom, char *nachosFileTo)
{
    NAppend(nachosFileFrom, nachosFileTo);
}

void cmd_ls_la(){
    fileSystem->List_Verbose();
}

void cmd_disk(){
    fileSystem->Print();
}

void cmd_rm(char* name){
    fileSystem->Remove(name);
}

void cmd_cat(char* fileName)
{
    fileSystem->Cat(fileName);
}



void startUpWelcome(){
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("=======================\n");
    printf("\n");
    printf("  Welcome to Nachos!\n");
    printf("\n");
    printf("=======================\n");
    printf("\n\n\n");
}

void LibreShell::startShell(){
    char * input=NULL;
    size_t maxLen = MAX_LINE_LENGTH - 1;
    int argsCnt=0;
    char args[MAX_ARG_COUNT][MAX_LINE_LENGTH];
    char pwdStr[MAX_LINE_LENGTH];
    startUpWelcome();
    while (true)
    {
        memset(pwdStr, 0, sizeof(pwdStr));
        getPwdStr(pwdStr);
        // printf("|Nachos Shell > linton@nachos:%s$ ",pwdStr);
        printf("|Nachos Shell > %s$ ", pwdStr);
        fflush(stdout);
        getline(&input,&maxLen,stdin);

        //以空格分割参数，存到二维数组中
        for (int i = 0, old_i = -1; i <= strlen(input); i++){
            //双指针
            if (input[i] == ' ' || i == strlen(input))
            {
                // char nowArg[MAX_LINE_LENGTH];
                // strncpy(args[argsCnt - 1], old_i == -1 ? input + old_i + 1 : input + old_i + 2, i == strlen(input) ? i - old_i - 3 : (old_i == -1 ? i - old_i - 1 : i - old_i - 2));
                argsCnt++;
                if (old_i == -1)
                    strncpy(args[argsCnt - 1], input + old_i + 1, i == strlen(input) ? i - old_i - 2 :  i - old_i - 1);
                else
                    strncpy(args[argsCnt - 1], input + old_i + 2, i == strlen(input) ? i - old_i - 3 :  i - old_i - 2);
                
                old_i=i-1;
            }
        }

        if(strcmp(args[0],"cd")==0)
        {
            cmd_cd(args[1]);
        }
        else if (strcmp(args[0], "rm") == 0)
        {
            cmd_rm(args[1]);
        }
        else if (strcmp(args[0], "quit")==0)
        {
            printf("bye!\n");
            break;
        }
        else if (strcmp(args[0], "pwd") == 0)
        {
            cmd_pwd();
        }
        else if (strcmp(args[0], "ls") == 0)
        {
            if (argsCnt == 2 && strcmp(args[1], "-la") == 0)
                cmd_ls_la();
            else 
                cmd_ls();
        }
        else if (strcmp(args[0], "cat") == 0)
        {
            if (argsCnt == 2)
                cmd_cat(args[1]);
            else
                alert("cat", "invalid operation.");
        }
        else if (strcmp(args[0], "ap") == 0)
        {
            if (argsCnt == 3)
                cmd_ap(args[1],args[2]);
            else
                alert("ap","invalid operation.");
        }
        else if (strcmp(args[0], "nap") == 0)
        {
            if (argsCnt == 3)
                cmd_nap(args[1], args[2]);
            else
                alert("nap", "invalid operation.");
        }
        else if (strcmp(args[0], "disk") == 0)
        {
            cmd_disk();
        }
        else if (strcmp(args[0], "mkdir") == 0)
        {
            cmd_mkdir(args[1]);
        }
        else if (strcmp(args[0], "touch") == 0)
        {
            // if (argsCnt == 3)
            //     cmd_new_with_content(args[1], args[2]);
            // else 
            if (argsCnt == 2)
                cmd_new(args[1]);
            else
                alert("ap", "invalid operation.");   
        }
        else
        {
            printf("%s: command not found.\n",args[0]);
        }

        // for(int i=0;i<argsCnt;i++)
        //     printf("%d:%s\n", i,args[i]);
        // printf("args count:%d\n", argsCnt);

        memset(input, 0, sizeof(input));
        for (int i = 0; i < argsCnt; i++)
            memset(args[i], 0, sizeof(args[i]));

        argsCnt=0;
    }
    
}
