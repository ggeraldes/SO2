
#include "../../header.h"




typedef struct {
	int posX, posY; //posicao
	int mov; //movimentos
}Sapo;

//------------------------------------------ BUFFER CIRCULAR -------------------------------------------------------------
DWORD WINAPI ThreadProdutor(LPVOID param) {
	DadosThreads* dados = (DadosThreads*)param;
	CelulaBuffer cel;
	//int contador = 0;
	//int soma = 0;

	while (!dados->terminar) {
		//aqui entramos na logica da aula teorica

		//esperamos por uma posicao para lermos
		WaitForSingleObject(dados->hSemLeitura, INFINITE);

		//esperamos que o mutex esteja livre
		WaitForSingleObject(dados->hMutex, INFINITE);


		//vamos copiar da proxima posicao de leitura do buffer circular para a nossa variavel cel
		CopyMemory(&cel, &dados->memPar->buffer[dados->memPar->posL], sizeof(CelulaBuffer));
		dados->memPar->posL++; //incrementamos a posicao de leitura para o proximo consumidor ler na posicao seguinte

		//se apos o incremento a posicao de leitura chegar ao fim, tenho de voltar ao inicio
		if (dados->memPar->posL == TAM_BUFFER)
			dados->memPar->posL = 0;

		//libertamos o mutex
		ReleaseMutex(dados->hMutex);

		//libertamos o semaforo. temos de libertar uma posicao de escrita
		ReleaseSemaphore(dados->hSemEscrita, 1, NULL);

		//contador++;
		//soma += cel.val;
		_tprintf(TEXT("O%d comando: %d, %d.\n"), cel.id, cel.comando[0], cel.comando[1]);
	}
	//_tprintf(TEXT("C%d consumiu %d items.\n"), dados->id, soma);

	return 0;
}
//-------------------------------------------------------------------------------------


int _tmain(int argc, LPTSTR argv[]) {
	GameInfo game;
	DadosThreads dados;
	srand(time(NULL));
	

	TCHAR COM[50]; //COMANDOS
	//GameState gamestate;
	game.nFaixas = 0;
	game.velocCarros = 0;

	game.state = 0;



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
			_tscanf_s(TEXT("%d"), &game.nFaixas);
		} while (game.nFaixas > 10);

		_tprintf(TEXT("Velocidade Inicial carros: "));
		_tscanf_s(TEXT("%d"), &game.velocCarros);
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
		
		if (game.velocCarros == 0 && game.nFaixas == 0) {
			do {
				_tprintf(TEXT("\nNúmero de faixas: "));
				_tscanf_s(TEXT("%d"), &game.nFaixas);
			} while (game.nFaixas > 10);

			_tprintf(TEXT("\nVelocidade Inicial carros: "));
			_tcscanf_s(TEXT("%d"), &game.velocCarros);
		}

		if(RegSetValueEx(key, TEXT("NFaixas"), 0, REG_DWORD, (LPBYTE)&game.nFaixas, sizeof(game.nFaixas)) == ERROR_SUCCESS)
			_tprintf(TEXT("\nNamed value 'NFaixas' was created"));

		if(RegSetValueEx(key, TEXT("VCarrosI"), 0, REG_DWORD, (LPBYTE)&game.velocCarros, sizeof(game.velocCarros)) == ERROR_SUCCESS)
			_tprintf(TEXT("\nNamed value 'VCarrosI' was created"));

		
	}
		
	else {
		_tprintf(TEXT("A chave foi aberta:%s\n"), key_name);

		if (game.velocCarros != 0 && game.nFaixas != 0) {

			if (RegSetValueEx(key, TEXT("NFaixas"), 0, REG_DWORD, (LPBYTE)&game.nFaixas, sizeof(game.nFaixas)) == ERROR_SUCCESS)
				_tprintf(TEXT("\nNamed value 'NFaixas' was edited"));

			if (RegSetValueEx(key, TEXT("VCarrosI"), 0, REG_DWORD, (LPBYTE)&game.velocCarros, sizeof(game.velocCarros)) == ERROR_SUCCESS)
				_tprintf(TEXT("\nNamed value 'VCarrosI' was edited"));

		}
		else {

			if (RegQueryValueEx(key, TEXT("NFaixas"), NULL, NULL, (LPBYTE)&game.nFaixas, &size) == ERROR_SUCCESS) {
				_tprintf(TEXT("\nNFaixas: %d"), game.nFaixas);
			}
			if (RegQueryValueEx(key, TEXT("VCarrosI"), NULL, NULL, (LPBYTE)&game.velocCarros, &size) == ERROR_SUCCESS) {
				_tprintf(TEXT("\nVCarrosI: %d"), game.velocCarros);
			}
			
			
		}
	}
		
	RegCloseKey(key);
	Sleep(2000); //EM PROGRESSO <-----------------------------

	//------------------------------------------------------------------BUFFER CIRCULAR-----------------------------------------------------------------------

	HANDLE hFileMap; //handle para o file map
	HANDLE hThread;
	//DadosThreads dados;
	//TCHAR comando[100];
	BOOL  primeiroProcesso = FALSE;


	//criar semaforo que conta as escritas
	dados.hSemEscrita = CreateSemaphore(NULL, TAM_BUFFER, TAM_BUFFER, TEXT("SO2_SEMAFORO_ESCRITA"));


	//criar semaforo que conta as leituras
	//0 porque nao ha nada para ser lido e depois podemos ir até um maximo de 10 posicoes para serem lidas
	dados.hSemLeitura = CreateSemaphore(NULL, 0, TAM_BUFFER, TEXT("SO2_SEMAFORO_LEITURA"));

	//criar mutex para os SERVIDORES
	dados.hMutex = CreateMutex(NULL, FALSE, TEXT("SO2_MUTEX_SERVIDOR"));

	if (dados.hSemEscrita == NULL || dados.hSemLeitura == NULL) {
		_tprintf(TEXT("Erro no CreateSemaphore\n"));
		return -1;
	}


	//o openfilemapping vai abrir um filemapping com o nome que passamos no lpName
	//se devolver um HANDLE ja existe entao fechamos
	//se devolver NULL nao existe e vamos fazer a inicializacao

	if (OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, TEXT("SO2_MEM_PARTILHADA"))) {
		_tprintf(TEXT("Servidor já se encontra aberto\n"));
		return -1;
	}
	else{
		primeiroProcesso = TRUE;
		//criamos o bloco de memoria partilhada
		hFileMap = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			sizeof(BufferCircular), //tamanho da memoria partilhada
			TEXT("SO2_MEM_PARTILHADA"));//nome do filemapping. nome que vai ser usado para partilha entre processos

		if (hFileMap == NULL) {
			_tprintf(TEXT("Erro no CreateFileMapping\n"));
			return -1;
		}
	}

	//mapeamos o bloco de memoria para o espaco de enderaçamento do nosso processo
	dados.memPar = (BufferCircular*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	if (dados.memPar == NULL) {
		_tprintf(TEXT("Erro no MapViewOfFile\n"));
		return -1;
	}

	if (primeiroProcesso == TRUE) {
		dados.memPar->nOperadores = 0;
		dados.memPar->posE = 0;
		dados.memPar->posL = 0;
	}

	dados.terminar = 0;

	

	//lancamos a thread
	hThread = CreateThread(NULL, 0, ThreadProdutor, &dados, 0, NULL);

	if (hThread == NULL) 
		_tprintf(TEXT("Problemas com a thread do buffer circular ...\n"));
	
	
	


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
	
	
	
	
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	FillConsoleOutputCharacter(hStdout, _T(' '), 1, t, &size);



	int random_number = rand() % 20;
	game.tabuleiro->tab[game.nFaixas-1][random_number] = 1;

	do {
		random_number = rand() % 20;
	} while (game.tabuleiro->tab[game.nFaixas - 1][random_number] == 1);

	game.tabuleiro->tab[game.nFaixas-1][random_number] = 1;


	for (int i = 0; i < game.nFaixas; i++) {
		_tprintf(TEXT("|"));
		for (int y = 0; y < COLUNAS; y++)
			if(game.tabuleiro->tab[i][y]!=1)
				_tprintf(TEXT(" "));
			else
				_tprintf(TEXT("s"));
		_tprintf(TEXT("|\n"));
	}


	dados.game = &game;


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

			if (game.state == 1)
				game.state = 0;
			else
				game.state = 1;

		}
		else if (_tcscmp(COM, TEXT("restart"))==0)
			game.state = 2;
			

		_tprintf(TEXT("Estado: %d\n"), game.state);

		

	} while (_tcscmp(COM, TEXT("close"))!=0 && game.state != 3);

	game.state = 3;

	//dados.terminar = 1;

	//esperar que a thread termine
	WaitForSingleObject(hThread, INFINITE);
	UnmapViewOfFile(dados.memPar);
	//CloseHandles ... mas é feito automaticamente quando o processo termina
	

	return 0;
}