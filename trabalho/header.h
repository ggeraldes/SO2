#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <time.h>

#define TAM 200
#define NUM_CHAR 26
#define TAM_BUFFER 26

#define NFAIXAS 1
#define MAXLINHAS 10
#define COLUNAS 20
#define MAXCARROS 8

/*
typedef struct {
	TCHAR* fileViewMap;
	HANDLE hEvent;
	HANDLE hMutex;
	int terminar;
}ThreadDados;
*/



typedef struct {
	int val;
}Coluna;

typedef struct {
	int id;
	int state; //0 - sem obstaculo, 1 - obstaculo direita, 2 - obstaculo esquerda
	int nCars;
	Coluna col[COLUNAS];
}Faixa;

typedef struct {
	int nFaixas;
	int velocCarros;
	int state; // 0 - em jogo, 1 - pause, 2 - reiniciar, 3 - Encerrar Sistema
	BOOL stop;
	int stopTime; //se estiver no estado stop
	Faixa faixa[MAXLINHAS];
	HANDLE gameMutex;
	HANDLE hMutexG;
	HWND hWnd;
}GameInfo;

typedef struct {
	GameInfo* fileViewMap;
	HANDLE hEvent;
	HANDLE hMutex;
	int terminar;
}ThreadTab;




//------------------------------------------------------------------------------------

//estrutura para o buffer circular
typedef struct {
	int id;
	int comando[3]; //comando, segundo comando, terceiro comando
}CelulaBuffer;


//representa a nossa memoria partilhada
typedef struct {
	int nOperadores;
	int posE; //proxima posicao de escrita
	int posL; //proxima posicao de leitura
	CelulaBuffer buffer[TAM_BUFFER]; //buffer circular em si (array de estruturas)
}BufferCircular;

//estrutura de apoio
typedef struct {
	BufferCircular* memPar; //ponteiro para a memoria partilhada
	HANDLE hSemEscrita; //handle para o semaforo que controla as escritas (controla quantas posicoes estao vazias)
	HANDLE hSemLeitura; //handle para o semaforo que controla as leituras (controla quantas posicoes estao preenchidas)
	HANDLE hMutex;
	int terminar; // 1 para sair, 0 em caso contrário
	int id;
	//GameInfo* game;
}DadosThreads;

//----------------------------------------------------------------------------



int random_60_40() {

	//srand(time(NULL)); // inicializa o gerador de números aleatórios
	int random_number = rand() % 10; // gera um número aleatório entre 0 e 9

	if (random_number < 6) { // 60% de chance de retornar 1
		return 1;
	}
	else { // 40% de chance de retornar 2
		return 2;
	}
}

int random_0_1_2() {
	return rand() % 3;  // Modulo 3 restricts the result to 0, 1, or 2
}