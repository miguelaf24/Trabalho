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
	char uname_temp[MAX_LOGIN], upass_temp[MAX_LOGIN];
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
	
	printf("Username: ");
	fflush(stdin);
	fgets(user_struct.user_data_username, sizeof(user_struct.user_data_username),stdin);
	user_struct.user_data_username[strlen(user_struct.user_data_username) -1]='\0';
	
	printf("Password: ");
	fflush(stdin);
	fgets(user_struct.user_data_upass, sizeof(user_struct.user_data_upass),stdin);
	user_struct.user_data_upass[strlen(user_struct.user_data_upass) -1]='\0';
	
	user_struct.pid = getpid();
	strcpy(user_struct.user_data_fifo, FIFO_CLIENTE);
	user_struct.user_data_order = 0;
	strcpy(user_struct.user_data_cmd, "login");
	
	fd_envi = open(FIFOSERV,O_WRONLY);
	write(fd_envi, &user_struct, sizeof(user_struct));
	close(fd_envi);
	
	strcpy(uname_temp, user_struct.user_data_uname);
	strcpy(upass_temp, user_struct.user_data_upass);
	
	fd_cli = open(FIFO_CLIENTE, O_RDONLY);
	read(fd_cli, &user_struct, sizeof(user_struct));
	close(fifo_cli);
	
	if(strcmp(user_struct.user_data_uname, "OK") == 0 && strcmp(user_struct.user_data_upass, "OK") == 0)
	{
		printf(" - Login aceite, ligado ao servidor\n");
		sleep(1);
		
		strcpy(user_struct.user_data_uname, uname_temp);
		strcpy(user_struct.user_data_upass, upass_temp);
		
		//start_game(&user_struct);
	}
	else if(strcmp(user_struct.user_data_uname, "FAIL") == 0 && strcmp(user_struct.user_data_upass, "FAIL") == 0)
	{
		printf(" - Login invalido, a sair...\n");
	}
	else if(strcmp(user_struct.user_data_uname, "EXISTS") == 0 && strcmp(user_struct.user_data_upass, "EXISTS") == 0)
	{
		printf(" - Login ja registado, a sair...\n");
	}
	else if(strcmp(user_struct.user_data_uname, "MAX") == 0 && strcmp(user_struct.user_data_upass, "MAX") == 0)
	{
		printf(" - Servidor cheio, a sair...\n");
	}
	
	//fechar o programa corretamente
	unlink(FIFO_CLIENTE);
	/*
	
	
	
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
	*/
	
	return 0; 
}