#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAX_ARGUMENTS 256
#define LINE_MAX 2048
#define INPUT 0
#define OUTPUT 1
#define APPEND 2

//set follow-fork-mode child
//set detach-on-fork off

/*
 * Function Create array of strings fro user input
 * Example :
parsing the string: "cat file.c > output.txt &"
results:arguments = {"cat","file.c",">","output.txt","&"},
 */
char ** parseCMD(char * cmdline) {
    int countSpace = 0, idx = 0;
    char * line = malloc(strlen(cmdline));
    strcpy(line, cmdline);
    if (line[strlen(line)-1] == '\n')
        line[strlen(line)-1] = 0;
    //how many spaces there is
    while(idx != strlen(line))
    {
        //https://www.geeksforgeeks.org/isblank-in-cc/
        if(line[idx] == ' ')
            countSpace++;
        idx++;
    }

    char **argv;
    argv = malloc((countSpace + 1) * sizeof(char *));
    int i = 0;
    argv[i] = strtok(line, " ");//separate string by spaces
    i++;
    while (i != (countSpace + 1)) {
        argv[i] = strtok(NULL, " ");
        i++;
    }
    return argv;
}
/*
removes the newline and space character from the end and start of a char*
*/
void removeWhiteSpace(char* buf){
    if(buf[strlen(buf)-1]==' ' || buf[strlen(buf)-1]=='\n')
        buf[strlen(buf)-1]='\0';
    if(buf[0]==' ' || buf[0]=='\n') memmove(buf, buf+1, strlen(buf));
}

/*
tokenizes char* buf using the delimiter c, and returns the array of strings in param
and the size of the array in pointer nr
*/
void tokenize_buffer(char** param,int *nr,char *buf,const char *c){
    char *token;
    token=strtok(buf,c);
    int pc=-1;
    while(token){
        param[++pc]=malloc(sizeof(token)+1);
        strcpy(param[pc],token);
        removeWhiteSpace(param[pc]);
        token=strtok(NULL,c);
    }
    param[++pc]=NULL;
    *nr=pc;
}





void dir() {
    struct dirent *de;
    DIR *dir = opendir(".");
    if (dir == NULL) // opendir returns NULL if couldn't open directory
    {
        printf("Could not open directory");
        closedir(dir);
        return;
    } else {
        printf("Manage to open directory");
        while ((de = readdir(dir)) != NULL) {
            if (de->d_name[0] != '.')
                printf("%s\n", de->d_name);
        }
    }
    closedir(dir);
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

//the execve here is according to https://linuxhint.com/c-execve-function-usage/
void otherCommands(char * cmdline) {
    pid_t pid = fork();
    if (pid == 0) {
        char **arguments = parseCMD(cmdline);
        char *path = "/bin/";
        char *fullPath = (char *) malloc(strlen(path) + sizeof(arguments[0]) + 1);
        strcpy(fullPath, path);
        strcat(fullPath, arguments[0]);

        char **env = {NULL};
        if (execve(fullPath, arguments, env) == -1) //execve will always override the process it run on, in this case it will override the child process
        {
            perror("error");
        }
        return;
    } else {
        // I'm the parent process
        wait(NULL);
        return;
    }
}

void Pipe(char** buf,int nr){//can support up to 10 piped commands
    if(nr>10) return;

    int fd[10][2],i,pc;
    char *argv[100];

    for(i=0;i<nr;i++){
        tokenize_buffer(argv,&pc,buf[i]," ");
        if(i!=nr-1){
            if(pipe(fd[i])<0){
                perror("pipe creating was not successfull\n");
                return;
            }
        }
        if(fork()==0){//child1
            if(i!=nr-1){
                dup2(fd[i][1],1);
                close(fd[i][0]);
                close(fd[i][1]);
            }

            if(i!=0){
                dup2(fd[i-1][0],0);
                close(fd[i-1][1]);
                close(fd[i-1][0]);
            }
            execvp(argv[0],argv);
            perror("invalid input ");
            exit(1);//in case exec is not successfull, exit
        }
        //parent
        if(i!=0){//second process
            close(fd[i-1][0]);
            close(fd[i-1][1]);
        }
        wait(NULL);
    }
}

void Redirect(char** buf,int nr,int mode){
    int pc,fd;
    char *argv[100];
    removeWhiteSpace(buf[1]);
    tokenize_buffer(argv,&pc,buf[0]," ");
    if(fork()==0){

        switch(mode){
            case INPUT:  fd=open(buf[1],O_RDONLY); break;
            case OUTPUT: fd=open(buf[1],O_WRONLY); break;
            case APPEND: fd=open(buf[1],O_WRONLY | O_APPEND); break;
            default: return;
        }

        if(fd<0){
            perror("cannot open file\n");
            return;
        }

        switch(mode){
            case INPUT:  		dup2(fd,0); break;
            case OUTPUT: 		dup2(fd,1); break;
            case APPEND: 		dup2(fd,1); break;
            default: return;
        }
        execvp(argv[0],argv);
        perror("invalid input ");
        exit(1);//in case exec is not successfull, exit
    }
    wait(NULL);
}

int server(char * cmdline)
{
    cmdline = cmdline + 2;
    int port = atoi(cmdline);

    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;
    char server_message[2048], client_message[2048];

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0) {
        printf("Error while creating socket\n");
        exit(1);
    }
    printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind to the set port and IP:
    if (bind(socket_desc, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("Couldn't bind to the port\n");
        exit(1);
    }
    printf("Done with binding\n");

    // Listen for clients:
    if (listen(socket_desc, 1) < 0) {
        printf("Error while listening\n");
        exit(1);
    }
    printf("\nListening for incoming connections.....\n");

    // Accept an incoming connection:
    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr *) &client_addr, &client_size);

    if (client_sock < 0) {
        printf("Can't accept\n");
        exit(1);
    }
    printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));


    // Receive client's message once (the command):
    if (recv(client_sock, client_message, sizeof(client_message), 0) < 0) {
        printf("Couldn't receive\n");
        exit(1);
    }
    printf("%s", client_message);

    // calculating result
    char **parsedCommand = parseCMD(client_message);
    pid_t pid = fork();
    char *tempFile = "temp.txt";
    if (pid == 0) {
        int file = open(tempFile, O_WRONLY | O_CREAT | O_RDONLY, 0777);
        dup2(file, STDOUT_FILENO);
        close(file);


        //I'm the child process
        // according to https://www.youtube.com/watch?v=iq7puCxsgHQ
        char *path = "/bin/";
        char *fullPath = (char *) malloc(strlen(path) + sizeof(parsedCommand[0]) + 1);
        strcpy(fullPath, path);
        strcat(fullPath, parsedCommand[0]);

        char **env = {NULL};
        if (execve(fullPath, parsedCommand, env) == -1) {
            perror("error");
        }
    } else {
        wait(NULL);
    }

    char messageToSend[2048];
    int scrFile;
    scrFile = open(tempFile, O_RDONLY);
    if (scrFile > 0) {
        read(scrFile, messageToSend, 2048);
        close(scrFile);
        remove(tempFile);
    }

    if (send(client_sock, messageToSend, strlen(messageToSend), 0) < 0) {
        printf("Can't send\n");
        exit(1);
    }

    // Closing the socket:
    close(client_sock);
    close(socket_desc);
}

int client(char * cmdline) {
    char *curlyBrace = strstr(cmdline, "}");
    *(curlyBrace - 1) = '\0';
    char *command = cmdline;

    char *ip = curlyBrace + 2;
    char *portStr = strstr(ip, ":");
    *portStr = '\0';
    portStr += 1;

    int port = atoi(portStr);


    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2048];
    char *client_message = command;

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0) {
        printf("Unable to create socket\n");
        exit(1);
    }

    printf("Socket created successfully\n");

    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Send connection request to server:
    if (connect(socket_desc, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("Unable to connect\n");
        exit(1);
    }
    printf("Connected with server successfully\n");

    // Send the message to server:
    if (send(socket_desc, client_message, strlen(client_message), 0) < 0) {
        printf("Unable to send message\n");
        exit(1);
    }

    // Receive the server's response:
    if (recv(socket_desc, server_message, sizeof(server_message), 0) < 0) {
        printf("Error while receiving server's msg\n");
        exit(1);
    }

    printf("Server's response: \n%s\n", server_message);

    // Close the socket:
    close(socket_desc);
}


int main(int argc, char const *argv[])
{
    char ** parsedLine;
    char cmdLine[LINE_MAX];
    char pwd[PATH_MAX];

    while (1) {
        memset(cmdLine, 0, LINE_MAX);
        if (read(0, cmdLine, LINE_MAX) == -1)
            exit(2);

        //for this function we used:
        //https://man7.org/linux/man-pages/man3/opendir.3.html
        //https://man7.org/linux/man-pages/man3/readdir.3.html
        if((strcmp(cmdLine, "DIR") == 0) || (strcmp(cmdLine, "DIR\n") == 0) || (strcmp(cmdLine, "dir") == 0) || (strcmp(cmdLine, "dir\n") == 0))
        {
            dir();
            continue;
        }

        else
        {
            char * buff = malloc(LINE_MAX);
            strcpy(buff, cmdLine);
            char * ptr = strtok(buff, " ");

            //for this function we used the code from task 1
            if(strcmp(ptr, "COPY") == 0)
            {
                char * src = strtok(NULL, " ");
                char * dst = strtok(NULL, " ");
                copy(src, dst);
                continue;
            }

            //the functions for pipe and <> adapted from:
            // https://github.com/csabagabor/Basic-Shell-implementation-in-C/blob/master/shell.c
            //https://aljensencprogramming.wordpress.com/2014/05/16/implementing-pipes-in-c/
            //
            else
            {
                int nr=0;
                char * buffer[LINE_MAX];
                //searching the char |
                if (strchr(cmdLine , '|') != NULL) {
                    tokenize_buffer(buffer,&nr,cmdLine,"|");
                    Pipe(buffer,nr);
                    continue;
                }

                else
                {
                    char * s = malloc(LINE_MAX/2);
                    s = strpbrk(cmdLine,"<>");
                    if (s != NULL) {
                        if (*s == '<') {
                            tokenize_buffer(buffer, &nr, cmdLine, "<");
                            if (nr == 2)
                                Redirect(buffer, nr, INPUT);
                            else
                                printf("Incorrect input redirection!(has to to be in this form: command < file)");
                        }
                        else {
                            tokenize_buffer(buffer, &nr, cmdLine, ">");
                            if (nr == 2)
                                Redirect(buffer, nr, OUTPUT);
                            else
                                printf("Incorrect output redirection!(has to to be in this form: command > file)");
                        }
                        continue;
                    }

                    else {
                        s = strpbrk(cmdLine, "{}");
                        if (s != NULL) {
                            //do somthing
                            if (*s == '{') {
                                server(cmdLine);
                            } else {
                                client(cmdLine);
                            }
                            continue;
                        }

                        else
                        {
                            otherCommands(cmdLine);
                        }
                    }
                    free(s);
                }
            }
        }
    }

    return 0;
}
