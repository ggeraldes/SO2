#include "../../header.h"

DWORD WINAPI ThreadOperador(LPVOID param) {
	DadosThreads* dados = (DadosThreads*)param;
	CelulaBuffer cel;
	int contador = 0;
	TCHAR COM[50];


	while (!dados->terminar) {
		cel.id = dados->id;

		COORD pos;
		pos.X = 0;
		pos.Y = 12;
		DWORD size;
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		do {
			FillConsoleOutputCharacter(hConsole, _T(' '), 80 * 1, pos, &size);
			SetConsoleCursorPosition(hConsole, pos);

			_tprintf(TEXT("COMANDO: "));
			_fgetts(COM, 50, stdin);
			COM[_tcslen(COM) - 1] = '\0';
		} while (_tcscmp(COM, TEXT("close")) != 0 && _tcscmp(COM, TEXT("stop")) != 0 && _tcscmp(COM, TEXT("invert")) != 0 && _tcscmp(COM, TEXT("insert")) != 0);

		if (_tcscmp(COM, TEXT("stop")) == 0) {

			cel.comando[0] = 1;
			_tprintf(TEXT("TEMPO: "));
			_tscanf_s(TEXT("%d"), &cel.comando[1]);

		}
		else if (_tcscmp(COM, TEXT("invert")) == 0) {

			cel.comando[0] = 2;
			_tprintf(TEXT("FAIXA: "));
			_tscanf_s(TEXT("%d"), &cel.comando[1]);

		}
		else if (_tcscmp(COM, TEXT("insert")) == 0) {

			cel.comando[0] = 3;
			_tprintf(TEXT("FAIXA: "));
			_tscanf_s(TEXT("%d"), &cel.comando[1]);
		}
		else if (_tcscmp(COM, TEXT("close")) == 0) 
			return 1;

		//aqui entramos na logica da aula teorica

		//esperamos por uma posicao para escrevermos
		WaitForSingleObject(dados->hSemEscrita, INFINITE);

		//esperamos que o mutex esteja livre
		WaitForSingleObject(dados->hMutex, INFINITE);

		//vamos copiar a variavel cel para a memoria partilhada (para a posição de escrita)
		CopyMemory(&dados->memPar->buffer[dados->memPar->posE], &cel, sizeof(CelulaBuffer));
		dados->memPar->posE++; //incrementamos a posicao de escrita para o proximo produtor escrever na posicao seguinte

		//se apos o incremento a posicao de escrita chegar ao fim, tenho de voltar ao inicio
		if (dados->memPar->posE == TAM_BUFFER)
			dados->memPar->posE = 0;

		//libertamos o mutex
		ReleaseMutex(dados->hMutex);

		//libertamos o semaforo. temos de libertar uma posicao de leitura
		ReleaseSemaphore(dados->hSemLeitura, 1, NULL);

		contador++;
		_tprintf(TEXT("EU: %d, %d\n"), cel.comando[0], cel.comando[1]);
		//Sleep(num_aleatorio(2, 4) * 1000);
	}
	//_tprintf(TEXT("Sevidor: %d items.\n"), contador);

	return 0;
}

//------------------------------------------ MEMORIA PARTILHADA -------------------------------------------------------------
DWORD WINAPI RecebeTabuleiro(LPVOID param) {
	//TCHAR msg[NUM_CHAR];
	ThreadTab* dados = (ThreadTab*)param;
	
	COORD t, y;
	t.X = 0; y.X = 9;
	t.Y = 0; y.Y = 12;
	DWORD size;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE hBack = hConsole;
	while (1) {



		//esperar até que evento desbloqueie
		WaitForSingleObject(dados->hEvent, INFINITE);


		//verifica se é preciso terminar a thread ou nao
		if (dados->terminar)
			break;



		//faço o lock para o mutex
		WaitForSingleObject(dados->hMutex, INFINITE);

		FillConsoleOutputCharacter(hConsole, _T(' '), 80 * 1, t, &size);
		SetConsoleCursorPosition(hConsole, t);
		for (int i = 0; i < dados->fileViewMap->nFaixas; i++) {
			_tprintf(TEXT("|"));
			for (int y = 0; y < COLUNAS; y++)
				if (dados->fileViewMap->faixa[i].col[y].val == 1) {
					SetConsoleTextAttribute(hConsole, (FOREGROUND_GREEN));
					_tprintf(TEXT("s"));
					SetConsoleTextAttribute(hConsole, (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED));
				}
				else if (dados->fileViewMap->faixa[i].col[y].val == 2) {
					SetConsoleTextAttribute(hConsole, (FOREGROUND_BLUE));
					_tprintf(TEXT("c"));
					SetConsoleTextAttribute(hConsole, (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED));
				}
				else
					_tprintf(TEXT(" "));
			_tprintf(TEXT("|\n"));

		}
		SetConsoleCursorPosition(hConsole, y);
		//faço unlock do mutex
		ReleaseMutex(dados->hMutex);

		//Sleep(10000);
	}

	return 0;
}
//-------------------------------------------------------------------------------------

int _tmain(int argc, LPTSTR argv[]) {


#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	//------------------------------------------ BUFFER CIRCULAR -------------------------------------------------------------
	HANDLE hFileMapBC; //handle para o file map
	HANDLE hThread[2];
	DadosThreads dados;
	BOOL  primeiroProcesso = FALSE;

	//criar semaforo que conta as escritas
	dados.hSemEscrita = CreateSemaphore(NULL, TAM_BUFFER, TAM_BUFFER, TEXT("SO2_SEMAFORO_ESCRITA"));

	//criar semaforo que conta as leituras
	//0 porque nao ha nada para ser lido e depois podemos ir até um maximo de 10 posicoes para serem lidas
	dados.hSemLeitura = CreateSemaphore(NULL, 0, TAM_BUFFER, TEXT("SO2_SEMAFORO_LEITURA"));

	//criar mutex para os produtores
	dados.hMutex = CreateMutex(NULL, FALSE, TEXT("SO2_MUTEX_CONSUMIDOR"));

	if (dados.hSemEscrita == NULL || dados.hSemLeitura == NULL || dados.hMutex == NULL) {
		_tprintf(TEXT("Erro no CreateSemaphore ou no CreateMutex\n"));
		return -1;
	}

	//o openfilemapping vai abrir um filemapping com o nome que passamos no lpName
   //se devolver um HANDLE ja existe e nao fazemos a inicializacao
   //se devolver NULL nao existe e vamos fazer a inicializacao

	hFileMapBC = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, TEXT("SO2_MEM_PARTILHADA"));
	if (hFileMapBC == NULL) {
		_tprintf(TEXT("Servidor não se encontra aberto\n"));
		return -1;
	}

	//mapeamos o bloco de memoria para o espaco de enderaçamento do nosso processo
	dados.memPar = (BufferCircular*)MapViewOfFile(hFileMapBC, FILE_MAP_ALL_ACCESS, 0, 0, 0);


	if (dados.memPar == NULL) {
		_tprintf(TEXT("Erro no MapViewOfFile\n"));
		return -1;
	}

	dados.terminar = 0;

	//temos de usar o mutex para aumentar o nOperadores para termos os ids corretos
	WaitForSingleObject(dados.hMutex, INFINITE);
	dados.memPar->nOperadores++;
	dados.id = dados.memPar->nOperadores;
	ReleaseMutex(dados.hMutex);


	//lancamos a thread
	hThread[0] = CreateThread(NULL, 0, ThreadOperador, &dados, 0, NULL);
	if (hThread[0] == NULL) {
		//_tprintf(TEXT("Escreva qualquer coisa para sair ...\n"));
		//_getts_s(comando, 100);
		//dados.terminar = 1;
		return -1;
		
	}
	//---------------------------------------------------------------------------TABULEIRO-----------------------------------------------------------------------
	//---------------------- MEMORIA PARTILHADA ------------------------------
	HANDLE hFileMap;
	ThreadTab tab;
	//HANDLE hThreads;
	//HANDLE hSem;


	 //mapeia ficheiro num bloco de memoria
	hFileMap = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(GameInfo), // alterar o tamanho do filemapping
		TEXT("TP_MEM_OPERADOR")); //nome do file mapping, tem de ser único

	if (hFileMap == NULL) {
		_tprintf(TEXT("Erro no CreateFileMapping\n"));
		//CloseHandle(hFile); //recebe um handle e fecha esse handle , no entanto o handle é limpo sempre que o processo termina
		return 1;
	}

	//mapeia bloco de memoria para espaço de endereçamento
	tab.fileViewMap = (GameInfo*)MapViewOfFile(
		hFileMap,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		0);

	if (tab.fileViewMap == NULL) {
		_tprintf(TEXT("Erro no MapViewOfFile\n"));
		return 1;
	}


	tab.hEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		TEXT("SO2_EVENTO"));

	if (tab.hEvent == NULL) {
		_tprintf(TEXT("Erro no CreateEvent\n"));
		UnmapViewOfFile(tab.fileViewMap);
		return 1;
	}

	tab.hMutex = CreateMutex(
		NULL,
		FALSE,
		TEXT("SO2_MUTEX"));

	if (tab.hMutex == NULL) {
		_tprintf(TEXT("Erro no CreateMutex\n"));
		UnmapViewOfFile(tab.fileViewMap);
		return 1;
	}

	tab.terminar = 0;

	hThread[1] = CreateThread(NULL, 0, RecebeTabuleiro, &tab, 0, NULL);
	if (hThread[1] == NULL) {
		//_tprintf(TEXT("Escreva qualquer coisa para sair ...\n"));
		//_getts_s(comando, 100);
		//dados.terminar = 1;
		return -1;

	}

	//--------------------------------
	//esperar que a thread termine
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
	UnmapViewOfFile(dados.memPar);
	//CloseHandles ... mas é feito automaticamente quando o processo termina
	
	return 0;
}
