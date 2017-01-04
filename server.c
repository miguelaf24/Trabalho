#include "header.h"

//aviso de erro
#define ERR_DEFAULT "ERRO "
#define ERR_SYNTAX " ERRO - sintaxe incorrecta -> ./server ficheiro_users\n"

//ASCII colour escapes
#define ASC_C_NORMAL	"\x1b[0m"
#define ASC_C_RED		"\x1b[31m"
#define ASC_C_GREEN		"\x1b[32m"
#define ASC_C_YELLOW	"\x1b[33m"
#define ASC_C_BLUE		"\x1b[34m"
#define ASC_C_CYAN		"\x1b[36m"

//max jogadores - A MUDAR PARA VARIAVEL DE AMBIENTE
#define MAX_JOG		18

//1- pacman
//fantasmas:
//			2 - verde
//			3 - amarelo
//			4 - magenta
//			5 - ciano
int playerOrder = 0;//QUANTIDADE DE JOGADORES

int atacantes = 2;

int defesas = 2;

//posi��es iniciais
int init_xy_Jogador[18][2] = {
	{0, 10},
	{10, 12},
	{10, 6},
	{10, 15},
	{10, 3},
	{20, 12},
	{20, 6},
	{20, 15},
	{20, 3},

	{50, 10},
	{40, 12},
	{40, 6},
	{40, 15},
	{40, 3},
	{30, 12},
	{30, 6},
	{30, 15},
	{30, 3}};


int init_xy_E1_0[2] = {0, 10};
int init_xy_E1_1[2] = {10, 12};
int init_xy_E1_2[2] = {10, 6};
int init_xy_E1_3[2] = {10, 15};
int init_xy_E1_4[2] = {10, 3};
int init_xy_E1_6[2] = {20, 12};
int init_xy_E1_7[2] = {20, 6};
int init_xy_E1_8[2] = {20, 15};
int init_xy_E1_9[2] = {20, 3};

int init_xy_E2_0[2] = {50, 10};
int init_xy_E2_1[2] = {40, 12};
int init_xy_E2_2[2] = {40, 6};
int init_xy_E2_3[2] = {40, 15};
int init_xy_E2_4[2] = {40, 3};
int init_xy_E2_6[2] = {30, 12};
int init_xy_E2_7[2] = {30, 6};
int init_xy_E2_8[2] = {30, 15};
int init_xy_E2_9[2] = {30, 3};

int posse_bola = -1;
int init_bola[2] = {25,10};
//guardar posi��es antigas(caso o cliente tenha entrado mas a
//IA j� tenha alterado a posi��o do seu fantasma)
int old_xy_ghost1[2] = {0, 0};
int old_xy_ghost2[2] = {0, 0};
int old_xy_ghost3[2] = {0, 0};
int old_xy_ghost4[2] = {0, 0};

//posi��es de A e B
int pos_xy_A[2] = {0, 0};
int pos_xy_B[2] = {0, 0};

int pos_ocupadas[18]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int configurar = 0;
//n�mero total de casas com comida
int count_total_food = 0;
//total casas 'O' j� comidas
int count_eaten = 0;
//vidas do pacman
int pacman_lives = PACMAN_LIVES_DEFAULT;

//placeholder para o nome do mapa em uso
//char map[MAX_CMD] = "original.map"; //por carrega o mapa original
//"backup" do mapa do jogo
char backup_game_map[MAP_X][MAP_Y];

//fun��o de fecho do programa
void terminate()
{
	//remover o fifo do servidor
	unlink(FSERV);
	
	exit(0);
}

void limpa_consola()
{
	system("clear");//limpar a consola
}

//fun��o que trata sinais
void signal_handle(int sign)
{
	switch (sign)
	{
		case SIGUSR1:
			printf(" SIGUSR1 recebido, a terminar...\n");
			terminate();
			break;
		case SIGINT:
			printf(" Programa interrompido, a terminar...\n");
			terminate();
			break;
	}
}

//atualizar as posi��es do pacman e dos fantasmas
void update_pos(char game_map[MAP_X][MAP_Y])
{
	int i, j;
	
	game_map[init_bola[0]][init_bola[1]] = 'o';
	for(i=0;i<18;i++){
		if(pos_ocupadas[i]!=-1){
			if(i<9)
				game_map[init_xy_Jogador[i][0]][init_xy_Jogador[i][1]] = 'A'+i;
			else
				game_map[init_xy_Jogador[i][0]][init_xy_Jogador[i][1]] = 'a'+i-9;
		}
	}
	
}

//gerar o mapa do jogo
void create_game_map(char game_map[MAP_X][MAP_Y])
{
	
	int i, j;
	
	//ler para o array do mapa
	for(i=0; i < MAP_Y ; i++)
	{
		for(j = 0; j < MAP_X; j++)
		{
			game_map[j][i] = ' ';
			//backup_game_map[j][i] = 117; //criar backup
		}
	}
	
	for(i=6;i<15;i++){
		game_map[0][i]=(char)186;
		game_map[MAP_X-1][i]=(char)186;
	}
	game_map[0][14]=200;
	game_map[MAP_X-1][14]=188;
}

//procurar um fantasma pela sua posi��o [x,y]
//para verificar se est� comest�vel ou n�o
int search_ghost(int x, int y, user_data *us_players, int *us_players_num)
{
	int i;
	
	for(i = 0; i < *us_players_num; i++)
	{
		if(us_players[i].posx == x && us_players[i].posy == y)
			return i; //encontrado
		
	}
	
	return 0;
}

//fun��o que verifica quais clientes continuam ligados
//usando-a como uma rotina chamada em ciclo, facilita o processo
//de ter em conta quais clientes continuam ligados e quais sairam
// recebe o array de structs dos jogadores ligados e quantos est�o ligados
//caso 1 cliente tenha saido, ajustar o array
void verify_connected_clients(user_data *us_players, int *us_players_num)
{
	//sair logo caso nao haja jogadores ligados
	if (*us_players_num == 0)
		return;
	//else...continuar a fun��o
	
	int i, j;
	int count_ok = 0; //contador que for�a
	//o ciclo a continuar enquanto count_ok != *us_players_num
	
	while(count_ok != *us_players_num)
	{
		//para cada struct registada no array, fazer access()
		for(i=0; i < *us_players_num; i++)
		{
			if(access(us_players[i].user_data_fifo, F_OK) == 0)
				count_ok++; //OK
			else
				us_players[i].user_data_pid = 0; //NOT OK
				//colocar o pid a 0 como uma referencia para
				//"este cliente j� saiu, apag�-lo de seguida"
		}
		
		//verificar se existe alguma struct com pid = 0
		//se sim, fazer memset a 0's para essa struct individual
		//e reajustar o array
		for(i=0; i < *us_players_num; i++)
		{
			if(us_players[i].user_data_pid == 0)
			{//cliente que j� saiu
				printf(ASC_C_GREEN " - Utilizador %s desligou-se..." ASC_C_NORMAL, us_players[i].user_data_uname);
				
				//se for pacman e jogo estiver a correr
				//terminar o jogo
				
				memset(&(us_players[i]), 0, sizeof(user_data)); //memset apenas
				//ao membro do array em quest�o
				
				//puxar todos os elementos � frente deste, 1 casa para tr�s
				for(j = i; j < (*us_players_num)-1; j++)
				{
					us_players[j] = us_players[j+1]; //elemento atual toma os valores do seguinte
					//repetir at� ao penultimo elemento (para nao aceder a ultimo+1 -> segmentation fault)
				}
				//como o ultimo elemento j� foi movido 1 casa para tr�s
				memset(&(us_players[(*us_players_num)-1]), 0, sizeof(user_data)); //limp�-lo
				//como s� � removido 1 elemento de cada vez
				(*us_players_num)--; //decrementar o numero de clientes ligados
				printf(ASC_C_GREEN "dados atualizados" ASC_C_NORMAL "\n");
			}
			//else..continue
		}
		
		
	}//fim while	
}

//--------------------------------COMANDOS CLIENTE--------------------------------//
//tratar comandos recebidos do cliente que falou para o fifo do server
void trata_comando_cliente(user_data *user_struct_temp, user_data *us_players, int *us_players_num, int *isGameRunning, char game_map[MAP_X][MAP_Y])
{
	int i, j;
	int fifo_cli;
	char msgToSend[MAX_CMD]; //mensagem a enviar de volta
	
	printf(" - Entrei 1 %s order %d...\n", user_struct_temp->user_data_cmd, user_struct_temp->user_data_order);
	fflush(stdout);
	if(strcmp(user_struct_temp->user_data_cmd, "ENTRAR") == 0) //comando PLAY vindo dum cliente
	{
		//strcpy(msgToSend, "gameDown");
		int seguir=0;
		if(*isGameRunning!=0) //JOGO A CORRER
		{
			seguir = 1;
		}
		else
		{//INICIAR JOGO
			if(configurar==0){
				seguir=2;
				configurar=1;
			}
		}
		if(seguir != 0){
			for(i = 0; i < *us_players_num; i++)
			{
				if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
				{
					us_players[i].next_menu = 1;
					if(seguir == 2)
						us_players[i].criador = 1;
					else
						for(j =0;j<18;j++)
							us_players[i].escolha_pos[j] = pos_ocupadas[j];
				}
			}
		}
	}

	if(strcmp(user_struct_temp->user_data_cmd, "CONFIG") == 0) //comando PLAY vindo dum cliente
	{
		//printf("-1");
		//fflush(stdout);
		strcpy(msgToSend, "gameDown");
		printf("-- %d - %d --", user_struct_temp->n_defesas, user_struct_temp->n_atacantes);
		//fflush(stdout);
		defesas = user_struct_temp->n_defesas;
		//printf("0.1");
		//fflush(stdout);
		atacantes = user_struct_temp->n_atacantes;
		//printf("0.2\n");
		//fflush(stdout);
		for(i=4;i>4-(4-defesas);i--){
		//	printf("%d",i);
		//	fflush(stdout);
			pos_ocupadas[i]=-1;
			pos_ocupadas[i+9]=-1;
		}
		printf("1\n");
		fflush(stdout);
		for(i=8;i>8-(4-atacantes);i--){
		//	printf("%d",i);
		//	fflush(stdout);
			pos_ocupadas[i]=-1;
			pos_ocupadas[i+9]=-1;
		}
		//printf("2");
		//fflush(stdout);
		for(i = 0; i < *us_players_num; i++)
		{
			if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
			{
				us_players[i].next_menu = 1;
				printf("3");
				fflush(stdout);
				for(j =0;j<18;j++)
							us_players[i].escolha_pos[j] = pos_ocupadas[j];
				printf("4");
				fflush(stdout);
			}
		}
	}

	if(strcmp(user_struct_temp->user_data_cmd, "PLAY") == 0) //comando PLAY vindo dum cliente
	{
		int temp =0;
		strcpy(msgToSend, "gameUp");
		if(*isGameRunning!=0) //JOGO A CORRER
		{
			//VER SE A POS EST� OCUPADA
			if(pos_ocupadas[user_struct_temp->user_data_order - 1]==0){//POS LIVRE
				temp=1;
				printf(" - Entrei %d...\n",user_struct_temp->user_data_order);
				fflush(stdout);
				playerOrder++;//incrementa numero de jogadores
				
			}
			else{ //POS OCUPADA
				for(i = 0; i < *us_players_num; i++)
				{
					if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
					{
						us_players[i].user_data_order=0;
						for(j =0;j<18;j++)
							us_players[i].escolha_pos[j] = pos_ocupadas[j];
					}
				}
			}
		}
		else
		{//INICIAR JOGO
			temp=1;
			printf(" - Entrei %d...",user_struct_temp->user_data_order);
			fflush(stdout);

			*isGameRunning = 1; //um jogo est� agora a correr
			//encontrar o jogador que criou o jogo
			//e colocar lhe user_data_order a 1 (pacman)
			playerOrder = 1;//numero de jogadores (inicialmente a 1)
			//pacman_lives = 3;
			
			for(i = 0; i < *us_players_num; i++)
			{
				if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
				{
					us_players[i].user_data_order = user_struct_temp->user_data_order;
					//us_players[i].posx = init_xy_Jogador[user_struct_temp->user_data_order-1][0];
					//us_players[i].posy = init_xy_Jogador[user_struct_temp->user_data_order-1][1];
					//us_players[i].ghosts = 0;
					//us_players[i].food = 0;
					us_players[i].user_data_ingame = 1;
				}
			}
		}
		if(temp == 1){
			pos_ocupadas[user_struct_temp->user_data_order - 1]=1;
			for(i = 0; i < *us_players_num; i++)
			{
				if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
				{
					us_players[i].posx = init_xy_Jogador[user_struct_temp->user_data_order-1][0];
					us_players[i].posy = init_xy_Jogador[user_struct_temp->user_data_order-1][1];
					
					us_players[i].user_data_order = user_struct_temp->user_data_order; //posi��o do jogador
					us_players[i].ghosts = 0;
					//us_players[i].food = 0;
					us_players[i].edible = 0;
					us_players[i].user_data_ingame = 1;
				}
			}
		}	
	}

	if(strcmp(user_struct_temp->user_data_cmd, "QUIT") == 0) //comando QUIT vindo dum cliente
	{
		for(i = 0; i < *us_players_num; i++)
		{
			if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
			{
				break; //quebrar o ciclo quando encontrado o jogador em quest�o
			}
		}
		
		//se o jogador tiver order == 1 (� o pacman) o jogo termina
		//sen�o, o fantasma que controlava simplesmente voltar� ao controlo da IA (caso implementado)
		if(us_players[i].user_data_order == 1) //se for o pacman
		{
			//terminar jogo
			*isGameRunning = 0;
			strcpy(msgToSend, "gameDown");
			us_players[i].user_data_order = 0;
			count_eaten = 0;
			//pacman_lives = PACMAN_LIVES_DEFAULT;
			//repor o mapa
			//memcpy(*game_map, *backup_game_map, MAP_X*MAP_Y);
			
			for(i = 0; i < *us_players_num; i++)
			{
				us_players[i].user_data_order = 0;
				//us_players[i].food = 0;
				us_players[i].ghosts = 0;
				us_players[i].user_data_ingame = 0;
				us_players[i].pacmans = 0;
				us_players[i].edible = 0;
				//us_players[i].isAtopFood = 'N';
			}
		}
		else
		{
			//remover o cliente do jogo (manter conex�o ao servidor)
			//e passar o controlo do fantasma � IA
			playerOrder--;
			//encontrar o cliente
			for(i = 0; i < *us_players_num; i++)
			{
				if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
				{
					break; //quebrar o ciclo quando encontrado o jogador em quest�o
				}
			}
			
			
			//us_players[i].food = 0;
			us_players[i].ghosts = 0;
			us_players[i].user_data_ingame = 0;
			us_players[i].pacmans = 0;
			us_players[i].edible = 0;
			sprintf(msgToSend, "%dleft", us_players[i].user_data_order);
			us_players[i].user_data_order = 0;
		}	
	}
	if(strcmp(user_struct_temp->user_data_cmd, "KEYUP") == 0) 
	{
		for(i = 0; i < *us_players_num; i++)
		{
			if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
			{
				break; //quebrar o ciclo quando encontrado o jogador em quest�o
			}
		}
		if(init_xy_Jogador[user_struct_temp->user_data_order-1][1]!=0)
			init_xy_Jogador[user_struct_temp->user_data_order-1][1]--;
		
	}
	if(strcmp(user_struct_temp->user_data_cmd, "KEYDOWN") == 0)
	{
		for(i = 0; i < *us_players_num; i++)
		{
			if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
			{
				break; //quebrar o ciclo quando encontrado o jogador em quest�o
			}
		}
		if(init_xy_Jogador[user_struct_temp->user_data_order-1][1]!=20)
			init_xy_Jogador[user_struct_temp->user_data_order-1][1]++;
		
	}
	if(strcmp(user_struct_temp->user_data_cmd, "KEYLEFT") == 0)
	{
		for(i = 0; i < *us_players_num; i++)
		{
			if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
			{
				break; //quebrar o ciclo quando encontrado o jogador em quest�o
			}
		}
		if(init_xy_Jogador[user_struct_temp->user_data_order-1][0]!=0)
			init_xy_Jogador[user_struct_temp->user_data_order-1][0]--;
		
	}
	if(strcmp(user_struct_temp->user_data_cmd, "KEYRIGHT") == 0) 
	{
		for(i = 0; i < *us_players_num; i++)
		{
			if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
			{
				break; //quebrar o ciclo quando encontrado o jogador em quest�o
			}
		}
		if(init_xy_Jogador[user_struct_temp->user_data_order-1][0]!=50)
			init_xy_Jogador[user_struct_temp->user_data_order-1][0]++;
	}

	int comp_temp = user_struct_temp->user_data_cmd[0]-'0';
	if(comp_temp >=0 && comp_temp <=9){
		if(comp_temp>=6)comp_temp--;
		int aux;
		if(us_players[i].user_data_order<=9)aux=0+comp_temp;
		else aux=9+comp_temp;
		if(pos_ocupadas[aux]==0){
			for(i = 0; i < *us_players_num; i++)
			{
				if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
				{
					break; //quebrar o ciclo quando encontrado o jogador em quest�o
				}
			}
			pos_ocupadas[us_players[i].user_data_order-1]=0;
			us_players[i].user_data_order=aux+1;
			pos_ocupadas[aux]=1;
		}
	}
	
	//verifica��es de fim de jogo
	/*
	if(count_total_food - count_eaten == 0)
	{
		memset(msgToSend, 0, sizeof(msgToSend));
		//pacman venceu
		strcpy(msgToSend, "pacmanWon");
		//terminar jogo
		*isGameRunning = 0;
		count_eaten = 0;
		pacman_lives = PACMAN_LIVES_DEFAULT;
		//repor o mapa
		memcpy(*game_map, *backup_game_map, MAP_X*MAP_Y);
		
		for(i = 0; i < *us_players_num; i++)
		{
			us_players[i].user_data_order = 0;
			us_players[i].food = 0;
			us_players[i].ghosts = 0;
			us_players[i].user_data_ingame = 0;
			us_players[i].pacmans = 0;
			us_players[i].edible = 0;
			us_players[i].isAtopFood = 'N';
		}
	}
	if(pacman_lives == 0)
	{
		memset(msgToSend, 0, sizeof(msgToSend));
		//fantasmas venceram
		strcpy(msgToSend, "ghostsWon");
		//terminar jogo
		*isGameRunning = 0;
		count_eaten = 0;
		pacman_lives = PACMAN_LIVES_DEFAULT;
		//repor o mapa
		memcpy(*game_map, *backup_game_map, MAP_X*MAP_Y);
		
		for(i = 0; i < *us_players_num; i++)
		{
			us_players[i].user_data_order = 0;
			us_players[i].food = 0;
			us_players[i].ghosts = 0;
			us_players[i].user_data_ingame = 0;
			us_players[i].pacmans = 0;
			us_players[i].edible = 0;
			us_players[i].isAtopFood = 'N';
		}
	}
	*/
	for(i = 0; i < *us_players_num; i++)
	{	
		//reset � mensagem actualmente em user_data_cmd
		memset(us_players[i].user_data_cmd, 0, MAX_CMD);
		//colocar msgToSend em user_data_cmd
		strcpy(us_players[i].user_data_cmd, msgToSend);		
		//colocar o mapa do jogo
		memcpy(*us_players[i].user_data_map, *game_map, MAP_X*MAP_Y);
		update_pos(us_players[i].user_data_map);
		//abrir o fifo respectivo e escrever para l�
		fifo_cli = open(us_players[i].user_data_fifo, O_WRONLY);
		write(fifo_cli, &us_players[i], sizeof(user_data));
		close(fifo_cli);
	}
}

//--------------------------------COMANDOS SERVIDOR--------------------------------//
void trata_stdin(char * fname_users, user_data *us_players, int *us_players_num, int *isGameRunning, char game_map[MAP_X][MAP_Y])
{
	char comando[MAX_CMD]; //string que guarda comandos
	char *cmd, *arg1, *arg2;
	int i;
	int fifo_cli;
	
	FILE *f_users; //ficheiro de nomes/passwds
	
	fgets(comando, sizeof(comando), stdin);
	comando[strlen(comando) - 1] = '\0'; //retirar line break posto por fgets()
	
	//maior comando poss�vel cont�m 3 palavras (1 comando + 2 args)
    cmd = strtok(comando, " ");
    arg1 = strtok(NULL, " ");
    arg2 = strtok(NULL, " ");
	
    //para evitar segmentation fault (em comandos que s� t�m 1 ou nenhum args),
	//alocar espa�o de 1*char e atribuir um valor de string vazia
    if(cmd == NULL)
    {
        cmd = malloc(sizeof(char));
        cmd = '\0';
    }
    if(arg1 == NULL)
    {
        arg1 = malloc(sizeof(char));
		arg1 = '\0';
    }
    if(arg2 == NULL)
    {
        arg2 = malloc(sizeof(char));
		arg2 = '\0';
    }
	
	//caso stdin seja apenas '\n' -> um click em enter
	//� convertido para \0 e � verificado de seguida
	//para evitar segmentation fault em strcmp()
	if(cmd == '\0')
	{
		return;
	}
	
	//verificar se o comando � v�lido
	if(strcmp(cmd, "add") == 0) //add username password
	{
		//quaisquer que sejam os conte�dos de username
		//e password, nao h� v�lido/inv�lido neste comando
		//com excep��o de strings vazias
		if(arg1 == '\0' || arg2 == '\0')
		{
			printf(ASC_C_RED " - Sintaxe invalida (use help)" ASC_C_NORMAL);
			printf("\n");
		}
		else
		{
			f_users = fopen(fname_users, "a"); //abrir append text
			fprintf(f_users, "%s:%s;\n", arg1, arg2);
			fclose(f_users);
			printf(ASC_C_GREEN " - Utilizador %s adicionado com sucesso" ASC_C_NORMAL, arg1);
			printf("\n");
		}
	}
	else if(strcmp(cmd, "users") == 0) //users
	{
		if(*us_players_num == 0)
			printf(" - Nenhum jogador ligado\n");
		for(i = 0; i < *us_players_num; i++)
		{
			if(*isGameRunning)
			{//jogo a decorrer
				if(us_players[i].user_data_order != 0)
					printf(" - " ASC_C_GREEN "%s" ASC_C_NORMAL ": a jogar\n", us_players[i].user_data_uname);	
				else
					printf(" - " ASC_C_GREEN "%s" ASC_C_NORMAL ": fora de jogo\n", us_players[i].user_data_uname);
			}
			else
			{//NOT jogo a decorrer
				printf(" - " ASC_C_GREEN "%s" ASC_C_NORMAL ": fora de jogo\n", us_players[i].user_data_uname);
			}
		}
	}
	else if(strcmp(cmd, "kick") == 0) //kick username
	{
		if(arg1 == '\0')
		{
			printf(ASC_C_RED " - Sintaxe invalida (use help)" ASC_C_NORMAL);
			printf("\n");
		}
		else
		{
			for(i = 0; i < *us_players_num; i++)
			{
				//encontrar o jogador cujo uname = arg1
				if(strcmp(us_players[i].user_data_uname, arg1) == 0)
				{
					//abrir fifo do cliente em quest�o e escrever "kicked"
					memset(us_players[i].user_data_cmd, 0, MAX_CMD);
					strcpy(us_players[i].user_data_cmd, "kicked");
					fifo_cli = open(us_players[i].user_data_fifo, O_WRONLY);
					write(fifo_cli, &us_players[i], sizeof(user_data));
					close(fifo_cli);
				}
			}
		}
	}
	else if(strcmp(cmd, "game") == 0) //game
	{
		if(*isGameRunning)
		{//jogo a decorrer
			
			printf(" Migalhas por apanhar: %d\n", (count_total_food - count_eaten));
			printf(" Vidas do Pacman: %d\n", pacman_lives);
			
			for(i = 0; i < *us_players_num; i++)
			{
				//if(us_players[i].user_data_order != 0)
					//printf(" - " ASC_C_GREEN "%s" ASC_C_NORMAL "[%d pts]: a jogar como ", us_players[i].user_data_uname, calc_points(us_players[i].food, us_players[i].ghosts, us_players[i].pacmans, us_players[i].user_data_order));
				switch(us_players[i].user_data_order)
				{
					case 1:
						printf("Pacman\n");
						break;
					case 2:
						printf("Fantasma Ciano\n");
						break;
					case 3:
						printf("Fantasma Vermelho\n");
						break;
					case 4:
						printf("Fantasma Verde\n");
						break;
					case 5:
						printf("Fantasma Azul\n");
						break;
				}
			}
		}
		else
		{//NOT jogo a decorrer
			printf(" - Nao existe jogo a decorrer\n");
		}
	}
	else if(strcmp(cmd, "shutdown") == 0) //shutdown
	{
		//enviar "server shutdown" para todos os clientes registados
		for(i = 0; i < *us_players_num; i++)
		{
			memset(us_players[i].user_data_cmd, 0, MAX_CMD);
			strcpy(us_players[i].user_data_cmd, "server shutdown");
			fifo_cli = open(us_players[i].user_data_fifo, O_WRONLY);
			write(fifo_cli, &us_players[i], sizeof(user_data));
			close(fifo_cli);
		}
		printf(" - Servidor a terminar...\n");
		terminate();
	}
	else if(strcmp(cmd, "help") == 0) //help (mostra comandos poss�veis)
	{
		printf(" - " ASC_C_CYAN "add" ASC_C_YELLOW " username password" ASC_C_NORMAL " -> adicionar um user\n");
		printf(" - " ASC_C_CYAN "users" ASC_C_NORMAL " -> listar users\n");
		printf(" - " ASC_C_CYAN "kick" ASC_C_YELLOW " username" ASC_C_NORMAL " -> terminar conexao com user\n");
		printf(" - " ASC_C_CYAN "game" ASC_C_NORMAL " -> mostra info do jogo a decorrer\n");
		printf(" - " ASC_C_CYAN "shutdown" ASC_C_NORMAL " -> encerrar servidor\n");
		printf(" - " ASC_C_CYAN "map" ASC_C_YELLOW " nome-ficheiro" ASC_C_NORMAL " -> usar mapa diferente\n");
		printf(" - " ASC_C_CYAN "clear" ASC_C_NORMAL " -> limpar ecra\n");
		//printf(" - " ASC_C_CYAN "history" ASC_C_NORMAL " -> mostra historico do servidor\n");
	}
	else if(strcmp(cmd, "clear") == 0) //clear (limpa a consola)
	{
		limpa_consola();
	}
	//else if(strcmp(cmd, "history") == 0) //history (mostra historico de output)
	//{
	//	printf("COMANDO HISTORY\n");
	//}
	else //comando inv�lido
	{
		printf(ASC_C_RED " - Comando invalido (use help)" ASC_C_NORMAL);
		printf("\n");
	}
	
	
	fflush(stdin); //limpar stdin
	//limpa_consola();
}

//--------------------------------LOGIN--------------------------------//
int verifica_registo(user_data *us_players, int *us_players_num, user_data *us_temp)
{
	int i;
	
	//percorrer o array de structs de jogadores (us_players)
	//e verificar em cada struct
	for(i = 0; i < *us_players_num; i++)
	{
		//compara uname e upass
		if(strcmp(us_players[i].user_data_uname, us_temp->user_data_uname) == 0 &&
		strcmp(us_players[i].user_data_upass, us_temp->user_data_upass) == 0)
			return 1;
	}
	return 0;
}
void falhou_login(user_data *us_players, int *us_players_num, user_data *us_temp, int situation)
{
	switch(situation)
	{
		//max de jogadores atingido
		case 0:
			printf(ASC_C_GREEN " - Utilizador %s tentou ligar-se...limite de jogadores atingido" ASC_C_NORMAL, us_temp->user_data_uname);
			printf("\n");
			//para o cliente tomar conhecimento colocar MAX no seu username e password
			memset(us_temp->user_data_uname, 0, sizeof(us_temp->user_data_uname));
			memset(us_temp->user_data_upass, 0, sizeof(us_temp->user_data_upass));
			//colocar MAX nos 2 campos
			strcpy(us_temp->user_data_uname, "MAX");
			strcpy(us_temp->user_data_upass, "MAX");
			break;
		
		//login inv�lido
		case 1:
			printf(ASC_C_GREEN " - Utilizador %s tentou ligar-se...login invalido" ASC_C_NORMAL, us_temp->user_data_uname);
			printf("\n");
			//para o cliente tomar conhecimento colocar FAIL no seu username e password
			memset(us_temp->user_data_uname, 0, sizeof(us_temp->user_data_uname));
			memset(us_temp->user_data_upass, 0, sizeof(us_temp->user_data_upass));
			//colocar FAIL nos 2 campos
			strcpy(us_temp->user_data_uname, "FAIL");
			strcpy(us_temp->user_data_upass, "FAIL");
			break;
		
		//user j� est� ligado
		case 2:
			printf(ASC_C_GREEN " - Utilizador %s tentou ligar-se...ja existe" ASC_C_NORMAL, us_temp->user_data_uname);
			printf("\n");
			//para o cliente tomar conhecimento colocar EXISTS no seu username e password
			memset(us_temp->user_data_uname, 0, sizeof(us_temp->user_data_uname));
			memset(us_temp->user_data_upass, 0, sizeof(us_temp->user_data_upass));
			//colocar EXISTS nos 2 campos
			strcpy(us_temp->user_data_uname, "EXISTS");
			strcpy(us_temp->user_data_upass, "EXISTS");
			break;
	}
}
void faz_login(user_data *us_players, int *us_players_num, user_data *us_temp)
{
	int i;
	if(*us_players_num == MAX_JOG) //numero de jogadores no maximo
	{
		falhou_login(us_players, us_players_num, us_temp, 0);
		return;
	}
	//caso esteja tudo OK, completar login
	
	//colocar toda a informa��o no array de users na posi��o correta
	strcpy(us_players[(*us_players_num)].user_data_fifo, us_temp->user_data_fifo);
	strcpy(us_players[(*us_players_num)].user_data_uname, us_temp->user_data_uname);
	strcpy(us_players[(*us_players_num)].user_data_upass, us_temp->user_data_upass);
	us_players[(*us_players_num)].user_data_pid = us_temp->user_data_pid;
	strcpy(us_players[(*us_players_num)].user_data_cmd, ""); //como foi completado o login
	//e user_data_cmd continha a string "login", registar no array de users uma string vazia
	//para evitar possiveis futuras confusoes
	us_players[(*us_players_num)].user_data_ingame = 0; //nao est� em jogo
	
	//incrementar o numero de jogadores ligados
	(*us_players_num)++;

	printf(ASC_C_GREEN " - Utilizador %s tentou ligar-se...login aceite" ASC_C_NORMAL, us_temp->user_data_uname);
	printf("\n");
	//para o cliente tomar conhecimento colocar OK no seu username e password
	memset(us_temp->user_data_uname, 0, sizeof(us_temp->user_data_uname));
	memset(us_temp->user_data_upass, 0, sizeof(us_temp->user_data_upass));
	//colocar OK nos 2 campos
	strcpy(us_temp->user_data_uname, "OK");
	strcpy(us_temp->user_data_upass, "OK");
	for(i =0;i<18;i++)
		us_temp->escolha_pos[i] = pos_ocupadas[i];
}
void verifica_login(user_data *us_players, int *us_players_num, user_data *us_temp, char * fname_users)
{
	FILE *f_users; //ficheiro de nomes/passwds
	char line[MAX_CMD], *uname, *upass;
	int uname_found = 0, upass_found = 0;
	int fifo_cli_temp; //file descriptor do cliente que "falou"
	f_users = fopen(fname_users, "r"); //abrir read text
	while(fgets(line, sizeof(line), f_users) != NULL) //enquanto nao chegar ao fim do ficheiro
	{												  //ler linha inteira
		uname = strtok(line, ":"); //retirar username
		
		upass = strtok(NULL, ";"); //retirar password
		
		//comparar uname ao us_temp->uname e upass ao us_temp->upass
		if(strcmp(uname, us_temp->user_data_uname) == 0 && strcmp(upass, us_temp->user_data_upass) == 0)
		{
			//username aceite
			uname_found = 1;
			//password aceite
			upass_found = 1;
			break;
		} //senao, continuar a procurar uname's e upass's
		
		
	}
	fclose(f_users);
	
	if(uname_found && upass_found)
		faz_login(us_players, us_players_num, us_temp);
	else
		falhou_login(us_players, us_players_num, us_temp, 1); //login inv�lido
	
	memset(us_temp->user_data_cmd, 0, MAX_CMD);
	//escrever de volta para o cliente que "falou"
	fifo_cli_temp = open(us_temp->user_data_fifo, O_WRONLY); //abrir para escrita apenas
	write(fifo_cli_temp, us_temp, sizeof(*us_temp));
	close(fifo_cli_temp);
}

//--------------------------------FIFO SERVER--------------------------------//
//fun��o que l� o conte�do do fifo do servidor
//apenas 1 tipo de dados � trocado entre 
// cliente - servidor -> a estrutura user_data
void trata_fifo_server(int fifo_serv, user_data *us_players, int *us_players_num, char * fname_users, int *isGameRunning, char game_map[MAP_X][MAP_Y])
{
	//estrutura de dados trocada entre programas
	user_data user_struct_temp;
	int fifo_cli_temp; 

	read(fifo_serv, &user_struct_temp, sizeof(user_struct_temp));

	if(verifica_registo(us_players, us_players_num, &user_struct_temp) && strcmp(user_struct_temp.user_data_cmd, "login") != 0)
	{	//JOGO
		//COMANDOS CLIENTES
		trata_comando_cliente(&user_struct_temp, us_players, us_players_num, isGameRunning, game_map);
	}
	else if(verifica_registo(us_players, us_players_num, &user_struct_temp) && strcmp(user_struct_temp.user_data_cmd, "login") == 0)
	{	//ERRO LOGIN
		//caso uname:passwd usados j� estejam ligados
		//mas o cliente que enviou, pretende fazer login -> uname j� est� ligado
		falhou_login(us_players, us_players_num, &user_struct_temp, 2);
		//escrever logo para o cliente
		fifo_cli_temp = open(user_struct_temp.user_data_fifo, O_WRONLY); //abrir para escrita apenas
		write(fifo_cli_temp, &user_struct_temp, sizeof(user_struct_temp));
		close(fifo_cli_temp);
	}
	else //LOGIN
	{
		verifica_login(us_players, us_players_num, &user_struct_temp, fname_users);
	}
}

void main(int argc, char *argv[])
{
	limpa_consola();
	
	//alterar umask do servidor
	umask(0000);
	
	//registar handles dos sinais
	signal(SIGINT, signal_handle);
	signal(SIGUSR1, signal_handle);
	
	//file descriptors
	fd_set fd_read;
	int fd_return;
	char fname_users[MAX_LOGIN]; //ficheiro com login's
	int fifo_serv; //file descriptor retornado de open()
	
	//timeval
	struct timeval tv;	//timeout for select()
	
	int isGameRunning = 0; //nao h� jogo nenhum a decorrer inicialmente
	user_data us_players[MAX_JOG];//array de estruturas de jogadores
	int us_players_num = 0; // numero de jogadores registados
	
	int i, j;
	//mapa do jogo (vers�o do servidor -> sempre atualizada -> fonte de copia para clientes)
	char game_map[MAP_X][MAP_Y];
	
	if(argc != 2) //caso nao seja especificado o nome do ficheiro de usernames/passwords
	{
		printf(ERR_SYNTAX);
		return; //fechar o programa
	}
	else
	{
		strcpy(fname_users, argv[1]); //guardar o nome do ficheiro dos login's
		strcat(fname_users, ".usr"); //adicionar uma extens�o espec�fica
	}
	
	printf(" A abrir servidor (pid:%d)\n", getpid());
	
	//criar fifo do servidor//
	if(mkfifo(FSERV, 0777) != 0)
	{
		perror(ERR_DEFAULT);
		return; //sair do programa pois foi imposs�vel criar fifo OU este j� existe (impede 2 servidores de correr em simult�neo)
	}
	printf(" -FIFO criado\n");
	
	//verificar se o ficheiro de login's especificado existe
	//senao -> cri�-lo
	if(access(fname_users, F_OK) == 0)
	{
		printf(" -Ficheiro encontrado\n");
	}
	else
	{
		//nao existe
		creat(fname_users, 0666); //criar ficheiro
		printf(" -Ficheiro inexistente. A criar...\n -Ficheiro criado\n");
	}
	//garantida a existencia do ficheiro de login's neste ponto
	
	//criar o mapa original
	create_game_map(game_map);
	
	sleep(2);
	limpa_consola();
	
	fifo_serv = open(FSERV, O_RDWR);
	if(fifo_serv == -1)
	{
		perror(ERR_DEFAULT);
		terminate();
	}
	
	//para orientar o utilizador do servidor
	printf(" - Use " ASC_C_CYAN "help" ASC_C_NORMAL " para ver os comandos disponiveis\n");
	
	//ciclo base do servidor
	while(1)
	{
		//////////////////select//////////////////
		tv.tv_sec = 0;		
		tv.tv_usec = 1000;	//1ms
		FD_ZERO(&fd_read);
	
		//descriptors a ter em conta
		FD_SET(0, &fd_read); //stdin
		FD_SET(fifo_serv, &fd_read); //fifo server
		
		
		//espera 1ms por altera��es no fd de stdin ou no fifo
		fd_return = select(fifo_serv+1, &fd_read, NULL, NULL, &tv);
		if(fd_return == -1) // ERRO
			perror(ERR_DEFAULT);
		else
		{
			if(FD_ISSET(0, &fd_read)) //stdin tem dados
			{
				//COMANDOS SERVIDOR
				trata_stdin(fname_users, us_players, &us_players_num, &isGameRunning, game_map); //tratar o input
			}
			if(FD_ISSET(fifo_serv, &fd_read)) //fifo server tem dados
			{
				//DADOS JOGADORES
				trata_fifo_server(fifo_serv, us_players, &us_players_num, fname_users, &isGameRunning, game_map); //ler do fifo do servidor	
			}
		}
		//descriptors a ter em conta (eliminar no fim de cada itera��o)
		FD_CLR(fifo_serv, &fd_read); //fifo server
		FD_CLR(0, &fd_read); //stdin 
		//verificar constantemente o estado dos clientes (se continua ligado ou nao)
		verify_connected_clients(us_players, &us_players_num);
	}
	
	
	//fechar o programa corretamente
	terminate();	
}