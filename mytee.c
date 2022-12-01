#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_LENGTH 1024

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("should have 2 arguments\n");
        exit(0);
    }

    char * buff = malloc(MAX_LENGTH); //creating a string to put the content of the stdIn when we read it
    int byteRead = read(0, buff, MAX_LENGTH); //read from stdIn and putting what we read to the string file1

    //writing to stdOut(file descreptor 1) what we raed in stdIn
    if(write(1, buff, byteRead) != byteRead)
    {
        printf("\nfaild to write\n");
    }

    int fd = open (argv[1], O_RDWR | O_CREAT | O_TRUNC, S_IRWXU); //open the file to where we want to copy for writing and if it dose not exist create it
    //checking if the file got opend
    if(fd == -1)
    {
        printf("couldn't open destination file\n");
        exit(1);
    }
    //writing to the file the content of stdIn
    if(write(fd, buff, byteRead) != byteRead)
    {
        printf("couldn't write to file.\n" );
    }
    close(fd);
    return 0;
}