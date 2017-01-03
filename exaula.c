
//gcc ex.c -o ex -lncurses 
#include <ncurses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <string.h>
#include <stdio.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h> 
#include <pthread.h> //bibliotecas para os threads

typedef struct{
char y, x; 
}POSICAO; 

typedef struct{
char fim, humano,num; 
int tempo;

}JOGADOR;
int maxY, maxX; 
POSICAO ele[4]={ {3,10},{15,15},{12,40},{19,45}};

pthread_mutex_t trinco;
void *move_jogador(void *dados){
JOGADOR *j;
POSICAO d; 
int ch; 
j=(JOGADOR *) dados; 
do{ 
	if (!j->humano){
		ch=rand()%4; 
		d.y=0;d.x=0; 
		switch(ch){
	case 0:d.y--;break; 
	case 1:d.y++;break;
	case 2:d.x--;break;	
	case 3:d.x++;break;
	
	}
	
	if (j->num>=0&&j->num<4){
	pthread_mutex_lock(&trinco); 
	mvaddch(ele[j->num].y,ele[j->num].x,' ');
	ele[j->num].y+=d.y; 
	ele[j->num].x+=d.x;
	mvaddch(ele[j->num].y,ele[j->num].x,'0'+j->num);
		refresh(); 
		}
	pthread_mutex_unlock(&trinco);
	}
	usleep(j->tempo);	
}while(!j->fim); 
pthread_exit(0); 
}
 
int main(void){
POSICAO d; 
JOGADOR j[4]={{0,0,0,100000},{0,0,1,300000},{0,0,2,500000},{0,0,3,700000}};
pthread_t tarefa[4];
int ch,i,num; 
initscr(); 
noecho(); //não faz o echo das teclas no ecrã 
cbreak(); //não espera pelo enter no final de linha 
keypad(stdscr,TRUE); 
curs_set(0); //evita que o cursor apareça no ecrã 
getmaxyx(stdscr,maxY,maxX); 
pthread_mutex_init(&trinco,NULL); 

//a coordenada y é sempre primeiro que o x

//desenhar todos os jogadores 
//clear(); 

/*for(i=0;i<4;i++)
	mvaddch(ele[i].y,ele[i].x,'0'+i);

refresh(); */
for(i=0;i<4;i++)
	pthread_create(&tarefa[i],NULL,&move_jogador,(void *)&j[i]); 

num=-1; //jogador que controlo 

do{
	ch=getch(); 
	d.y=0;
	d.x=0; 
	switch(ch){
	case '0':num=0; j[0].humano=1; break; 
	case '1':num=1;j[1].humano=1;break;
	case '2':num=2;j[2].humano=1;break;	
	case '3':num=3;j[3].humano=1;break;
	case 'd':num=-1;
		for (i=0;i<4;i++){
			j[i].humano=0; 
		}
	break;	
	case KEY_UP: 
	d.y--; 
	break; 
	case KEY_DOWN:
	d.y++;
	break;
	case KEY_LEFT:
	d.x--;
	break;	
	case KEY_RIGHT:
	d.x++;
	break; 
	}

	if(num>=0&&num<4){
	pthread_mutex_lock(&trinco);
		mvaddch(ele[num].y,ele[num].x,' ');
		//verifica se existe jogador na posição ou se sai de fora do campo 
		ele[num].y+=d.y; 
		ele[num].x+=d.x;
		mvaddch(ele[num].y,ele[num].x,'0'+num);
		refresh(); 
	pthread_mutex_unlock(&trinco);
	}
	
		
}while(ch!='q');
for(i=0;i<4;i++){
	j[i].fim=1; 
	pthread_join(tarefa[i],NULL); 
}
pthread_mutex_destroy(&trinco);
endwin();//terminar o ncurses 
exit(0); 
}