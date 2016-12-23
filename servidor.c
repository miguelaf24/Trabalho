#include "header.h"

void main(int argc, char*argv[]){
	char str[90];
	int t, aux, fd_resp, fd; 
	char fname_users[20]; 
	USER user_struct;
	
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
	
	
	//criacao do fifo 
	mkfifo(FIFOSERV,0777);
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
	}while(user_struct.algo!=0); 
	close(fd); 
	unlink(FIFOSERV); 
	printf("A terminar...\n"); 
	
}