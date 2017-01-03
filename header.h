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

//fifo's			MUDAR 'agnelo' PARA NOME GENERICO DE USER
#define FSERV "serv.FIFO"
#define FCLI "cli%d.FIFO" //juntar pid() para tornar cada fifo de cada cliente unico

//algumas constantes
#define MAX_LOGIN 	64
#define MAX_CMD		256

//dimensoes do terminal
#define TERM_MAX_X		60
#define TERM_MAX_Y		35
//mapa do pacman
#define MAP_X			51//56
#define MAP_Y			21//33
/////////////////////////
#define C_SUP_ESQ_X		2
#define C_SUP_ESQ_y		1
#define C_SUP_DIR_X		58
#define C_SUP_DIR_Y		1
#define C_INF_ESQ_X		2
#define C_INF_ESQ_Y		34
#define C_INF_DIR_X		58
#define C_INF_DIR_Y		34

//vidas do pacman
#define PACMAN_LIVES_DEFAULT 3


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
	int criador;
	int next_menu;
	int n_atacantes;
	int n_defesas;
	
	int escolha_pos[18];
	//mapa do jogo
	char user_data_map[MAP_X][MAP_Y];
	//de acordo com a atribuição do servidor
	//numero do jogador
	//sendo que o primeiro a entrar (o que criar o jogo /joga com o pacman)
	//tem o numero 1
	//o primeiro fantasma, numero 2, etc etc
	int user_data_order;
	int posx, posy; //para mais facilmente saber a posição da personagem do jogador
	//independentemente de ser o pacman ou um fantasma
	int food; //número de migalhas comidas
	int ghosts; //número de fantasmas comidos
	int pacmans; //SÓ PARA FANTASMAS - número de pacman's comidos
	int edible; //SÓ PARA FANTASMAS - 1 se o fantasma for comestivel, 0 se não
	char isAtopFood; //SÓ PARA FANTASMAS - 1 se a célula actual (última para a qual se moveu)
	//continha comida ou não (para poder repor), 0 se não
};