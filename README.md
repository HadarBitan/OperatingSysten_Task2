318463882 318356599
# OperatingSysten_Task2

## How to run?
### Part A

* mytee: receives a file.The user will write to stdin and what he wrote will be written to the file and to stdout.

  USE: ./mytee -> file.txt

* mync: read input from stdin and transfer by socket to stdout.
* 
  USE: open two terminals:
  
  1 ./nc -lv (port_number) -> to open server
  
  2 ./nc -v (ip_address , port) -> open the client

### Part B

* COPY: the same like in assignment 1 -> copies the content of SRC file to DEST file.

  USE: COPY
  
* DIR: prints everything the curret reposetory contains like ls we familiar with.

  USE: DIR
  
* USE:  you can try any linux command in the shell you know.

* pipe

  USE:  first | seconde -> for example: cat file1.txt | sort -> will sort for us what the file contains.
  
*

*



#### code we have used:
[https://dev.to/0xbf/implement-tee-cli-in-c-c-lang-2p5c](https://github.com/csabagabor/Basic-Shell-implementation-in-C/blob/master/shell.c)
 
[https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/](https://aljensencprogramming.wordpress.com/2014/05/16/implementing-pipes-in-c/)

https://man7.org/linux/man-pages/man3/opendir.3.html

https://man7.org/linux/man-pages/man3/readdir.3.html

https://aljensencprogramming.wordpress.com/2014/05/16/implementing-pipes-in-c/
