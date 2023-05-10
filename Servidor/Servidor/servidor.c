
#include "../../header.h"

#define NFAIXAS 1
#define MAXLINHAS 10
#define COLUNAS 20
#define MAXCARROS 8



typedef struct {
	int posX, posY; //posicao
	int mov; //movimentos
}Sapo;


int _tmain(int argc, LPTSTR argv[]) {
	
	
	srand(time(NULL));

	TCHAR COM[50]; //COMANDOS
	GameState gamestate;
	GameInfo gameinfo;

	gameinfo.nFaixas = 0;
	gameinfo.velocCarros = 0;

	gamestate.state = 0;


	HKEY key; //Handle para a chave depois aberta/criada
	TCHAR key_name[TAM] = TEXT("Software\\TP\\KEYFrogger");
	DWORD result; // o que aconteceu com a chave
	DWORD size;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	do{
		_tprintf(TEXT("Fazer especificação inicial? "));
		_fgetts(COM, 50, stdin);
		COM[_tcslen(COM) - 1] = '\0';
		
			
	} while (_tcscmp(COM, TEXT("yes")) != 0 && _tcscmp(COM, TEXT("no")) != 0);

	if (_tcscmp(COM, TEXT("yes")) == 0) {

		_tprintf(TEXT("Número de faixas: "));
		_tscanf_s(TEXT("%d"), &gameinfo.nFaixas);

		_tprintf(TEXT("Velocidade Inicial carros: "));
		_tscanf_s(TEXT("%d"), &gameinfo.velocCarros);
		//fflush(stdin);
	}
		
	//-----------------------------------------------------------------------------------REGISTRY---------------------------------------------------------------------
	//Criar ou abrir uma chave do Registry
	if (RegCreateKeyEx(HKEY_CURRENT_USER,key_name,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&key,&result) != ERROR_SUCCESS) {
		_tprintf(TEXT("Chave nao foi criada nem aberta ! ERRO!"));
		return -1;
	}

	if (result == REG_CREATED_NEW_KEY) {

		_tprintf(TEXT("A chave foi criada: %s"), key_name);
		
		if (gameinfo.velocCarros == 0 && gameinfo.nFaixas == 0) {
			_tprintf(TEXT("Número de faixas: "));
			_tcscanf_s(TEXT("%d"), &gameinfo.nFaixas);

			_tprintf(TEXT("Velocidade Inicial carros: "));
			_tcscanf_s(TEXT("%d"), &gameinfo.velocCarros);
		}

		if(RegSetValueEx(key, TEXT("NFaixas"), 0, REG_DWORD, (LPBYTE)&gameinfo.nFaixas, sizeof(gameinfo.nFaixas)) == ERROR_SUCCESS)
			_tprintf(TEXT("\nNamed value 'NFaixas' was created"));

		if(RegSetValueEx(key, TEXT("VCarrosI"), 0, REG_DWORD, (LPBYTE)&gameinfo.velocCarros, sizeof(gameinfo.velocCarros)) == ERROR_SUCCESS)
			_tprintf(TEXT("\nNamed value 'VCarrosI' was created"));

		
	}
		
	else {
		_tprintf(TEXT("A chave foi aberta:%s\n"), key_name);

		if (gameinfo.velocCarros != 0 && gameinfo.nFaixas != 0) {

			if (RegSetValueEx(key, TEXT("NFaixas"), 0, REG_DWORD, (LPBYTE)&gameinfo.nFaixas, sizeof(gameinfo.nFaixas)) == ERROR_SUCCESS)
				_tprintf(TEXT("\nNamed value 'NFaixas' was edited"));

			if (RegSetValueEx(key, TEXT("VCarrosI"), 0, REG_DWORD, (LPBYTE)&gameinfo.velocCarros, sizeof(gameinfo.velocCarros)) == ERROR_SUCCESS)
				_tprintf(TEXT("\nNamed value 'VCarrosI' was edited"));

		}
		else {

			if (RegQueryValueEx(key, TEXT("NFaixas"), NULL, NULL, (LPBYTE)&gameinfo.nFaixas, &size) == ERROR_SUCCESS) {
				_tprintf(TEXT("\nNFaixas: %d"), gameinfo.nFaixas);
			}
			if (RegQueryValueEx(key, TEXT("VCarrosI"), NULL, NULL, (LPBYTE)&gameinfo.velocCarros, &size) == ERROR_SUCCESS) {
				_tprintf(TEXT("\nVCarrosI: %d"), gameinfo.velocCarros);
			}
			
			
		}
	}
		
	RegCloseKey(key);
	Sleep(2000); //EM PROGRESSO <-----------------------------
	//---------------------------------------------------------------------------TABULEIRO-----------------------------------------------------------------------
	
	int tab[10][COLUNAS];
	
	COORD t;
	t.X = 30;
	t.Y = 0;
	HANDLE hStdout;
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	FillConsoleOutputCharacter(hStdout, _T(' '), 80 * 1, t, &size);

	int random_number = rand() % 20;
	tab[gameinfo.nFaixas-1][random_number] = 1;

	do {
		random_number = rand() % 20;
	} while (tab[0][random_number] == 1);

	tab[gameinfo.nFaixas-1][random_number] = 1;
	for (int i = 0; i < gameinfo.nFaixas; i++) {
		_tprintf(TEXT("|"));
		for (int y = 0; y < COLUNAS; y++)
			if(tab[i][y]!=1)
				_tprintf(TEXT("-"));
			else
				_tprintf(TEXT("s"));
		_tprintf(TEXT("|\n"));
	}



	//-----------------------------------------------------------------------------------------------------------------------------------------------------------
	
	COORD pos;
	pos.X = 0;
	pos.Y = 0;
	
	FillConsoleOutputCharacter(hStdout, _T(' '), 80 * 7, pos, &size);
	do {
		FillConsoleOutputCharacter(hStdout, _T(' '), 80 * 1, pos, &size);
		SetConsoleCursorPosition(hStdout, pos);

		_tprintf(TEXT("COMANDO: "));
		_fgetts(COM, 50, stdin);
		
		COM[_tcslen(COM) - 1] = '\0';

		if (_tcscmp(COM, TEXT("pause"))==0) {

			if (gamestate.state == 1)
				gamestate.state = 0;
			else
				gamestate.state = 1;

		}
		else if (_tcscmp(COM, TEXT("restart"))==0)
			gamestate.state = 2;
			

		_tprintf(TEXT("Estado: %d\n"), gamestate.state);

		

	} while (_tcscmp(COM, TEXT("close")) || gamestate.state != 3);

	gamestate.state = 3;
	

	return 0;
}