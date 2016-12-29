#include "header.h"

//aviso de erro
#define ERR_DEFAULT "ERRO "

//função de fecho do programa
void terminate(char *nome_fifo)
{
	//remover o fifo do cliente
	unlink(nome_fifo);
	
	exit(0);
}

void limpa_consola()
{
	system("clear");//limpar a consola
}

//função que verifica se o servidor corre ou nao
int fifo_server_exists()
{
	if(access(FSERV, F_OK) == 0)
	{
		return 1;
	}
	else
	{
		//nao existe
		return 0;
	}
}

//função que trata sinais
void signal_handle(int sign)
{
	char nome_fifo_cli[MAX_LOGIN];
	
	switch (sign)
	{
		case SIGINT:
			printf(" Programa interrompido, a terminar...\n");
			sprintf(nome_fifo_cli, FCLI, getpid()); //como nao é possivel
			endwin(); //caso aconteça enquanto o ncurses funciona
			//passar como argumento ao signal handle
			terminate(nome_fifo_cli);
			break;
	}
}

//função que interpreta o conteúdo do fifo do cliente
void trata_fifo_client(int fifo_cli, user_data *user_struct, int * isGameRunning, int *isPlaying, int *pacman_lifes)
{
	
	memset(user_struct, 0, sizeof(*user_struct));
	//ler conteudo do fifo
	read(fifo_cli, user_struct, sizeof(*user_struct));
	if(strcmp(user_struct->user_data_cmd, "kicked") == 0) //comando kick vindo do server
	{
		endwin();
		printf(" - Foi expulso do servidor, a terminar...\n");
		terminate(user_struct->user_data_fifo);
	}
	else if(strcmp(user_struct->user_data_cmd, "server shutdown") == 0) //comando shutdown vindo do server
	{
		endwin();
		printf(" - O servidor foi encerrado, a terminar...\n");
		terminate(user_struct->user_data_fifo);
	}
	else if(strcmp(user_struct->user_data_cmd, "gameUp") == 0) //server diz que há jogo aberto
	{
		*isGameRunning = 1;
		if(user_struct->user_data_order != 0)
		{
			*isPlaying = 1; //colocar o cliente a jogar
		}
	}
	else if(strcmp(user_struct->user_data_cmd, "gameDown") == 0) //server diz que NAO há jogo aberto
	{
		*isGameRunning = 0;
		*isPlaying = 0;
		user_struct->user_data_ingame = 0;
	}
	else if(atoi(&user_struct->user_data_cmd[0]) == user_struct->user_data_order)
	{
		*isPlaying = 0;
		user_struct->user_data_ingame = 0;
	}
	else if(strcmp(user_struct->user_data_cmd, "pacmanWon") == 0) //server diz que o pacman venceu
	{
		*isGameRunning = 0;
		*isPlaying = 0;
		user_struct->user_data_ingame = 0;
	}
	else if(strcmp(user_struct->user_data_cmd, "ghostsWon") == 0) //server diz que os fantasmas venceram
	{
		*isGameRunning = 0;
		*isPlaying = 0;
		user_struct->user_data_ingame = 0;
	}
	
	clear(); //limpar o ecrã apenas nesta situação para evitar flicker
}

//função que delimita o menu visualmente
void print_menu_limit(WINDOW * w, int dim_x, int dim_y)
{
	int i;
	
	for(i = 0; i < dim_x; i++)
		mvwaddch(w, 0, i, 97 | A_ALTCHARSET);
	for(i = 0; i < dim_y; i++)
	{
		mvwaddch(w, i, 0, 97 | A_ALTCHARSET);
		mvwaddch(w, i, 1, 97 | A_ALTCHARSET);
	}
	for(i = 0; i < dim_x; i++)
		mvwaddch(w, dim_y-1, i, 97 | A_ALTCHARSET);
	for(i = 0; i < dim_y; i++)
	{
		mvwaddch(w, i, dim_x-2, 97 | A_ALTCHARSET);
		mvwaddch(w, i, dim_x-1, 97 | A_ALTCHARSET);
	}
	wattron(w, A_REVERSE); 
	mvwprintw(w, 0, dim_x/2 -5, "Soccer ISEC");
	wattroff(w, A_REVERSE); 
	
}

//função que mostra o menu quando fora de jogo
void print_menu(WINDOW * w, int c)
{
	
	
	mvwprintw(w, 2, 5, "VERDE");
	mvwprintw(w, 3, 7, "0");
	mvwprintw(w, 4, 7, "1");
	mvwprintw(w, 5, 7, "2");
	mvwprintw(w, 6, 7, "3");
	mvwprintw(w, 7, 7, "4");
	mvwprintw(w, 8, 7, "6");
	mvwprintw(w, 9, 7, "7");
	mvwprintw(w, 10, 7, "8");
	mvwprintw(w, 11, 7, "9");
	
	mvwprintw(w, 2, 13, "VERMELHO");
	mvwprintw(w, 3, 17, "0");
	mvwprintw(w, 4, 17, "1");
	mvwprintw(w, 5, 17, "2");
	mvwprintw(w, 6, 17, "3");
	mvwprintw(w, 7, 17, "4");
	mvwprintw(w, 8, 17, "6");
	mvwprintw(w, 9, 17, "7");
	mvwprintw(w, 10, 17, "8");
	mvwprintw(w, 11, 17, "9");
	
	mvwprintw(w, 13, 10, "SAIR");
	
	switch(c)
	{
		case 1:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 3, 7, "0");
			wattroff(w, A_REVERSE); 
			break;
		case 2:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 4, 7, "1");
			wattroff(w, A_REVERSE); 
			break;
		case 3:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 5, 7, "2");
			wattroff(w, A_REVERSE); 
			break;
		case 4:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 6, 7, "3");
			wattroff(w, A_REVERSE); 
			break;
		case 5:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 7, 7, "4");
			wattroff(w, A_REVERSE); 
			break;
		case 6:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 8, 7, "6");
			wattroff(w, A_REVERSE); 
			break;
		case 7:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 9, 7, "7");
			wattroff(w, A_REVERSE); 
			break;
		case 8:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 10, 7, "8");
			wattroff(w, A_REVERSE); 
			break;
		case 9:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 11, 7, "9");
			wattroff(w, A_REVERSE); 
			break;
		case 10:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 3, 17, "0");
			wattroff(w, A_REVERSE); 
			break;
		case 11:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 4, 17, "1");
			wattroff(w, A_REVERSE); 
			break;
		case 12:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 5, 17, "2");
			wattroff(w, A_REVERSE); 
			break;
		case 13:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 6, 17, "3");
			wattroff(w, A_REVERSE); 
			break;
		case 14:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 7, 17, "4");
			wattroff(w, A_REVERSE); 
			break;
		case 15:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 8, 17, "6");
			wattroff(w, A_REVERSE); 
			break;
		case 16:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 9, 17, "7");
			wattroff(w, A_REVERSE); 
			break;
		case 17:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 10, 17, "8");
			wattroff(w, A_REVERSE); 
			break;
		case 18:
			wattron(w, A_REVERSE); 
			mvwprintw(w, 11, 17, "9");
			wattroff(w, A_REVERSE); 
			break;
	}
	wrefresh(w);
}

//enviar mensagem para o servidor
void enviar_mensagem_server(user_data *user_struct, char msgToSend[MAX_CMD])
{
	int fifo_serv;
	
	memset(user_struct->user_data_cmd, 0, MAX_CMD);
	strcpy(user_struct->user_data_cmd, msgToSend);
	
	fifo_serv = open(FSERV, O_WRONLY);
	write(fifo_serv, user_struct, sizeof(user_data));
	close(fifo_serv);
}

//mostrar o mapa do jogo
void print_game_map(user_data *user_struct, WINDOW * wgame)
{
	int i,j;
	
	start_color();
	use_default_colors();
	
	char old = getbkgd(wgame);
	
	init_pair(1, COLOR_YELLOW, -1);
	init_pair(2, COLOR_CYAN, -1);
	init_pair(3, COLOR_RED, -1);
	init_pair(4, COLOR_GREEN, -1);
	init_pair(5, COLOR_BLUE, -1);
	init_pair(6, COLOR_WHITE, -1);
	
	clear();
	for(i = 0; i < MAP_Y; i++)
	{
		for(j = 0; j < MAP_X; j++)
		{
			if(user_struct->user_data_map[j][i] == 'W')
			{
				mvwaddch(wgame, i, j, 97 | A_ALTCHARSET);
			}
			else if(user_struct->user_data_map[j][i] == 'S')
			{
				attron(A_BOLD);
				//mvwaddch(wgame, i, j, 126 | A_ALTCHARSET);
				mvwaddch(wgame, i, j, '*');
				attroff(A_BOLD);
			}
			else if(user_struct->user_data_map[j][i] == 'O')
			{
				attron(A_DIM);
				mvwaddch(wgame, i, j, 126 | A_ALTCHARSET);
				attroff(A_DIM);
			}
			else if(user_struct->user_data_map[j][i] == 'N')
			{
				mvwaddch(wgame, i, j, ' ');
			}
			else if(user_struct->user_data_map[j][i] == 'G')
			{
				mvwaddch(wgame, i, j, 'G');
			}
			else if(user_struct->user_data_map[j][i] == 'T')
			{
				mvwaddch(wgame, i, j, 'T');
			}
			else if(user_struct->user_data_map[j][i] == '1')
			{
				attron(A_BOLD);
				wbkgdset(wgame, COLOR_PAIR(1));
				mvwaddch(wgame, i, j, 'C');
				wbkgdset(wgame, old);
				attroff(A_BOLD);
			}
			else if(user_struct->user_data_map[j][i] == '2')
			{
				attron(A_BOLD);
				wbkgdset(wgame, COLOR_PAIR(2));
				mvwaddch(wgame, i, j, 'A');
				wbkgdset(wgame, old);
				attroff(A_BOLD);
			}
			else if(user_struct->user_data_map[j][i] == '3')
			{
				attron(A_BOLD);
				wbkgdset(wgame, COLOR_PAIR(3));
				mvwaddch(wgame, i, j, 'A');
				wbkgdset(wgame, old);
				attroff(A_BOLD);
			}
			else if(user_struct->user_data_map[j][i] == '4')
			{
				attron(A_BOLD);
				wbkgdset(wgame, COLOR_PAIR(4));
				mvwaddch(wgame, i, j, 'A');
				wbkgdset(wgame, old);
				attroff(A_BOLD);
			}
			else if(user_struct->user_data_map[j][i] == '5')
			{
				attron(A_BOLD);
				wbkgdset(wgame, COLOR_PAIR(5));
				mvwaddch(wgame, i, j, 'A');
				wbkgdset(wgame, old);
				attroff(A_BOLD);
			}
		}
	}
	
	refresh();
	wrefresh(wgame);
	
}

//função principal do jogo
//aqui corre o ciclo base do jogo
//só é chamada se o login for aceite pelo servidor
void start_game(user_data *user_struct)
{
	int p_scr_x, p_scr_y; //dimensoes do ecra parent (stdscr)
	int score_size = 2; //tamanho Y da janela score
	
	int fifo_serv, fifo_cli; //file descriptor retornado de open()
	
	int isGameRunning = 0; //nao há jogo nenhum a decorrer inicialmente
	int isPlaying = 0; //inicialmente o cliente nao está a jogar
	
	//file descriptors
	fd_set fd_read;
	int fd_return;
	
	int i, j;
	
	//vidas do pacman
	int pacman_lifes = PACMAN_LIVES_DEFAULT;
	
	//timeval
	struct timeval tv;	//timeout for select()
	
	int choice = 1, c;
	
	//janela menu
	WINDOW *wmenu, *wgame;
	
	//iniciar ncurses
	initscr();
	noecho(); //ignorar prints para o terminal
	curs_set(FALSE); //nao mostrar cursor
	cbreak(); //disable line buffering
	
	//abrir o fifo deste cliente em non-block mode
	fifo_cli = open(user_struct->user_data_fifo, O_RDWR| O_NONBLOCK);
	if(fifo_cli == -1)
	{
		perror(ERR_DEFAULT);
		endwin();
		terminate(user_struct->user_data_fifo);
	}
	
	//mostrar menu inicial (está atento a key presses e ao proprio fifo)
	resizeterm(TERM_MAX_Y, TERM_MAX_X); //redimensionar o terminal
	
	wmenu = newwin(18, 25, 6, 10);
	keypad(wmenu, TRUE);
	wgame = newwin(MAP_Y, MAP_X, 1, 2);
	keypad(wgame, TRUE);
	
	clear();
	mvprintw(1, 1, "User: %s", user_struct->user_data_uname);
	if(isGameRunning)
		mvprintw(2, 1, "Jogo a decorrer");
	else
		mvprintw(2, 1, "Nao existe jogo a decorrer");
	refresh();
	print_menu_limit(wmenu, 40, 20);

	while(1)
	{
		//////////////select//////////////////
		tv.tv_sec = 0;		
		tv.tv_usec = 1000;	//1ms
		FD_ZERO(&fd_read);
	
		// descriptors a ter em conta
		FD_SET(fifo_cli, &fd_read); //fifo server
		
		// espera "infinitamente" por alterações no fd de stdin ou no fifo
		fd_return = select(fifo_cli+1, &fd_read, NULL, NULL, &tv);
		if(fd_return == -1) // ERRO
			perror(ERR_DEFAULT);
		else
		{
			if(FD_ISSET(fifo_cli, &fd_read)) //fifo client tem dados
			{
				//ler info que foi escrita no fifo do cliente
				trata_fifo_client(fifo_cli, user_struct, &isGameRunning, &isPlaying, &pacman_lifes);
				//apenas fazer refresh caso seja enviado algo ao cliente
				if(!isPlaying)
				{
					werase(wgame);
					wrefresh(wgame);
					isPlaying = 0;
					clear();
					mvprintw(1, 1, "User: %s", user_struct->user_data_uname);
					if(isGameRunning)
						mvprintw(2, 1, "Jogo a decorrer");
					else
						mvprintw(2, 1, "Nao existe jogo a decorrer");
					refresh();
					print_menu_limit(wmenu, 20, 8);
					print_menu(wmenu, choice);
					wrefresh(wmenu);
				}
				else
				{//se estiver a jogar e recebeu alguma mensagem do servidor
					print_game_map(user_struct, wgame);
				}
			}
		}

		
		//getch trabalha em no-delay mode
		nodelay(wmenu, TRUE);
		c = wgetch(wmenu);
		switch(c)
		{
			case KEY_UP:
				if(!isPlaying)
				{
					if(choice == 10)
						choice = 19;
					else if(choice == 1)
						choice = 19;
					else
						choice --;
				}
				else
				{
					//enviar KEYUP para server
					enviar_mensagem_server(user_struct, "KEYUP");
				}
				break;
			
			case KEY_DOWN:
				if(!isPlaying)
				{
					if(choice == 9)
						choise =19;
					else if(choice == 19)
						choice = 1;
					else
						choice ++;
				}
				else
				{
					//enviar KEYDOWN para server
					enviar_mensagem_server(user_struct, "KEYDOWN");
				}
				break;
				
			case KEY_RIGHT:
				if(!isPlaying){
					if(choice >= 1 && choise<=9)
						choise +=9;
					else if(choice >= 10 && choise<=18)
						choise -=9;
				}
				else
				{
					//enviar KEYRIGHT para server
					enviar_mensagem_server(user_struct, "KEYRIGHT");
				}
				break;
				
			case KEY_LEFT:
				if(!isPlaying){
					if(choice >= 1 && choise<=9)
						choise +=9;
					else if(choice >= 10 && choise<=18)
						choise -=9;
				}
				else
				{
					//enviar KEYLEFT para server
					enviar_mensagem_server(user_struct, "KEYLEFT");
				}
				break;
				
			case 113: //Q
				if(isPlaying)
				{
					//enviar QUIT para server
					enviar_mensagem_server(user_struct, "QUIT");
					werase(wgame);
					wrefresh(wgame);
					isPlaying = 0;
					clear();
					mvprintw(1, 1, "User: %s", user_struct->user_data_uname);
					if(isGameRunning)
						mvprintw(2, 1, "Jogo a decorrer");
					else
						mvprintw(2, 1, "Nao existe jogo a decorrer");
					refresh();
					print_menu_limit(wmenu, 20, 8);
					print_menu(wmenu, choice);
					wrefresh(wmenu);
				}
				break;
				
			case 10: //ENTER
				if(!isPlaying)
				{
					if(choice == 1) //JOGAR
					{
						//começar o jogo
						enviar_mensagem_server(user_struct, "PLAY");
					}
					else if(choice == 2) //SAIR
					{
						endwin();
						terminate(user_struct->user_data_fifo);
					}
				}
				break;
		}
		
		
		//se o cliente estiver a jogar, começar logo por mostrar o mapa
		//e toda a informação visual relativa ao jogo
		if(!isPlaying)
		{
			//mostrar menu com a devida opção escolhida
			print_menu(wmenu, choice);
		}
		
		//testar constantemente se o servidor ainda corre
		if(fifo_server_exists() == 0)
		{
			//nao existe
			endwin();
			printf(" - Perdeu contacto com o servidor, a terminar...\n");
			terminate(user_struct->user_data_fifo);
		}
		
		// descriptors a ter em conta (eliminar no fim de cada iteração)
		FD_CLR(fifo_cli, &fd_read); //fifo server
		
	}	
	
	//antes de sair, terminar o ncurses
	endwin();
}

void main()
{
	limpa_consola();
	
	//alterar umask do cliente
	umask(0000);
	
	//registar handles dos sinais
	signal(SIGINT, signal_handle);
	
	char nome_fifo_cli[MAX_LOGIN]; //nome do fifo do cliente+pid
	int fifo_serv, fifo_cli; //file descriptor retornado de open()
	user_data user_struct;
	char uname_temp[MAX_LOGIN], upass_temp[MAX_LOGIN];
	
	printf(" A abrir cliente(pid:%d)\n", getpid());
	
	if(fifo_server_exists() != 0)
	{
		printf(" -Servidor detetado a correr\n");
	}
	else
	{
		//nao existe
		printf(" -Servidor nao detetado, a terminar...\n");
		exit(0);
	}
	
	//criar fifo do cliente//
	sprintf(nome_fifo_cli, FCLI, getpid());
	if(mkfifo(nome_fifo_cli, 0777) != 0)
	{
		perror(ERR_DEFAULT);
		return; //sair do programa pois foi impossível criar fifo OU este já existe
	}
	printf(" -FIFO criado\n");
	
	sleep(2);
	limpa_consola();
	
	printf("Login\n Insira username: ");
	fflush(stdin);
	fgets(user_struct.user_data_uname, sizeof(user_struct.user_data_uname), stdin);
	user_struct.user_data_uname[strlen(user_struct.user_data_uname) - 1] = '\0';
	
	printf(" Insira password: ");
	fflush(stdin);
	fgets(user_struct.user_data_upass, sizeof(user_struct.user_data_upass), stdin);
	user_struct.user_data_upass[strlen(user_struct.user_data_upass) - 1] = '\0';
	
	user_struct.user_data_pid = getpid();
	strcpy(user_struct.user_data_fifo, nome_fifo_cli);
	user_struct.user_data_order = 0;
	strcpy(user_struct.user_data_cmd, "login"); //dizer ao servidor que pretende ligar-se
	
	//escrever login para o fifo do server e enviar
	fifo_serv = open(FSERV, O_WRONLY); //abrir para escrita apenas
	write(fifo_serv, &user_struct, sizeof(user_struct));
	close(fifo_serv);
	printf("Aguarde...\n");
	strcpy(uname_temp, user_struct.user_data_uname);
	strcpy(upass_temp, user_struct.user_data_upass);
	
	//o servidor está feito para logo após um login
	//escrever de volta, então o cliente terá de abrir o seu proprio fifo
	//para leitura
	fifo_cli = open(nome_fifo_cli, O_RDONLY); //abrir para leitura apenas
	read(fifo_cli, &user_struct, sizeof(user_struct));
	close(fifo_cli);
	
	//verificar qual o retorno escrito pelo servidor
	if(strcmp(user_struct.user_data_uname, "OK") == 0 && strcmp(user_struct.user_data_upass, "OK") == 0)
	{//LOGIN ACEITE
		//o login foi aceite pelo servidor, este cliente pode entrar no seu ciclo do jogo
		printf(" - Login aceite, ligado ao servidor\n");
		sleep(1);
		
		//recolocar pass e username nos sitios respectivos na struct do cliente
		strcpy(user_struct.user_data_uname, uname_temp);
		strcpy(user_struct.user_data_upass, upass_temp);
		
		//função que controla o jogo todo
		start_game(&user_struct);
	}
	else if(strcmp(user_struct.user_data_uname, "FAIL") == 0 && strcmp(user_struct.user_data_upass, "FAIL") == 0)
	{//LOGIN INVÁLIDO
		//o login é inválido, o cliente será abortado
		printf(" - Login invalido, a sair...\n");
	}
	else if(strcmp(user_struct.user_data_uname, "EXISTS") == 0 && strcmp(user_struct.user_data_upass, "EXISTS") == 0)
	{//LOGIN JÁ EXISTENTE
		//o login já existe no servidor, o cliente será abortado
		printf(" - Login ja registado, a sair...\n");
	}
	else if(strcmp(user_struct.user_data_uname, "MAX") == 0 && strcmp(user_struct.user_data_upass, "MAX") == 0)
	{//MAXIMO DE USERS ATINGIDO
		//o login é valido mas o numero de utilizadores no servidor chegou ao limite
		printf(" - Servidor cheio, a sair...\n");
	}
	
	//fechar o programa corretamente
	terminate(nome_fifo_cli);
	
}