#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <direct.h>
#include <process.h>
#define MAX_CHARS 81

//show files in directory
int listdir(char* path){
    WIN32_FIND_DATAA fdFile;
    HANDLE hFind = NULL;
    char sPath[2048];

    sprintf(sPath, "%s\\*.*", path);

    if((hFind = FindFirstFileA(sPath, &fdFile)) == INVALID_HANDLE_VALUE) {
        printf("Path not found: [%s]\n", path);
        return 0;
    }

    do{
        if(strcmp(fdFile.cFileName, ".") != 0 &&
            strcmp(fdFile.cFileName, "..") != 0) {

            sprintf(sPath, "%s\\%s", path, fdFile.cFileName);

            if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                printf("[DIR] %s\n", sPath);
            }else{
                printf("%s\n", sPath);
            }
        }
    }while(FindNextFileA(hFind, &fdFile));

    FindClose(hFind);
    return 1;
}

//print file contents
void cat(char* filename) {
    FILE* fp;
    //read file
    fp = fopen(filename, "r");

    if(fp == NULL){
        printf("error\n");
        exit(1);
    }

    int ch = fgetc(fp);
    
    //print file contents
    char line[256]={0};
    char* ptr;
    while(fgets(line,256,fp)){
        ptr = strchr(line, '\n');
        if(ptr != NULL){
            *ptr = '\0';
        }
        puts(line);
    }

    fclose(fp);
}

//change directory to path
void cd(char* path) {
    if(!path){
        printf("error: missing path\n");
        return;
    }
    if(_chdir(path) != 0){
        perror("cd");
    }
    _chdir(path);
}

//clear
void cls() {
    system("cls");
}

//copy directory
void copy(char* file1, char* file2) {
    FILE *f1;
    FILE *f2;
    char contents[MAX_CHARS];

    if (!file1 || !file2) {
        printf("error: missing file\n");
        return;
    }

    f1 = fopen(file1, "rb");
    if (!f1) {
        perror("error: missing file\n");
        fclose(f1);
        return;
    }

    f2 = fopen(file2, "wb");
    if (!f2) {
        perror("error: missing file\n");
        fclose(f2);
        return;
    }

    int size = fread(contents, 1, sizeof(contents), f1);
    while (size > 0)
        fwrite(contents, 1, size, f2);

    fclose(f1);
    fclose(f2);
}

//echo command
void echo(char** cmd) {
    for(int i = 1; cmd[i] != NULL; i++){
        printf("%s ", cmd[i]);
    }
    printf("\n");
}

//execute file
void exec_cmd(char* path, char** cmd) {
    if(!path){
        printf("error: missing path\n");
        return;
    }
    execvp(path, cmd);
    perror("exec");
}

//help
void help() {
    printf("commands:\n");
    printf("cat *file name* : print file contents\n");
    printf("cd *path* : change current directory to specified path\n");
    printf("cls : clear screen\n");
    printf("copy *file1* *file2* : copy file1 to file2\n");
    printf("echo *text* : echo command\n");
    printf("exec *file.exe* : execute a exe file\n");
    printf("dir *path* : show files in a directory\n");
    printf("mkdir *directory name* : make a new directory\n");
    printf("delfile *file* : delete file\n");
    printf("wc *file* : count words, sentences, and lines in a file\n");
    printf("exit : exit minishell\n");
    printf("help : show help\n");
}

//delete file
void delfile(char* file) {
    if(!file){
        printf("error: missing file\n");
        return;
    }
    if (remove(file) != 0){
        perror("delfile");
    }
    remove(file);
}

//make directory
void makedir(char* dirname) {
    if(!dirname){
        printf("error: missing directory name\n");
        return;
    }
    if (_mkdir(dirname) != 0){
        perror("mkdir");
    }
    _mkdir(dirname);
}

//count
void mywc(char* filename) {
    FILE* fp;
    int words = 0;
    int sentences = 0;
    int lines = 0;

    //read file
    fp = fopen(filename, "r");

    if(fp == NULL){
        printf("error\n");
        exit(1);
    }

    int ch = fgetc(fp);
    //counts words, sentences, and lines
    while(ch != -1){
        if(ch == ' ' || ch == '\n'){
            words++;
        }
        if(ch == '.' || ch == '!' || ch == '?'){
            sentences++;
        }
        ch = fgetc(fp);
    }
    fclose(fp);

    char line[256]={0};
    char* ptr;

    fp = fopen(filename, "r");

    if(fp == NULL){
        printf("error\n");
        exit(1);
    }
    
    while(fgets(line,256,fp)){
        lines++;
    }
    fclose(fp);

    printf("Words: %d\n", words);
    printf("Sentences: %d\n", sentences);
    printf("Lines: %d\n", lines);
}

//pipe command
int pipeCommandsWin32(char* cmd1, char* cmd2){
    PROCESS_INFORMATION piSource, piDest;
    STARTUPINFOA siSource, siDest;
    SECURITY_ATTRIBUTES sa;

    HANDLE hPipeRead, hPipeWrite;
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hStdIn  = GetStdHandle(STD_INPUT_HANDLE);

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    //create pipe
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &sa, 0)) {
        return 1;
    }
    
    ZeroMemory(&piSource, sizeof(piSource));
    ZeroMemory(&piDest, sizeof(piDest));

    siSource.cb = sizeof(STARTUPINFOA);
    siDest.cb = sizeof(STARTUPINFOA);

    siSource.dwFlags = STARTF_USESTDHANDLES;
    siDest.dwFlags   = STARTF_USESTDHANDLES;

    //source
    siSource.hStdInput  = hStdIn;
    siSource.hStdOutput = hPipeWrite;
    siSource.hStdError  = hStdOut;
    //destination
    siDest.hStdInput  = hPipeRead;
    siDest.hStdOutput = hStdOut;
    siDest.hStdError  = hStdOut;

    //wrap
    char fullCmd1[256];
    char fullCmd2[256];
    sprintf(fullCmd1, "cmd /c %s", cmd1);
    sprintf(fullCmd2, "cmd /c %s", cmd2);

    //first command process
    if(!CreateProcessA(NULL, fullCmd1, NULL, NULL, TRUE, 0, NULL, NULL, &siSource, &piSource)){
        return 1;
    }
    CloseHandle(piSource.hThread);
    //second command process
    if(!CreateProcessA(NULL, fullCmd2, NULL, NULL, TRUE, 0, NULL, NULL, &siDest, &piDest)){
        return 1;
    }
    
    //close
    CloseHandle(hPipeWrite);
    CloseHandle(hPipeRead);

    WaitForSingleObject(piSource.hProcess, INFINITE);
    WaitForSingleObject(piDest.hProcess, INFINITE);

    CloseHandle(piSource.hProcess);
    CloseHandle(piSource.hThread);
    CloseHandle(piDest.hProcess);
    CloseHandle(piDest.hThread);
    return 0;
}

//check for pipe command
void checkPipe(char* line) {
    char* cmd1 = strtok(line, "|");
    char* cmd2 = strtok(NULL, "|");

    if (!cmd1 || !cmd2) {
        printf("error: missing command\n");
        return;
    }

    pipeCommandsWin32(cmd1, cmd2);
}


int main() {
    char path[1024];
    char line[256];
    char* cmd[10];
    while(1){
        getcwd(path, sizeof(path));
        printf("%s> ",path);
        fgets(line, sizeof(line), stdin);

        //check pipe
        if(strchr(line, '|')){
            checkPipe(line);
            continue;
        }

        char* token = strtok(line, " \n\r");
        int i = 0;
        while(token != NULL){
            cmd[i] = token;
            i++;
            token = strtok(NULL, " \n\r");
        }

        //matching inputs to their commands
        if(strcmp(cmd[0], "exit") == 0) {
            break;
        }
        else if(strcmp(cmd[0], "cls") == 0) {
            cls();
        }
        else if(strcmp(cmd[0], "help") == 0) {
            help();
        }
        else if(strcmp(cmd[0], "cd") == 0) {
            cd(cmd[1]);
        }
        else if(strcmp(cmd[0], "cat") == 0) {
            cat(cmd[1]);
        }
        else if(strcmp(cmd[0], "copy") == 0) {
            copy(cmd[1], cmd[2]);
        }
        else if(strcmp(cmd[0], "echo") == 0) {
            echo(cmd);
        }
        else if(strcmp(cmd[0], "wc") == 0) {
            mywc(cmd[1]);
        }
        else if(strcmp(cmd[0], "mkdir") == 0) {
            makedir(cmd[1]);
        }
        else if(strcmp(cmd[0], "delfile") == 0) {
            delfile(cmd[1]);
        }
        else if(strcmp(cmd[0], "dir") == 0) {
            listdir(cmd[1]);
        }
        else if(strcmp(cmd[0], "exec") == 0) {
            exec_cmd(cmd[1], cmd);
        }
    }
}
