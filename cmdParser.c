#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//#ifndef NULL
//#define NULL 0
//#endif

#define MAX_ARGUMENTS 256
//#define FREE(X) if(X) free((void*)X)


/*
 we'll create a new struct to hols the information of the cmd line
Example :
parsing the string: "cat file.c > output.txt &"
results in a cmdLine struct, consisting of:
							 arguments = {"cat", "file.c"},
							 outputRedirect = "output.txt",
							 blocking = 0, etc.
  */
typedef struct cmdLine
{
    char * arguments[MAX_ARGUMENTS]; /* command line arguments (arg 0 is the command)*/
    int argCount;		/* number of arguments */
    char *inputRedirect;	/* input redirection path. NULL if no input redirection */
    char *outputRedirect;	/* output redirection path. NULL if no output redirection */
    char blocking;	/* boolean indicating blocking/non-blocking */
    int idx;				/* index of current command in the chain of cmdLines (0 for the first) */
    struct cmdLine *next;	/* next cmdLine in chain */
} cmdLine;


//checks if the line from the cmd is empty
static int isEmpty(const char *str)
{
    if (!str)
        return 1;

    while (*str)
        if (!isspace(*(str++)))
            return 0;

    return 1;
}

static char *cloneFirstWord(char *str)
{
    char *start = NULL;
    char *end = NULL;
    char *word;

    while (!end)
    {
        switch (*str)
        {
            case '>':
            case '<':
            case 0:
                end = str - 1;
                break;
            case ' ':
                if (start)
                    end = str - 1;
                break;
            default:
                if (!start)
                    start = str;
                break;
        }
        str++;
    }

    if (start == NULL)
        return NULL;

    word = (char*) malloc(end-start+2);
    strncpy(word, start, ((int)(end-start)+1)) ;
    word[ (int)((end-start)+1)] = 0;

    return word;
}

//static void extractRedirections(char *strLine, cmdLine *pCmdLine)
//{
//    char *s = strLine;
//
//    while ( (s = strpbrk(s,"<>")) )
//    {
//        if (*s == '<')
//        {
//            free(pCmdLine->inputRedirect);
//            pCmdLine->inputRedirect = cloneFirstWord(s+1);
//        }
//        else
//        {
//            free(pCmdLine->outputRedirect);
//            pCmdLine->outputRedirect = cloneFirstWord(s+1);
//        }
//
//        *s++ = 0;
//    }
//}

static cmdLine * parseSingleCmdLine(const char *strLine)
{
    char * delimiter = " ";
    char * line, * result, * buff;

    if (isEmpty(strLine))
        return NULL;

    cmdLine* pCmdLine = (cmdLine*)malloc( sizeof(cmdLine) ) ;
    memset(pCmdLine, 0, sizeof(cmdLine));

    strcpy(line, strLine);
//    extractRedirections(line, pCmdLine);
    result = strtok( line, delimiter);
    while( result && pCmdLine->argCount < MAX_ARGUMENTS-1)
    {
        strcpy(buff, result);
        ((char**)pCmdLine->arguments)[pCmdLine->argCount++] = buff;
        result = strtok ( NULL, delimiter);
    }

    free(line);
    return pCmdLine;
}

static cmdLine * _parseCmdLines(char *line)
{
    char * nextStrCmd;
    cmdLine * pCmdLine;
    char pipeDelimiter = '|';

    if (isEmpty(line))
        return NULL;

    //searching the char |
    nextStrCmd = strchr(line , pipeDelimiter);
    if (nextStrCmd) //if we couldn't find it then put 0
        *nextStrCmd = 0;

    pCmdLine = parseSingleCmdLine(line);
    if (!pCmdLine)
        return NULL;

    if (nextStrCmd)
        pCmdLine->next = _parseCmdLines(nextStrCmd+1);

    return pCmdLine;
}

cmdLine * parseCmdLines(char * strLine)
{
    char * line, * ampersand;
    cmdLine * head, * last;
    int idx = 0;

    if (isEmpty(strLine))
        return NULL;

    strcpy(line, strLine);
    if (line[strlen(line)-1] == '\n')
        line[strlen(line)-1] = 0;

    //searching te char &
    ampersand = strchr( line,  '&');
    if (ampersand)
        *(ampersand) = 0;

    if ( (last = head = _parseCmdLines(line)) )
    {
        while (last->next)
            last = last->next;
        last->blocking = ampersand? 0:1;
    }

    for (last = head; last; last = last->next)
        last->idx = idx++;

    free(line);
    return head;
}


