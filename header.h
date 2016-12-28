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

#define MAX_CMD 256
#define MAX_LOGIN 64

#define MAX_JOG 20

#define MAP_X			28//56
#define MAP_Y			31//33

typedef struct utilizador user_data;
struct utilizador{
	char user_data_fifo[MAX_LOGIN];
	char user_data_uname[MAX_LOGIN];
	char user_data_upass[MAX_LOGIN];
	char user_data_cmd[MAX_CMD];
	int user_data_order;
	int user_data_ingame;
	int algo, pid; 
};
