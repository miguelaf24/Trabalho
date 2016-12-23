//definir aqui os includes todos necessários para o trabalho 
#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <locale.h>

#define FIFOSERV "server.FIFO"
#define FIFOCLI "cli%d.FIFO"

typedef struct {
int algo, pid; 
}USER;
