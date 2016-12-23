#include "header.h"

//função para verificar se o servidor está a correr
int FIFO_SERVIDOR_CORRER()
{
	if(access(FIFOSERV,F_OK) == 0)
	{
		return 1;
	}
	else
	{
		//nao existe
		return 0;
	}
}

int main(void){
	char FIFO_CLIENTE[90];
	int fd_envi, fd_cli,t,fd_resp,resp; 
	USER user_struct; 
	//verificacao do servidor se esta a correr atraves da funcao 
	if (FIFO_SERVIDOR_CORRER()!=0){
		printf("O Servidor foi detetado!\n");
	}
	else{
		printf("O Servidor nao foi detetado!\n" );
		printf("Antes de iniciar o cliente e necessario colocar o servidor a correr!\n "); 
		exit(0); 
	}
	user_struct.pid=getpid();//obtem o pid do cliente 
	
	//criacao do fifo para o cliente 
	sprintf(FIFO_CLIENTE,FIFOCLI,user_struct.pid); 
	if(mkfifo(FIFO_CLIENTE,0777)!=0){
		//sai do programa porque o fifo com o pid anteriormente obtido já existe
		return 0; 
	}
	printf("FIFO do Cliente criado!\n"); 
	
	//código abaixo só para testar comunicação 
	fd_envi=open(FIFOSERV,O_WRONLY); //abertura do ficheiro para comunicação com o servidor
	do{
	printf("Vou mandar algo para o servidor\n"); 
	scanf("%d", &user_struct.algo); 
	//escrita para o servidor 
	t=write(fd_envi,&user_struct,sizeof(user_struct)); 
	printf("Mandei um pedido para o servidor de %d bytes\n", t); 
	
	//receber a resposta por parte do servidor 
	fd_resp=open(FIFO_CLIENTE, O_RDONLY); 
	t=read(fd_resp, &resp,sizeof(resp));
	close(fd_resp); 
	printf("Recebi a resposta do servidor com o seguinte conteudo %d %d\n", resp, t); 
	}while(user_struct.algo!=0);
	
	close(fd_envi); //fecho do ficheiro para comunicação com o servidor
	unlink(FIFO_CLIENTE); 
	
	
	return 0; 
}