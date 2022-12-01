#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/wait.h>
#include "cmdParser.c"

#define LINE_MAX 2048

void dir()
{

}

void copy(char * src, char * dst)
{
    char * pathSrcFile = realpath(src, NULL); //gettin full path of the file we want to copy
    //gettint the byte size of the file we want to copy
    struct stat sizeFile1;  
    stat(pathSrcFile, &sizeFile1);
    int size1 = sizeFile1.st_size;
    char * file1 = malloc(size1); //creating a string to put the content of the file when we read it
    int f1 = open(pathSrcFile, O_RDONLY); //open the file we want to copy just for read using his full path
    //checking if the file got opend
    if(f1 == -1)
    {
        printf("couldn't open source file\n");
        exit(1);
    }
    read(f1, file1, size1); //read from the file and putting what we read to the string file1
    //open the destination file and write to it
    int f2 = open (dst, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU); //open the file to where we want to copy for writing and if it dose not exist create it
    //checking if the file got opend
    if(f2 == -1)
    {
        printf("couldn't open destination file\n");
        exit(1);
    }
    //writing to the file the content of the file we wanted to copy
    if(write(f2, file1, size1) == size1)
    {
        printf("file is copied.\n");
    }
    close(f1);
    close(f2);
}

void otherCommands(cmdLine * pCmdLine)

{
        int pid = fork();
        if(!pid)
        {
            execvp(pCmdLine->arguments[0],pCmdLine->arguments);
            perror("error\n");
            _exit(1);
        }
        else {
            if (pCmdLine->blocking)
                waitpid(pid,0,0);
        }
}

int main(int argc, char const *argv[])
{
    cmdLine* parsedLine;
    char cmdLine[LINE_MAX];
    char pwd[PATH_MAX];

    while (1) {
        getcwd(pwd,PATH_MAX);
        printf("%s$ ", pwd);

        if (read(0, cmdLine, LINE_MAX) == -1)
            exit(2);
        if(strcmp(cmdLine, "quit") == 0)
            return 0;
        if(strcmp(cmdLine, "DIR") == 0)
            dir();
        else
        {
            char *ptr = strtok(cmdLine, " ");
            if(strcmp(ptr, "COPY") == 0)
            {
                char * src = strtok(NULL, " ");
                char * dst = strtok(NULL, " ");
                copy(src, dst);
                continue;
            }
            else
            {
                parsedLine = parseCmdLines(cmdLine);
                otherCommands(parsedLine);
            }
        }
    }
    return 0;
}
