#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct{
    char *name;
    struct tm time;
    int value;
}Command;

typedef struct{
    char *name;
    char *value;
}EnvVar;

Command **cmdsInLog;
EnvVar **envVariable;
int cmdNum = 0;
int varCapacity = 0;
int empty = 0;

char *getInput(){
    int pos = 0;
    int bufferSize = 1024;
    char input;
    char *buffer = malloc(1024 * sizeof (char));
    if(buffer == NULL){
        printf("Allocation failed\n");
        exit(0);
    }

    while(1){
        input = getchar();
        if(input == EOF){
            buffer[pos] = '\0';
            return buffer;
        }
        else if(input == '\n'){
            buffer[pos] = '\0';
            return buffer;
        }
        if(pos >= bufferSize){
            buffer = realloc(buffer, bufferSize);
        }
        buffer[pos] = input;
        pos++;
    }
}

void addToLog(Command *command){
    if(cmdNum == 0){
        cmdsInLog = malloc(100* sizeof(Command));
        if(cmdsInLog == NULL){
            printf("Allocation failed\n");
            exit(0);
        }
    }
    cmdsInLog[cmdNum++] = command;
}

int logInfo(){
    if(cmdNum == empty) {
        printf("Log does not have any command history\n");
        return 0;
    }
    else{
        int i = 0;
        while(i < cmdNum) {
            printf("%s",asctime(&cmdsInLog[i]->time));
            printf(" %s %d\n", cmdsInLog[i]->name, cmdsInLog[i]->value);
            i++;
        }
        return 0;
    }
}

void incorrectInput(){
    printf("Command '%s' not found\n", cmdsInLog[cmdNum-1]->name);
}

void quit(){
    printf("Bye!\n");
    printf("\033[0m");
    exit(0);
}

int addEVariable(EnvVar *variable) {
    if(varCapacity == empty){
        envVariable = malloc(100* sizeof(EnvVar));
        if(envVariable == NULL){
            printf("Allocation failed\n");
            quit();
        }
    }
    envVariable[varCapacity++] = variable;
    return 0;
}

int updateEValue(EnvVar *variable){
    int result = 1;
    int i = 0;
    while(i < varCapacity) {
        if(strcmp(envVariable[i]->name, variable->name) == 0){
            envVariable[i] = variable;
            result = 0;
            break;
        }
        i++;
    }
    return result;
}

int eVarNameExist(char *envVarName){
    int result = 0;
    int i = 0;
    while(i < varCapacity) {
        if(strcmp(envVariable[i]->name, envVarName) == 0){
            result = 1;
            break;
        }
        i++;
    }
    return result;
}

char *getEValue(char* envVarName){
    char *eValue;
    int i = 0;
    while(i < varCapacity) {
        if(strcmp(envVariable[i]->name, envVarName) == 0){
            eValue = envVariable[i]->value;
            break;
        }
        i++;
    }
    return eValue;
}

void inputTokenizer(char *input, char **token){
    int i = 0;
    token[i]= strtok(input," \n\t");
    while(token[i] != NULL && i < 100) {
        token[++i] = strtok(NULL, " \n\t");
    }
}

int builtinCmdTest(char *parse){
    int builtinNum = 4;
    char* buildCmds[builtinNum];
    buildCmds[0] = "exit";
    buildCmds[1] = "print";
    buildCmds[2] = "theme";
    buildCmds[3] = "log";
    
    for(int i = 0; i < builtinNum; i++){
        if(strcmp(parse, buildCmds[i]) == 0) {
            return i + 1;
        }
    }
    
    return 0;
}

int startWithDollar(char *input){
    if(input[0]=='$'){
        return 1;
    }
    return 0;
}

int cmdPrint(char **input){
        int i = 1;
        while (input[i] != NULL) {
            if(startWithDollar(input[i])){
                if(eVarNameExist(input[i])) {
                    printf("%s ", getEValue(input[i]));
                }
                else{
                    printf("EnvVar '%s' does not exist\n", input[i]);
                    return 1;
                }
            }
            else {
                printf("%s ", input[i]);
            }
            i++;
        }
        printf("\n");
        return 0;
}

int changeTheme(char *colour){
    int choice = 0;
    int colourNum = 3;
    char *allowedColours[colourNum];
    allowedColours[0] = "red";
    allowedColours[1] = "green";
    allowedColours[2] = "blue";

    if(colour == NULL) {
        printf("Please provide a colour of your choice\n");
        return 1;
    }
    
    for(int i = 0; i < colourNum; i++ ){
        if(strcmp(colour,allowedColours[i]) == 0) {
            choice = i + 1;
        }
    }

    switch(choice){
        case 1:
            printf("\033[1;31m");
            return 0;
        case 2:
            printf("\033[1;32m");
            return 0;
        case 3:
            printf("\033[1;34m");
            return 0;
        default:
            printf("unsupported theme\n");
    }
    return 1;
}

int containEqual(char* cmd){
    char c;
    int i = 0;
    int result = 0;
    while(true) {
        c = cmd[i];
        if(c == '=') {
            result = 1;
            break;
        }
        else if(c =='\0') {
            result = 0;
            break;
        }
        i++;
    }
    return result;
}

int nonBuildCmd(char** parse){
    int fds[2];
    int isValid[2];
    int valid = 0;
    int result = 0;
    int bytes = 0;
    char buff[1024];
    
    if (pipe(fds) == -1) {
        printf("Failed to initialize the pipe fds\n");
        return 1;
    }
    if (pipe(isValid) == -1) {
        printf("Failed to initialize the pipe isValid\n");
        return 1;
    }

    pid_t p1 = fork();
    if(p1 == -1) {
        printf("Failed to fork a child\n");
        return 1;
    }
    else if(p1 == 0) {
        dup2(fds[1], STDOUT_FILENO);
        close(fds[0]);

        if(execvp(parse[0], parse) < 0) {
            valid = 1;
        }

        close(isValid[0]);
        write(isValid[1], &valid, sizeof(valid));
        close(isValid[1]);
        exit(0);
    }
    else {
        wait(NULL);
        
        close(fds[1]);
        bytes = read(fds[0], buff, sizeof(buff));
        write(1, buff, bytes);
        close(fds[0]);
        
        close(isValid[1]);
        dup(isValid[0]);
        read(isValid[0], &result, sizeof(result));
        close(isValid[0]);

        if(result == 0) {
            return 0;
        }
        else {
            return 1;
        }
    }
}

int main(int argc, char *argv[]) {
    int interactive = 1;
    int action;
    bool logAdded;
    time_t localTime;

    if(argc == interactive) {
        while (1) {
            logAdded = false;
            char *userInput;
            char *args[100];
            Command *cmd = malloc(sizeof (Command));
            
            printf("cshell$ ");
            userInput = getInput();
            if(*userInput == '\0') {
                printf("Please provide an input\n");
                continue;
            }
            
            time(&localTime);
            cmd->time = *localtime(&localTime);
            
            inputTokenizer(userInput, args);
            cmd->name = strdup(args[0]);

            if((action = builtinCmdTest(args[0])) != 0) {
                switch (action) {
                    case 1:
                        quit();
                    case 2:
                        cmd->value = cmdPrint(args);
                        break;
                    case 3:
                        cmd->value = changeTheme(args[1]);
                        break;
                    case 4:
                        cmd->value = logInfo();
                        break;
                }
            } 
            else if(startWithDollar(args[0]) && containEqual(args[0])) {
                EnvVar *variable = malloc(sizeof(EnvVar));
                if(variable == NULL){
                    printf("Allocation failed\n");
                    exit(0);
                }
                char * varName = strtok(args[0], "=");
                char * varValue = strtok(NULL, "=");
                variable->name = strdup(varName);
                variable->value = strdup(varValue);
                if(eVarNameExist(variable->name) == 0) {
                    cmd->value = addEVariable(variable);
                }
                else {
                    cmd->value = updateEValue(variable);
                }
            }
            else {
                if(nonBuildCmd(args) == 0) {
                    cmd->value = 0;
                }
                else {
                    cmd->value = 1;
                    addToLog(cmd);
                    logAdded = true;
                    incorrectInput();
                }
            }
            if(logAdded == false) {
                addToLog(cmd);
            }
        }
    }
    else{
        char *args[100], userInput[1024];
        FILE *fp = fopen(argv[1], "r");
        if(fp == NULL) {
            printf("Failed to open the file\n");
            exit(EXIT_FAILURE);
        }
        while (fgets(userInput, sizeof(userInput),fp)) {
            Command *cmd = malloc(sizeof (Command));
            if(cmd == NULL){
                printf("Allocation failed\n");
                exit(0);
            }

            time(&localTime);
            cmd->time = *localtime(&localTime);
            
            inputTokenizer(userInput, args);
            cmd->name = strdup(args[0]);
            if((action = builtinCmdTest(args[0])) != 0) {
                switch (action) {
                    case 1:
                        quit();
                    case 2:
                        cmd->value = cmdPrint(args);
                        break;
                    case 3:
                        cmd->value = changeTheme(args[1]);
                        break;
                    case 4:
                        cmd->value = logInfo();
                        break;
                }
            } 
            else if(startWithDollar(args[0]) && containEqual(args[0])) {
                EnvVar *variable = malloc(sizeof(EnvVar));
                if(variable == NULL){
                    printf("Allocation failed\n");
                    exit(0);
                }
                char * varName = strtok(args[0], "=");
                char * varValue = strtok(NULL, "=");
                variable->name = strdup(varName);
                variable->value = strdup(varValue);
                if(eVarNameExist(variable->name) == 0) {
                    cmd->value = addEVariable(variable);
                }
                else {
                    cmd->value = updateEValue(variable);
                }
            } 
            else {
                if(nonBuildCmd(args) == 0) {
                    cmd->value = 0;
                }
                else {
                    incorrectInput();
                    quit();
                    cmd->value = 1;
                }
            }
            addToLog(cmd);
        }
        quit();
    }
    return 0;
}
