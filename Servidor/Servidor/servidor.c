
#include "../../header.h"

#define NFAIXAS 1
#define MAXLINHAS 10
#define COLUNAS 20
#define MAXCARROS 8



typedef struct {
	int posX, posY; //posicao
	int mov; //movimentos
}Sapo;

//------------------------------------------MEMORIA PARTILHADA------------------------
DWORD WINAPI ThreadLer(LPVOID param) {
	ThreadDados* dados = (ThreadDados*)param;

	DWORD size;
	COORD t;
	t.X = 40;
	t.Y = 0;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	int linhas = 0;

	while (1) {

		if (linhas > 10) {
			//FillConsoleOutputCharacter(hConsole, _T(' '), 80 * 1, t, &size);
			FillConsoleOutputCharacter(hConsole, _T(' '), t.X * t.Y, t, &size);
			t.Y = 0;
			linhas = 0;
		}
		

		//esperar até que evento desbloqueie
		WaitForSingleObject(dados->hEvent, INFINITE);

		t.Y++;
		linhas++;

		//verifica se é preciso terminar a thread ou nao
		if (dados->terminar)
			break;

		
		SetConsoleCursorPosition(hConsole, t);
		
		//faço o lock para o mutex
		WaitForSingleObject(dados->hMutex, INFINITE);
		_tprintf(TEXT("Mensagem recebida: %s\n"), dados->fileViewMap);

		//faço unlock do mutex
		ReleaseMutex(dados->hMutex);

		Sleep(1000);
	}

	return 0;
}

DWORD WINAPI ThreadEscrever(LPVOID param) {
	TCHAR msg[NUM_CHAR];
	ThreadDados* dados = (ThreadDados*)param;


	while (!(dados->terminar)) {
		_fgetts(msg, NUM_CHAR, stdin);
		msg[_tcslen(msg) - 1] = '\0'; //terminamos a string de maneira correta

		if (_tcscmp(msg, TEXT("fim")) == 0)
			dados->terminar = 1;


		//faço lock ao mutex
		WaitForSingleObject(dados->hMutex, INFINITE);

		//limpa memoria antes de fazer a copia
		ZeroMemory(dados->fileViewMap, NUM_CHAR * sizeof(TCHAR));

		//copia memoria de um sitio para outro (aqui copia a mensagem escrita no terminal para o fileViewMap)
		CopyMemory(dados->fileViewMap, msg, _tcslen(msg) * sizeof(TCHAR));

		//liberto mutex
		ReleaseMutex(dados->hMutex);

		//criamos evento
		SetEvent(dados->hEvent);
		Sleep(500);

		ResetEvent(dados->hEvent); //torna o evento novamente não assinalado
	}

	return 0;
}
//-------------------------------------------------------------------------------------


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
		do {
			_tprintf(TEXT("Número de faixas: "));
			_tscanf_s(TEXT("%d"), &gameinfo.nFaixas);
		} while (gameinfo.nFaixas > 10);

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
			do {
				_tprintf(TEXT("Número de faixas: "));
				_tscanf_s(TEXT("%d"), &gameinfo.nFaixas);
			} while (gameinfo.nFaixas > 10);

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

	//------------------------------------------------------------------MEMORIA PARTILHADA-----------------------------------------------------------------------
	
	/*HANDLE hFileMap;
	ThreadDados dados;
	HANDLE hThreads;
	//HANDLE hSem;


	 //mapeia ficheiro num bloco de memoria
	hFileMap = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		NUM_CHAR * sizeof(TCHAR), // alterar o tamanho do filemapping
		TEXT("TP_MEM_OPERADOR")); //nome do file mapping, tem de ser único

	if (hFileMap == NULL) {
		_tprintf(TEXT("Erro no CreateFileMapping\n"));
		//CloseHandle(hFile); //recebe um handle e fecha esse handle , no entanto o handle é limpo sempre que o processo termina
		return 1;
	}

	//mapeia bloco de memoria para espaço de endereçamento
	dados.fileViewMap = (TCHAR*)MapViewOfFile(
		hFileMap,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		0);

	if (dados.fileViewMap == NULL) {
		_tprintf(TEXT("Erro no MapViewOfFile\n"));
		return 1;
	}


	dados.hEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		TEXT("SO2_EVENTO"));

	if (dados.hEvent == NULL) {
		_tprintf(TEXT("Erro no CreateEvent\n"));
		UnmapViewOfFile(dados.fileViewMap);
		return 1;
	}

	dados.hMutex = CreateMutex(
		NULL,
		FALSE,
		TEXT("SO2_MUTEX"));

	if (dados.hMutex == NULL) {
		_tprintf(TEXT("Erro no CreateMutex\n"));
		UnmapViewOfFile(dados.fileViewMap);
		return 1;
	}

	dados.terminar = 0;

	hThreads = CreateThread(NULL, 0, ThreadLer, &dados, 0, NULL);

	*/

	//---------------------------------------------------------------------------TABULEIRO-----------------------------------------------------------------------
	COORD t;
	t.X = 0;
	t.Y = 0;
	HANDLE hStdout;

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// Set the console text attributes to the default values (white on black)
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	// Get the size of the console buffer
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	COORD bufferSize = { csbi.dwSize.X, csbi.dwSize.Y };

	// Fill the entire console buffer with spaces
	DWORD numCellsWritten;
	FillConsoleOutputCharacter(hConsole, _T(' '), bufferSize.X * bufferSize.Y, t, &numCellsWritten);

	// Set the console cursor position to the top left corner
	SetConsoleCursorPosition(hConsole, t);

	int tab[10][COLUNAS];
	
	
	
	
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	FillConsoleOutputCharacter(hStdout, _T(' '), 1, t, &size);



	int random_number = rand() % 20;
	tab[gameinfo.nFaixas-1][random_number] = 1;

	do {
		random_number = rand() % 20;
	} while (tab[gameinfo.nFaixas - 1][random_number] == 1);

	tab[gameinfo.nFaixas-1][random_number] = 1;


	for (int i = 0; i < gameinfo.nFaixas; i++) {
		_tprintf(TEXT("|"));
		for (int y = 0; y < COLUNAS; y++)
			if(tab[i][y]!=1)
				_tprintf(TEXT(" "));
			else
				_tprintf(TEXT("s"));
		_tprintf(TEXT("|\n"));
	}



	//-----------------------------------------------------------------------------------------------------------------------------------------------------------
	
	COORD pos;
	pos.X = 0;
	pos.Y = 12;
	

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

		

	} while (_tcscmp(COM, TEXT("close"))!=0 && gamestate.state != 3);

	gamestate.state = 3;

	//dados.terminar = 1;

	//WaitForSingleObject(hThreads, INFINITE);
	

	return 0;
}