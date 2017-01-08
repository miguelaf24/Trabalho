//em vez de fazer imensos includes em cada ficheiro, usar um header.h central
//tudo que tiver de ser definido em ambos server.c e client.c ficará aqui
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
#include <pthread.h>


#define FSERV "serv.FIFO"
#define FCLI "cli%d.FIFO" //juntar pid() para tornar cada fifo de cada cliente unico

//algumas constantes
#define MAX_LOGIN 	64
#define MAX_CMD		256

//dimensoes do terminal
#define TERM_MAX_X		60
#define TERM_MAX_Y		35

#define MAP_X	51
#define MAP_Y	21



//dados de cada utilizador
typedef struct utilizador user_data;
struct utilizador
{
	char user_data_fifo[MAX_LOGIN]; //nome do ficheiro FIFO do cliente
	char user_data_uname[MAX_LOGIN];
	char user_data_upass[MAX_LOGIN];
	int user_data_pid; //pid do cliente
	char user_data_cmd[MAX_CMD]; //contém as ações do cliente (key presses, etc)
	//ou simplesmente "login" na primeira ligação ao servidor
	int user_data_ingame; // 1/0
	int jogo_correr;
	int next_menu;
	int n_atacantes;
	int n_defesas;
	
	int escolha_pos[18];
	
	char user_data_map[MAP_X][MAP_Y];
	
	int user_data_order;
	int posx, posy; //para mais facilmente saber a posição da personagem do jogador
	
};