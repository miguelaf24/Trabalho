#include "header.h"
#define ERR_DEFAULT "ERRO "

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

void main(int argc, char*argv[]){
	char str[90];
	int t, aux, fd_resp, fd; 
	char fname_users[20]; 
	USER user_struct;
	user_data us_players[10];//array de estruturas de jogadores
	int us_players_num = 0;
	//alterar umask do servidor
	umask(0000);
	
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
	t=read(fd, &user_struct,sizeof(user_struct));
	if(t==sizeof(user_struct)){
		printf("Chegou um pedido(%d bytes)",t);
		printf("PEDIDO %d %d\n",user_struct.algo,user_struct.pid); 
		aux = 999; 
		sprintf(str,FIFOCLI,user_struct.pid); 
		//printf("%s",str); 
		fd_resp=open(str,O_WRONLY); 
		t=write(fd_resp,&aux,sizeof(aux)); 
		close(fd_resp); 
		printf("A resposta foi enviada %d bytes\n", t); 
	}
	}while(user_struct.algo!=0); */
	close(fd); 
	unlink(FIFOSERV); 
	printf("A terminar...\n"); 
	
}