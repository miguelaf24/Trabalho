#include "header.h"
#define ERR_DEFAULT "ERRO "

typedef struct utilizador user_data;

void terminate()
{
	//remover o fifo do servidor
	unlink(FSERV);
	
	exit(0);
}


void verify_connected_clients(user_data *us_players, int *us_players_num)
{
	//sair logo caso nao haja jogadores ligados
	if (*us_players_num == 0)
		return;
	//else...continuar a função
	
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
				
				//se for pacman e jogo estiver a correr
				//terminar o jogo
				
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
		
		*/
	}//fim while
}

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
        return;
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
	
	//verificar se o comando é válido
	if(strcmp(cmd, "add") == 0) //add username password
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
	else if(strcmp(cmd, "game") == 0) //game
	{
		if(*isGameRunning)
		{//jogo a decorrer
			
			//printf(" Migalhas por apanhar: %d\n", (count_total_food - count_eaten));
			//printf(" Vidas do Pacman: %d\n", pacman_lives);
			
			for(i = 0; i < *us_players_num; i++)
			{
				if(us_players[i].user_data_order != 0)
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
	else if(strcmp(cmd, "help") == 0) //help (mostra comandos possíveis)
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
		//limpa_consola();
	}
	//else if(strcmp(cmd, "history") == 0) //history (mostra historico de output)
	//{
	//	printf("COMANDO HISTORY\n");
	//}
	else //comando inválido
	{
		printf(ASC_C_RED " - Comando invalido (use help)" ASC_C_NORMAL);
		printf("\n");
	}
	
	
	fflush(stdin); //limpar stdin
	//limpa_consola();
}

//função que lê o conteúdo do fifo do servidor
//apenas 1 tipo de dados é trocado entre 
// cliente - servidor -> a estrutura user_data
void trata_fifo_server(int fifo_serv, user_data *us_players, int *us_players_num, char * fname_users, int *isGameRunning, char game_map[MAP_X][MAP_Y])
{
	//estrutura de dados trocada entre programas
	user_data user_struct_temp;
	int fifo_cli_temp; //file descriptor do cliente que "falou"
	
	//ler info do fifo do servidor
	read(fifo_serv, &user_struct_temp, sizeof(user_struct_temp));
	//verificar se o nome já está registado
	//	se SIM -> verificar que informações tráz
	//	se NAO -> fazer verificação do login
	if(verifica_registo(us_players, us_players_num, &user_struct_temp) && strcmp(user_struct_temp.user_data_cmd, "login") != 0)
	{
		//função que trata comandos vindos de clientes
		//trata_comando_cliente(&user_struct_temp, us_players, us_players_num, isGameRunning, game_map);
		
		
		
	}
	else if(verifica_registo(us_players, us_players_num, &user_struct_temp) && strcmp(user_struct_temp.user_data_cmd, "login") == 0)
	{
		//caso uname:passwd usados já estejam ligados
		//mas o cliente que enviou, pretende fazer login -> uname já está ligado
		falhou_login(us_players, us_players_num, &user_struct_temp, 2);
		//escrever logo para o cliente
		fifo_cli_temp = open(user_struct_temp.user_data_fifo, O_WRONLY); //abrir para escrita apenas
		write(fifo_cli_temp, &user_struct_temp, sizeof(user_struct_temp));
		close(fifo_cli_temp);
	}
	else //verificar login
	{
		verifica_login(us_players, us_players_num, &user_struct_temp, fname_users);
	}
}

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

//função de quando falha o login
//situation 0 -> maximo de jogadores atingido
//situation 1 -> login inválido
//situation 2 -> uname já está ligado
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
		
		//login inválido
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
		
		//user já está ligado
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
		falhou_login(us_players, us_players_num, us_temp, 1); //login inválido
	
	memset(us_temp->user_data_cmd, 0, MAX_CMD);
	//escrever de volta para o cliente que "falou"
	fifo_cli_temp = open(us_temp->user_data_fifo, O_WRONLY); //abrir para escrita apenas
	write(fifo_cli_temp, us_temp, sizeof(*us_temp));
	close(fifo_cli_temp);
}

//função que completa o login
void faz_login(user_data *us_players, int *us_players_num, user_data *us_temp)
{
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
	us_players[(*us_players_num)].pid = us_temp->pid;
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
}

void main(int argc, char*argv[]){
	char str[90];
	int t, aux, fd_resp, fd; 
	int fifo_serv;
	char fname_users[20]; 
	user_data us_players[10];//array de estruturas de jogadores
	int us_players_num = 0;
	char game_map[MAP_X][MAP_Y];
	//alterar umask do servidor
	umask(0000);
	int isGameRunning = 0;
	
	int fd_return;
	
	fd_set fd_read;
	struct timeval timeval;//timeout para o select()
	
	if(argc != 2) //caso nao seja especificado o nome do ficheiro de usernames/passwords
	{
		printf("Nao foi especificado o nome de ficheiro--->./servidor ficheiro\n");
		return; //fechar o programa
	}
	else
	{
		strcpy(fname_users, argv[1]); //guardar o nome do ficheiro dos login's
		strcat(fname_users, ".txt"); //adicionar uma extensão específica
	}
	
	printf(" A abrir servidor (pid:%d)\n",getpid());
	
	//criacao do fifo 
	if(mkfifo(FIFOSERV,0777)!=0){
		perror(ERR_DEFAULT);
		return;
	}
	printf("FIFO Criado\n");
	if(access(fname_users, F_OK) == 0)
		{
		printf("Ficheiro encontrado\n");
		printf("A abrir ficheiro...\n"); 
		}
	else
	{
		//nao existe
		creat(fname_users, 0666); //criar ficheiro
		printf("Ficheiro inexistente. A criar...\n Ficheiro criado\n");
	}
	
	fd=open(FIFOSERV, O_RDWR);//abertura do ficheiro
	
	if(fd==-1){
		perror(ERR_DEFAULT);
		unlink(FIFOSERV);
		exit(0);
	}
	
	fifo_serv = open(FSERV, O_RDWR | O_NONBLOCK);
	if(fifo_serv == -1)
	{
		perror(ERR_DEFAULT);
		terminate();
	}
	
	while(1){
		
		//////////////////select//////////////////
		timeval.tv_sec = 0;		
		timeval.tv_usec = 1000;	//1ms
		FD_ZERO(&fd_read);
		
		//descriptors a ter em conta
		FD_SET(0, &fd_read); //stdin
		FD_SET(fd, &fd_read); //fifo server
		
		
		//espera 1ms por alterações no fd de stdin ou no fifo
		fd_return = select(fd+1, &fd_read, NULL, NULL, &timeval);
		if(fd_return == -1) // ERRO
			perror(ERR_DEFAULT);
		else
		{
			if(FD_ISSET(0, &fd_read)) //stdin tem dados
			{
				trata_stdin(fname_users, us_players, &us_players_num, &isGameRunning, game_map); //tratar o input
			}
			if(FD_ISSET(fifo_serv, &fd_read)) //fifo server tem dados
			{
				trata_fifo_server(fifo_serv, us_players, &us_players_num, fname_users, &isGameRunning, game_map); //ler do fifo do servidor	
			}
		}
		//descriptors a ter em conta (eliminar no fim de cada iteração)
		FD_CLR(fifo_serv, &fd_read); //fifo server
		FD_CLR(0, &fd_read); //stdin 
		//verificar constantemente o estado dos clientes (se continua ligado ou nao)
		verify_connected_clients(us_players, &us_players_num);
		/**/
	}
	/*
	do{
	t=read(fd, &user_data,sizeof(user_data));
	if(t==sizeof(user_data)){
		printf("Chegou um pedido(%d bytes)",t);
		printf("PEDIDO %d %d\n",user_data.algo,user_data.pid); 
		aux = 999; 
		sprintf(str,FIFOCLI,user_data.pid); 
		//printf("%s",str); 
		fd_resp=open(str,O_WRONLY); 
		t=write(fd_resp,&aux,sizeof(aux)); 
		close(fd_resp); 
		printf("A resposta foi enviada %d bytes\n", t); 
	}
	}while(user_data.algo!=0); */
	close(fd); 
	unlink(FIFOSERV); 
	printf("A terminar...\n"); 
	
}