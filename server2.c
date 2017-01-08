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


typedef struct{
int tempo, num, num_oc, fim, humano;
}JOGADOR;


int playerOrder = 0;//QUANTIDADE DE JOGADORES

int golos_verde = 0;
int golos_vermelho = 0;
int atacantes = 2;

int defesas = 2;

int num_jog = 0;
//posições iniciais
int init_xy_Jogador[18][3] = {//x , y, dir
	{0, 10, -1},
	{10, 12, -1},
	{10, 7, -1},
	{10, 15, -1},
	{10, 5, -1},
	{20, 12, -1},
	{20, 8, -1},
	{20, 15, -1},
	{20, 6, -1},

	{50, 10, -1},
	{40, 13, -1},
	{40, 7, -1},
	{40, 15, -1},
	{40, 5, -1},
	{30, 12, -1},
	{30, 8, -1},
	{30, 15, -1},
	{30, 6, -1}};
int back_xy_Jogador[18][3] = {//x , y, dir
	{0, 10, -1},
	{10, 12, -1},
	{10, 7, -1},
	{10, 15, -1},
	{10, 5, -1},
	{20, 12, -1},
	{20, 8, -1},
	{20, 15, -1},
	{20, 6, -1},

	{50, 10, -1},
	{40, 13, -1},
	{40, 7, -1},
	{40, 15, -1},
	{40, 5, -1},
	{30, 12, -1},
	{30, 8, -1},
	{30, 15, -1},
	{30, 6, -1}};

int posse_bola = -1;
int posse_bola_ant = -1;
int dir_bola =-1;
int movimento_bola = -1;
int init_bola[2] = {25,10};
int fim = 0;
int chuto = -1;

int pos_ocupadas[18]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//--------threads-------
pthread_mutex_t trinco;



//função de fecho do programa
void terminate()
{
	//remover o fifo do servidor
	pthread_mutex_destroy(&trinco);
	unlink(FSERV);
	
	exit(0);
}

void limpa_consola()
{
	system("clear");//limpar a consola
}

//função que trata sinais
void signal_handle(int sign)
{
	switch (sign)
	{
		case SIGALRM:
			printf(" SIGALRM recebido, fim do jogo...\n");
			//terminate();
			fim=1;
			break;
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
		}
	}
	
	for(i=6;i<15;i++){
		game_map[0][i]=(char)186;
		game_map[MAP_X-1][i]=(char)186;
	}
	game_map[0][14]=200;
	game_map[MAP_X-1][14]=188;
}

void verify_connected_clients(user_data *us_players, int *us_players_num)
{
	if (*us_players_num == 0)
		return;
	
	int i, j;
	int count_ok = 0; //contador que força
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
				//"este cliente já saiu, apagá-lo de seguida"
		}
		
		//verificar se existe alguma struct com pid = 0
		//se sim, fazer memset a 0's para essa struct individual
		//e reajustar o array
		for(i=0; i < *us_players_num; i++)
		{
			if(us_players[i].user_data_pid == 0)
			{//cliente que já saiu
				printf(ASC_C_GREEN " - Utilizador %s desligou-se..." ASC_C_NORMAL, us_players[i].user_data_uname);
				
				memset(&(us_players[i]), 0, sizeof(user_data)); //memset apenas
				//ao membro do array em questão
				
				//puxar todos os elementos á frente deste, 1 casa para trás
				for(j = i; j < (*us_players_num)-1; j++)
				{
					us_players[j] = us_players[j+1]; //elemento atual toma os valores do seguinte
					//repetir até ao penultimo elemento (para nao aceder a ultimo+1 -> segmentation fault)
				}
				//como o ultimo elemento já foi movido 1 casa para trás
				memset(&(us_players[(*us_players_num)-1]), 0, sizeof(user_data)); //limpá-lo
				//como só é removido 1 elemento de cada vez
				(*us_players_num)--; //decrementar o numero de clientes ligados
				printf(ASC_C_GREEN "dados atualizados" ASC_C_NORMAL "\n");
			}
			//else..continue
		}
		
		
	}//fim while	
}

void *move_bola()
{
	int ch;
	do{
		if(init_bola[0]==0 && init_bola[1]>=6 && init_bola[1]<=14){
			golos_vermelho++;
			chuto=-1;
			posse_bola=0;
			init_bola[0]=init_xy_Jogador[0][0]+1;
			init_bola[1]=init_xy_Jogador[0][1];
		}
		else if(init_bola[0]==50 && init_bola[1]>=6 && init_bola[1]<=14){
			golos_verde++;
			chuto=-1;
			posse_bola=9;
			init_bola[0]=init_xy_Jogador[9][0]-1;
			init_bola[1]=init_xy_Jogador[9][1];
		}
		else{
			if(chuto != -1){
				if(chuto==5){
					if(posse_bola<9)dir_bola=3;
					else dir_bola=2;
				}
				else if(chuto==18){
					dir_bola= init_xy_Jogador[posse_bola][3];
				}
				else{
					dir_bola = rand()%4;
				}

				int acertou=0;

				if(posse_bola==0||posse_bola==9){
					acertou= rand()%4;
					if(acertou<1){
						acertou=1;
					}
					else{
						acertou = 0;
					}
				}
				else if(posse_bola<=4||(posse_bola>=10&&posse_bola<=13)){
					acertou= rand()%10;
					if(acertou<8){
						acertou=1;
					}
					else{
						acertou = 0;
					}
				}
				else{
					acertou= rand()%10;
					if(acertou<6){
						acertou=1;
					}
					else{
						acertou = 0;
					}
				}

				if(acertou==0){
					int aux = dir_bola;
					do{
						dir_bola = rand()%4;
					}while(aux==dir_bola);
				}

				chuto = -1;
				posse_bola_ant=posse_bola;
				posse_bola=-1;
				

				switch(dir_bola){
					case 0:
							if(init_xy_Jogador[posse_bola_ant][1]-1>0){//cima
								init_bola[1]=init_xy_Jogador[posse_bola_ant][1]-1;
								init_bola[0]=init_xy_Jogador[posse_bola_ant][0];
							}
							break;
						
					case 1:
						if(init_xy_Jogador[posse_bola_ant][1]+1<20){//baixo
							init_bola[1]=init_xy_Jogador[posse_bola_ant][1]+1;
							init_bola[0]=init_xy_Jogador[posse_bola_ant][0];
						}
						break;
					case 2:
						if(init_xy_Jogador[posse_bola_ant][0]-1>0){//esq
							init_bola[0]=init_xy_Jogador[posse_bola_ant][0]-1;
							init_bola[1]=init_xy_Jogador[posse_bola_ant][1];
						}
						break;
					case 3:
						if(init_xy_Jogador[posse_bola_ant][0]+1<50){//dir
							init_bola[0]=init_xy_Jogador[posse_bola_ant][0]+1;
							init_bola[1]=init_xy_Jogador[posse_bola_ant][1];
						}
						break;
				}
			}
			if(posse_bola == -1){
				if(dir_bola!=-1){
					switch(dir_bola){
						case 0:
							if(init_bola[1]>0)//cima
								init_bola[1]--;
								break;
							
						case 1:
						if(init_bola[1]<20)//baixo
							init_bola[1]++;
							break;
						case 2:
						if(init_bola[0]>0)//esq
							init_bola[0]--;
							break;
						case 3:
						if(init_bola[0]<50)//dir
							init_bola[0]++;
							break;
					}
					if((posse_bola_ant>=1&&posse_bola_ant<=4)||(posse_bola_ant>=10&&posse_bola_ant<=13))
						usleep(400000);
					else
						usleep(300000);
				}
				for(int i= 0; i<18;i++){
					if(pos_ocupadas[i]!=-1){
						if(init_xy_Jogador[i][0]>=init_bola[0]-1&&init_xy_Jogador[i][0]<=init_bola[0]+1){
							if(init_xy_Jogador[i][1]>=init_bola[1]-1&&init_xy_Jogador[i][1]<=init_bola[1]+1){
								posse_bola=i;
								dir_bola=-1;
							}
						}
					}
				}
				
			}
			else{
				for(int i= 0; i<18;i++){
					if(pos_ocupadas[i]!=-1){
						if(posse_bola!=i){
							//printf("%d,%d - %d,%d\n",init_xy_Jogador[i][0],init_xy_Jogador[i][1],init_bola[0],init_bola[1]);
							if((init_xy_Jogador[i][0]>=init_bola[0]-1&&init_xy_Jogador[i][0]<=init_bola[0]+1)&&(init_xy_Jogador[i][1]>=init_bola[1]-1&&init_xy_Jogador[i][1]<=init_bola[1]+1 )){
								posse_bola_ant=posse_bola;
								posse_bola=i;
								dir_bola=-1;
								break;
							}
						}
					}
				}
				if(movimento_bola==0){
					init_bola[1] = init_xy_Jogador[posse_bola][1]-1;
					init_bola[0] = init_xy_Jogador[posse_bola][0];
				}
				if(movimento_bola==1){
					init_bola[1] = init_xy_Jogador[posse_bola][1]+1;
					init_bola[0] = init_xy_Jogador[posse_bola][0];
				}
				if(movimento_bola==2){
					init_bola[1] = init_xy_Jogador[posse_bola][1];
					init_bola[0] = init_xy_Jogador[posse_bola][0]-1;
				}
				if(movimento_bola==3){
					init_bola[1] = init_xy_Jogador[posse_bola][1];
					init_bola[0] = init_xy_Jogador[posse_bola][0]+1;
				}
				usleep(100000);
			}
		}
	}while(fim!=1); 
}

void *move_jogador( void *dados)
{
	JOGADOR *j;
	j = (JOGADOR *) dados;
	do{
		//printf("%d-",j->num_oc);
		if(pos_ocupadas[j->num_oc] == 0){
			if(j->num_oc==0 || j->num_oc == 9){//GR
				if(posse_bola==0||posse_bola==9){
					if(j->num_oc==0)init_xy_Jogador[j->num_oc][2]=3;
					else init_xy_Jogador[j->num_oc][2]=2;
					chuto = 5;
				}
				else if(init_bola[1]>init_xy_Jogador[j->num_oc][1]){
					init_xy_Jogador[j->num_oc][2]=1;
				}
				else if(init_bola[1]<init_xy_Jogador[j->num_oc][1]){
					init_xy_Jogador[j->num_oc][2]=0;
				}
				else{
					if(j->num_oc==0)init_xy_Jogador[j->num_oc][2]=3;
					else init_xy_Jogador[j->num_oc][2]=2;
				}
			}
			else{
				init_xy_Jogador[j->num_oc][2]=rand()%4;
				if(posse_bola==j->num_oc){
					int chutar=rand()%10;
					if(chutar<1)chuto=5;
				}
			} 
		}
		int ocupado = 0;
		switch(init_xy_Jogador[j->num_oc][2]){
			case 0:
				if(init_xy_Jogador[j->num_oc][1]>0){
					for(int k = 0; k<18; k++){
						if(k!=j->num_oc){
							if(init_xy_Jogador[j->num_oc][1]-1==init_xy_Jogador[k][1]&&init_xy_Jogador[j->num_oc][0]==init_xy_Jogador[k][0]){
								ocupado=1;
								break;
							}
						}
					}
					if(ocupado==0)init_xy_Jogador[j->num_oc][1]--;
				}
				break;
			case 1:
				if(init_xy_Jogador[j->num_oc][1]<20){
					for(int k = 0; k<18; k++){
							if(k!=j->num_oc){
								if(init_xy_Jogador[j->num_oc][1]+1==init_xy_Jogador[k][1]&&init_xy_Jogador[j->num_oc][0]==init_xy_Jogador[k][0]){
									ocupado=1;
									break;
								}
							}
						}
					if(ocupado==0)init_xy_Jogador[j->num_oc][1]++;
				}
				
				break;
			case 2:
			if(init_xy_Jogador[j->num_oc][0]>0){
				for(int k = 0; k<18; k++){
						if(k!=j->num_oc){
							if(init_xy_Jogador[j->num_oc][1]==init_xy_Jogador[k][1]&&init_xy_Jogador[j->num_oc][0]-1==init_xy_Jogador[k][0]){
								ocupado=1;
								break;
							}
						}
					}
				if(ocupado==0){
					if(!(pos_ocupadas[j->num_oc]==0 && j->num_oc==9)){
						if(!(j->num_oc == 9 && init_xy_Jogador[j->num_oc][0] == 44))
							init_xy_Jogador[j->num_oc][0]--;
					}
				}
			}
			break;
			case 3:
			if(init_xy_Jogador[j->num_oc][0]<50){
				for(int k = 0; k<18; k++){
						if(k!=j->num_oc){
							if(init_xy_Jogador[j->num_oc][1]==init_xy_Jogador[k][1]&&init_xy_Jogador[j->num_oc][0]+1==init_xy_Jogador[k][0]){
								ocupado=1;
								break;
							}
						}
					}
				if(ocupado==0){
					if(!(pos_ocupadas[j->num_oc]==0 && j->num_oc==0)){
						if(!(j->num_oc == 0 && init_xy_Jogador[j->num_oc][0] == 6))
							init_xy_Jogador[j->num_oc][0]++;
					}
				}
			}
			break;
		}
		if(init_xy_Jogador[j->num_oc][2]!=-1){
			if(j->num_oc==posse_bola)
				movimento_bola=init_xy_Jogador[j->num_oc][2];
			init_xy_Jogador[j->num_oc][2]=-1;
		}
		usleep(j->tempo);
	}while(!fim); 
}

//--------------------------------COMANDOS CLIENTE--------------------------------//
//tratar comandos recebidos do cliente que falou para o fifo do server
void trata_comando_cliente(user_data *user_struct_temp, user_data *us_players, int *us_players_num, int *isGameRunning, char game_map[MAP_X][MAP_Y])
{
	int i, j;
	int fifo_cli;
	char msgToSend[MAX_CMD]; //mensagem a enviar de volta
	
	//printf(" - Entrei 1 %s order %d...\n", user_struct_temp->user_data_cmd, user_struct_temp->user_data_order);
	fflush(stdout);
	if(strcmp(user_struct_temp->user_data_cmd, "ENTRAR") == 0) //comando PLAY vindo dum cliente
	{
		//strcpy(msgToSend, "gameDown");
		int seguir=0;
		printf("isGameRunning %d\n",*isGameRunning);
		if(*isGameRunning!=0) //JOGO A CORRER
		{
			for(i = 0; i < *us_players_num; i++)
			{
				if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
				{
					strcpy(msgToSend, "gameUp");
					us_players[i].jogo_correr = 1;
					us_players[i].next_menu = 1;
					for(j =0;j<18;j++)
						us_players[i].escolha_pos[j] = pos_ocupadas[j];
				}
			}
		}
	}
	if(strcmp(user_struct_temp->user_data_cmd, "PLAY") == 0) //comando PLAY vindo dum cliente
	{
		int temp =0;
		strcpy(msgToSend, "gameUp");
		if(*isGameRunning!=0) //JOGO A CORRER
		{
			//VER SE A POS ESTÁ OCUPADA
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
		if(temp == 1){
			pos_ocupadas[user_struct_temp->user_data_order - 1]=1;
			for(i = 0; i < *us_players_num; i++)
			{
				if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
				{
					us_players[i].posx = init_xy_Jogador[user_struct_temp->user_data_order-1][0];
					us_players[i].posy = init_xy_Jogador[user_struct_temp->user_data_order-1][1];
					
					us_players[i].user_data_order = user_struct_temp->user_data_order; //posição do jogador
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
				break; //quebrar o ciclo quando encontrado o jogador em questão
			}
		}
		
		if(playerOrder == 1) 
		{
			playerOrder = 0;
			//terminar jogo
			fim = 1;
			alarm(0);
			strcpy(msgToSend, "gameDown");
			us_players[i].user_data_order = 0;
		}
		else
		{
			playerOrder--;
			//encontrar o cliente
			for(i = 0; i < *us_players_num; i++)
			{
				if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
				{
					break; //quebrar o ciclo quando encontrado o jogador em questão
				}
			}
			
			us_players[i].user_data_ingame = 0;
			sprintf(msgToSend, "%dleft", us_players[i].user_data_order);
			us_players[i].user_data_order = 0;
		}	
	}
	if(strcmp(user_struct_temp->user_data_cmd, "KEYSPACE") == 0) 
	{
		if(user_struct_temp->user_data_order-1 == posse_bola)
			chuto = 18;
	}
	if(strcmp(user_struct_temp->user_data_cmd, "KEYUP") == 0) 
	{
		init_xy_Jogador[user_struct_temp->user_data_order-1][2]=0;
	}
	if(strcmp(user_struct_temp->user_data_cmd, "KEYDOWN") == 0)
	{	
		init_xy_Jogador[user_struct_temp->user_data_order-1][2]=1;
	}
	if(strcmp(user_struct_temp->user_data_cmd, "KEYLEFT") == 0)
	{
		init_xy_Jogador[user_struct_temp->user_data_order-1][2]=2;
	}
	if(strcmp(user_struct_temp->user_data_cmd, "KEYRIGHT") == 0) 
	{
		init_xy_Jogador[user_struct_temp->user_data_order-1][2]=3;
	}

	int comp_temp = user_struct_temp->user_data_cmd[0]-'0';
	if(comp_temp >=0 && comp_temp <=9){
		if(comp_temp!=5){
			if(comp_temp>=6)comp_temp--;
			int aux;
			if(user_struct_temp->user_data_order<=9)aux=0+comp_temp;
			else aux=9+comp_temp;
			printf("%d\n",us_players[i].user_data_order);
			printf("%d\n",aux);
			if(user_struct_temp->user_data_order!=posse_bola){
				if(pos_ocupadas[aux]==0){
					for(i = 0; i < *us_players_num; i++)
					{
						if(strcmp(us_players[i].user_data_uname, user_struct_temp->user_data_uname) == 0 )
						{
							break; //quebrar o ciclo quando encontrado o jogador em questão
						}
					}
					pos_ocupadas[us_players[i].user_data_order-1]=0;
					us_players[i].user_data_order=aux+1;
					pos_ocupadas[aux]=1;
				}	
			}
			else{
				chuto = aux;
			}
		}
		else chuto = 5;
		
	}
	
	//verificações de fim de jogo
	
	for(i = 0; i < *us_players_num; i++)
	{	
		us_players[i].golos_vermelha=golos_vermelho;
		us_players[i].golos_verde=golos_verde;
		//reset á mensagem actualmente em user_data_cmd
		memset(us_players[i].user_data_cmd, 0, MAX_CMD);
		//colocar msgToSend em user_data_cmd
		strcpy(us_players[i].user_data_cmd, msgToSend);		
		//colocar o mapa do jogo
		memcpy(*us_players[i].user_data_map, *game_map, MAP_X*MAP_Y);
		update_pos(us_players[i].user_data_map);
		//abrir o fifo respectivo e escrever para lá
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
	
	//maior comando possível contém 3 palavras (1 comando + 2 args)
    cmd = strtok(comando, " ");
    arg1 = strtok(NULL, " ");
    arg2 = strtok(NULL, " ");
	
    //para evitar segmentation fault (em comandos que só têm 1 ou nenhum args),
	//alocar espaço de 1*char e atribuir um valor de string vazia
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
	//é convertido para \0 e é verificado de seguida
	//para evitar segmentation fault em strcmp()
	if(cmd == '\0')
	{
		return;
	}
	
	//verificar se o comando é válido
	if(strcmp(cmd, "start") == 0) //start n 
	{
		if(arg1 == '\0')
		{
			printf(ASC_C_RED " - Sintaxe invalida (use help)" ASC_C_NORMAL);
			printf("\n");
		}
		else
		{
			if(*isGameRunning==0){
				int j=0;
				while(j<strlen(arg1)){
					if(arg1[j]>'9'||arg1[j]<'0'){
						j=-1;
						break;
					}
					j++;
				}
				if(j!=-1){
					golos_verde = 0;
					golos_vermelho = 0;
					*isGameRunning=1;
					fim = 0;
					int time_game = atoi(arg1);
					alarm(time_game);
					for(i=0;i<18;i++)pos_ocupadas[i]=0;
					for(i=4;i>4-(4-defesas);i--){
						pos_ocupadas[i]=-1;
						pos_ocupadas[i+9]=-1;
					}
					num_jog=(defesas+atacantes+1)*2;
					for(i=8;i>8-(4-atacantes);i--){
						pos_ocupadas[i]=-1;
						pos_ocupadas[i+9]=-1;
					}
					playerOrder = 0;//numero de clientes em jogo
				}
			}
		}
	}
	else if(strcmp(cmd, "stop") == 0) //stop
	{
		if(*isGameRunning!=0){
			fim = 1;
			printf(" - Fim do jogo \n - " ASC_C_GREEN "%d " ASC_C_RED " %d\n"ASC_C_NORMAL, golos_verde, golos_vermelho);
		}
	}
	else if(strcmp(cmd, "ndefesas") == 0) //start n 
	{
		if(arg1 == '\0' && strlen(arg1)!=1)
		{
			printf(ASC_C_RED " - Sintaxe invalida (use help)" ASC_C_NORMAL);
			printf("\n");
		}
		else
		{
			if(*isGameRunning==0){
				if(arg1[0] <= '4' && arg1[0] >= '1'){
					defesas = atoi(arg1);
				}
			}
		}
	}
	else if(strcmp(cmd, "navancados") == 0) //start n 
	{
		if(arg1 == '\0' && strlen(arg1)!=1)
		{
			printf(ASC_C_RED " - Sintaxe invalida (use help)" ASC_C_NORMAL);
			printf("\n");
		}
		else
		{
			if(*isGameRunning==0){
				if(arg1[0] <= '4' && arg1[0] >= '1'){
					defesas = atoi(arg1);
				}
			}
		}
	}
	else if(strcmp(cmd, "user") == 0) //user username password
	{
		//quaisquer que sejam os conteúdos de username
		//e password, nao há válido/inválido neste comando
		//com excepção de strings vazias
		if(arg1 == '\0' || arg2 == '\0')
		{
			printf(ASC_C_RED " - Sintaxe invalida (use help)" ASC_C_NORMAL);
			printf("\n");
		}
		else
		{
			f_users = fopen(fname_users, "a"); //abrir append text
			fprintf(f_users, "%s %s\n", arg1, arg2);
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
	else if(strcmp(cmd, "red") == 0) //red username
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
					//abrir fifo do cliente em questão e escrever "kicked"
					memset(us_players[i].user_data_cmd, 0, MAX_CMD);
					strcpy(us_players[i].user_data_cmd, "kicked");
					fifo_cli = open(us_players[i].user_data_fifo, O_WRONLY);
					write(fifo_cli, &us_players[i], sizeof(user_data));
					close(fifo_cli);
				}
			}
		}
	}
	else if(strcmp(cmd, "result") == 0) //game
	{
		if(*isGameRunning)
		{//jogo a decorrer
				printf(" - " ASC_C_GREEN "%d " ASC_C_RED " %d\n"ASC_C_NORMAL, golos_verde, golos_vermelho);		
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
	else if(strcmp(cmd, "help") == 0) //help (mostra comandos possíveis)
	{
		printf(" - " ASC_C_CYAN "user" ASC_C_YELLOW " username password" ASC_C_NORMAL " -> adicionar um user\n");
		printf(" - " ASC_C_CYAN "users" ASC_C_NORMAL " -> listar users\n");
		printf(" - " ASC_C_CYAN "red" ASC_C_YELLOW " username" ASC_C_NORMAL " -> terminar conexao com user\n");
		printf(" - " ASC_C_CYAN "result" ASC_C_NORMAL " -> mostra info do jogo a decorrer\n");
		printf(" - " ASC_C_CYAN "shutdown" ASC_C_NORMAL " -> encerrar servidor\n");
		printf(" - " ASC_C_CYAN "clear" ASC_C_NORMAL " -> limpar ecra\n");
	}
	else if(strcmp(cmd, "clear") == 0) //clear (limpa a consola)
	{
		limpa_consola();
	}

	else //comando inválido
	{
		printf(ASC_C_RED " - Comando invalido (use help)" ASC_C_NORMAL);
		printf("\n");
	}
	
	
	fflush(stdin); //limpar stdin
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
			printf(ASC_C_RED " - Utilizador %s tentou ligar-se...limite de jogadores atingido" ASC_C_NORMAL, us_temp->user_data_uname);
			printf("\n");
			//para o cliente tomar conhecimento colocar MAX no seu username e password
			memset(us_temp->user_data_uname, 0, sizeof(us_temp->user_data_uname));
			memset(us_temp->user_data_upass, 0, sizeof(us_temp->user_data_upass));
			//colocar MAX nos 2 campos
			strcpy(us_temp->user_data_uname, "MAX");
			strcpy(us_temp->user_data_upass, "MAX");
			break;
		
		//login inválido
		case 1:
			printf(ASC_C_RED " - Utilizador %s tentou ligar-se...login invalido" ASC_C_NORMAL, us_temp->user_data_uname);
			printf("\n");
			//para o cliente tomar conhecimento colocar FAIL no seu username e password
			memset(us_temp->user_data_uname, 0, sizeof(us_temp->user_data_uname));
			memset(us_temp->user_data_upass, 0, sizeof(us_temp->user_data_upass));
			//colocar FAIL nos 2 campos
			strcpy(us_temp->user_data_uname, "FAIL");
			strcpy(us_temp->user_data_upass, "FAIL");
			break;
		
		//user já está ligado
		case 2:
			printf(ASC_C_RED " - Utilizador %s tentou ligar-se...ja existe" ASC_C_NORMAL, us_temp->user_data_uname);
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
void faz_login(user_data *us_players, int *us_players_num, user_data *us_temp, int *isGameRunning)
{
	int i;
	if(*us_players_num == MAX_JOG) //numero de jogadores no maximo
	{
		falhou_login(us_players, us_players_num, us_temp, 0);
		return;
	}
	//caso esteja tudo OK, completar login
	
	//colocar toda a informação no array de users na posição correta
	strcpy(us_players[(*us_players_num)].user_data_fifo, us_temp->user_data_fifo);
	strcpy(us_players[(*us_players_num)].user_data_uname, us_temp->user_data_uname);
	strcpy(us_players[(*us_players_num)].user_data_upass, us_temp->user_data_upass);
	us_players[(*us_players_num)].user_data_pid = us_temp->user_data_pid;
	strcpy(us_players[(*us_players_num)].user_data_cmd, ""); //como foi completado o login
	//e user_data_cmd continha a string "login", registar no array de users uma string vazia
	//para evitar possiveis futuras confusoes
	us_players[(*us_players_num)].user_data_ingame = 0; //nao está em jogo
	
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
	us_temp->jogo_correr=*isGameRunning;

}
void verifica_login(user_data *us_players, int *us_players_num, user_data *us_temp, char * fname_users, int *isGameRunning)
{
	FILE *f_users; //ficheiro de nomes/passwds
	char line[MAX_CMD], *uname, *upass;
	int uname_found = 0, upass_found = 0;
	int fifo_cli_temp; //file descriptor do cliente que "falou"
	f_users = fopen(fname_users, "r"); //abrir read text
	while(fgets(line, sizeof(line), f_users) != NULL) //enquanto nao chegar ao fim do ficheiro
	{												  //ler linha inteira
		uname = strtok(line, " "); //retirar username
		
		upass = strtok(NULL, "\n"); //retirar password
		
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
		faz_login(us_players, us_players_num, us_temp, isGameRunning);
	else
		falhou_login(us_players, us_players_num, us_temp, 1); //login inválido
	
	memset(us_temp->user_data_cmd, 0, MAX_CMD);
	//escrever de volta para o cliente que "falou"
	fifo_cli_temp = open(us_temp->user_data_fifo, O_WRONLY); //abrir para escrita apenas
	write(fifo_cli_temp, us_temp, sizeof(*us_temp));
	close(fifo_cli_temp);
}

//--------------------------------FIFO SERVER--------------------------------//
//função que lê o conteúdo do fifo do servidor
//apenas 1 tipo de dados é trocado entre 
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
		//caso uname:passwd usados já estejam ligados
		//mas o cliente que enviou, pretende fazer login -> uname já está ligado
		falhou_login(us_players, us_players_num, &user_struct_temp, 2);
		//escrever logo para o cliente
		fifo_cli_temp = open(user_struct_temp.user_data_fifo, O_WRONLY); //abrir para escrita apenas
		write(fifo_cli_temp, &user_struct_temp, sizeof(user_struct_temp));
		close(fifo_cli_temp);
	}
	else //LOGIN
	{
		verifica_login(us_players, us_players_num, &user_struct_temp, fname_users, isGameRunning);
	}
}

void main(int argc, char *argv[])
{
	limpa_consola();
	
	//alterar umask do servidor
	umask(0000);
	
	//registar handles dos sinais
	signal(SIGALRM, signal_handle);
	signal(SIGINT, signal_handle);
	signal(SIGUSR1, signal_handle);
	
	//file descriptors
	fd_set fd_read;
	int isStarted=0;
	int fd_return;
	char fname_users[MAX_LOGIN]; //ficheiro com login's
	int fifo_serv; //file descriptor retornado de open()

	JOGADOR *jog;
	pthread_t *tarefa;
	pthread_t tarefa_bola;
	pthread_mutex_init(&trinco,NULL);
	//timeval
	struct timeval tv;	//timeout for select()
	
	int isGameRunning = 0; //nao há jogo nenhum a decorrer inicialmente
	user_data us_players[MAX_JOG];//array de estruturas de jogadores
	int us_players_num = 0; // numero de jogadores registados
	
	int i, j;
	//mapa do jogo (versão do servidor -> sempre atualizada -> fonte de copia para clientes)
	char game_map[MAP_X][MAP_Y];
	
	if(argc != 2) //caso nao seja especificado o nome do ficheiro de usernames/passwords
	{
		printf(ERR_SYNTAX);
		return; //fechar o programa
	}
	else
	{
		strcpy(fname_users, argv[1]); //guardar o nome do ficheiro dos login's
		strcat(fname_users, ".txt"); //adicionar uma extensão específica
	}
	
	printf(" A abrir servidor (pid:%d)\n", getpid());
	
	//criar fifo do servidor//
	if(mkfifo(FSERV, 0777) != 0)
	{
		perror(ERR_DEFAULT);
		return; //sair do programa pois foi impossível criar fifo OU este já existe (impede 2 servidores de correr em simultâneo)
	}
	printf(" -FIFO criado\n");
	
	//verificar se o ficheiro de login's especificado existe
	//senao -> criá-lo
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
		
		
		//espera 1ms por alterações no fd de stdin ou no fifo
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
		if(isGameRunning!=0){
			if(isStarted==0){

				init_bola[0]=25;
				init_bola[1]=10;

				for(int k = 0; k<18; k++){
					init_xy_Jogador[k][0]=back_xy_Jogador[k][0];
					init_xy_Jogador[k][1]=back_xy_Jogador[k][1];
					init_xy_Jogador[k][2]=-1;
				}

				isStarted=1;

				pthread_create(&tarefa_bola,NULL,&move_bola,NULL);

				jog = malloc(sizeof(JOGADOR)*num_jog);
				if(jog == NULL){
					terminate();
				}
				tarefa = malloc(sizeof(pthread_t)*num_jog);
				if(tarefa==NULL){
					terminate();
				}

				for(i=0;i<num_jog;i++){
					jog[i].fim=0;
					jog[i].num=i;
					jog[i].humano=0;
					if(i<=defesas)jog[i].num_oc=i;
					else if(i<num_jog/2)jog[i].num_oc=i+4-defesas;
					else if(i<=num_jog/2+defesas)jog[i].num_oc=9+i-num_jog/2;
					else jog[i].num_oc=14+i-num_jog/2-defesas-1;
					if((i>0&&i<=defesas)||(i>num_jog/2&&i<=num_jog/2+defesas))
						jog[i].tempo=400000;
					else
						jog[i].tempo=300000;
					pthread_create(&tarefa[i],NULL,&move_jogador,(void *)&jog[i]);
					
				}
			}
			else{
				if(fim == 1){
					isGameRunning = 0;
					isStarted = 0;
					golos_vermelho=0;
					golos_verde=0;
				}
				else
					usleep(10000); 
				for (i=0;i<us_players_num;i++){
					if(us_players[i].user_data_ingame==1)
					{
						if(isGameRunning==0){
							us_players[i].golos_vermelha=golos_vermelho;
							us_players[i].golos_verde=golos_verde;
							us_players[i].next_menu=0;
							us_players[i].jogo_correr=0;
							//reset á mensagem actualmente em user_data_cmd
							memset(us_players[i].user_data_cmd, 0, MAX_CMD);
							//colocar msgToSend em user_data_cmd
							strcpy(us_players[i].user_data_cmd, "gameDown");	
						}
						else{
							us_players[i].golos_vermelha=golos_vermelho;
							us_players[i].golos_verde=golos_verde;
							memcpy(us_players[i].user_data_map, game_map, MAP_X*MAP_Y);
							update_pos(us_players[i].user_data_map);
						}
						//abrir o fifo respectivo e escrever para lá
						int fifo_cli = open(us_players[i].user_data_fifo, O_WRONLY);
						write(fifo_cli, &us_players[i], sizeof(user_data));
						close(fifo_cli);
					}
				}
			}
		}
		//descriptors a ter em conta (eliminar no fim de cada iteração)
		FD_CLR(fifo_serv, &fd_read); //fifo server
		FD_CLR(0, &fd_read); //stdin 
		//verificar constantemente o estado dos clientes (se continua ligado ou nao)
		verify_connected_clients(us_players, &us_players_num);
	}
	
	
	//fechar o programa corretamente
	terminate();	
}