#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <time.h>

#define TAM 200




typedef struct {
	int nFaixas;
	int velocCarros;
}GameInfo;

typedef struct {
	int state; // 0 - em jogo, 1 - pause, 2 - reiniciar, 3 - Encerrar Sistema
}GameState;



DWORD WINAPI ThreadMensagens(LPVOID param) {

	return 0;
}