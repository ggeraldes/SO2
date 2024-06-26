#include "../header.h"


COORD geral;
CONSOLE_SCREEN_BUFFER_INFO csbi;

typedef struct {
	int posX, posY; //posicao
	// mov; //movimentos
}Sapo;

GameInfo game;
Sapo posSapo[2];

//---------------------------------------------------PIPES---------------------------------------------

#define PIPE_READ TEXT("\\\\.\\pipe\\SAP-SER")
#define PIPE_WRITE TEXT("\\\\.\\pipe\\SER-SAP")

typedef struct {
	HANDLE hPipe[2];
	HANDLE hMutex; //para controlar o numClientes
	int numClientes;
	int terminar;
}ThreadDados;

//envia mensagem para todos os leitores que est�o disponiveis
DWORD WINAPI ThreadEscrevePipe(LPVOID param) {
	ThreadDados* dados = (ThreadDados*)param;
	TCHAR buf[256];
	DWORD n;
	int i;

	//aqui , o servidor j� recebeu um cliente
	do {
		//vai buscar informa��o � consola
		//_tprintf(TEXT("[ESCRITOR] Frase: "));
		//_fgetts(buf, 256, stdin);
		//buf[_tcslen(buf) - 1] = '\0';

		//bloqueamos aqui porque � uma regiao critica
		WaitForSingleObject(dados->hMutex, INFINITE);

		//escreve no named pipe
		for (i = 0; i < dados->numClientes; i++) {
			if (!WriteFile(dados->hPipe[i], &game, sizeof(GameInfo), &n, NULL)) {
				_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
				exit(-1);
			}

			//_tprintf(TEXT("[ESCRITOR] Enviei %d bytes ao leitor [%d]... (WriteFile)\n"), n, i);
		}
		//libertamos o mutex
		ReleaseMutex(dados->hMutex);
		Sleep(game.velocCarros * 200);

	} while (_tcscmp(buf, TEXT("fim")));

	dados->terminar = 1;

	return 0;

}

DWORD WINAPI ThreadLePipe(LPVOID param) {
	ThreadDados* dados = (ThreadDados*)param;
	int comando=0;
	DWORD n;
	BOOL ret;
	do {
		// Aguarda permiss�o para ler
		WaitForSingleObject(dados->hMutex, INFINITE);
		for (int i = 0; i < dados->numClientes; i++) {


			ret = ReadFile(dados->hPipe[i], &comando, sizeof(comando), &n, NULL);

			

			if (!ret || !n) {
				_tprintf(TEXT("[LEITOR] %d %d... (ReadFile)\n"), ret, n);
				break;
			}

			//_tprintf(TEXT("[LEITOR] Recebi %d bytes: '%s'... (ReadFile)\n"), n, buf);
			WaitForSingleObject(game.gameMutex, INFINITE);
				if (comando == 1) {
					if (posSapo[0].posX-1 == 0) {
						game.faixa[posSapo[0].posX].col[posSapo[0].posY].val = 0;
						game.faixa[posSapo[0].posX-1].col[posSapo[0].posY].val = 1;
						posSapo[0].posX = posSapo[0].posX - 1;
						printf("SAPO 1 VENCEU!");
						game.state = 3;

					}
					if (game.faixa[posSapo[0].posX - 1].col[posSapo[0].posY].val == 2 || game.faixa[posSapo[0].posX - 1].col[posSapo[0].posY].val == 3) {
						game.faixa[posSapo[0].posX].col[posSapo[0].posY].val = 0;
						game.faixa[game.nFaixas - 1].col[posSapo[0].posY].val = 1;
						posSapo[0].posX = game.nFaixas-1;
					}
					else {
						game.faixa[posSapo[0].posX].col[posSapo[0].posY].val = 0;
						game.faixa[posSapo[0].posX-1].col[posSapo[0].posY].val = 1;
						posSapo[0].posX = posSapo[0].posX - 1;
					}
				}
				else if (comando == 2) {
					if (posSapo[0].posX + 1 <= game.nFaixas-1) {
						if (game.faixa[posSapo[0].posX + 1].col[posSapo[0].posY].val == 2 || game.faixa[posSapo[0].posX + 1].col[posSapo[0].posY].val == 3) {
							game.faixa[posSapo[0].posX].col[posSapo[0].posY].val = 0;
							game.faixa[game.nFaixas - 1].col[posSapo[0].posY].val = 1;
							posSapo[0].posX = game.nFaixas - 1;
						}
						else {
							game.faixa[posSapo[0].posX].col[posSapo[0].posY].val = 0;
							game.faixa[posSapo[0].posX + 1].col[posSapo[0].posY].val = 1;
							posSapo[0].posX = posSapo[0].posX + 1;
						}

					}
					
				}
				else if (comando == 3) {
					if (posSapo[0].posY - 1 >= 0) {
						if (game.faixa[posSapo[0].posX].col[posSapo[0].posY - 1].val == 2 || game.faixa[posSapo[0].posX].col[posSapo[0].posY - 1].val == 3) {
							game.faixa[posSapo[0].posX].col[posSapo[0].posY].val = 0;
							game.faixa[game.nFaixas - 1].col[posSapo[0].posY].val = 1;
							posSapo[0].posX = game.nFaixas - 1;
						}
						else {
							game.faixa[posSapo[0].posX].col[posSapo[0].posY].val = 0;
							game.faixa[posSapo[0].posX].col[posSapo[0].posY - 1].val = 1;
							posSapo[0].posY = posSapo[0].posY - 1;
						}
					}
				}
				else if (comando == 4) {
					if (posSapo[0].posY + 1 <= COLUNAS-1) {
						if (game.faixa[posSapo[0].posX].col[posSapo[0].posY + 1].val == 2 || game.faixa[posSapo[0].posX].col[posSapo[0].posY + 1].val == 3) {
							game.faixa[posSapo[0].posX].col[posSapo[0].posY].val = 0;
							game.faixa[game.nFaixas - 1].col[posSapo[0].posY].val = 1;
							posSapo[0].posX = game.nFaixas - 1;
						}
						else {
							game.faixa[posSapo[0].posX].col[posSapo[0].posY].val = 0;
							game.faixa[posSapo[0].posX].col[posSapo[0].posY + 1].val = 1;
							posSapo[0].posY = posSapo[0].posY + 1;
						}
					}
				}
			ReleaseMutex(game.gameMutex);


		}
		// Libera a permiss�o de escrita
		ReleaseMutex(dados->hMutex);
	} while (game.state!=3);

	return;
}

DWORD WINAPI ThreadLigacoesOUT(LPVOID param) {
	ThreadDados* dados = (ThreadDados*)param;
	int i = 0;
	HANDLE hPipe;
	while (i < 1) {
		i++;
		// PIPE_ACCESS_OUTBOUND -> o ESCRITOR escreve e o LEITOR recebe
		//1� nome do named pipe, 2�configura��o da abertura, 3�configura��es do pipe,4� numero maximo de instancias,5� e 6�tamanho do buffer de entrada e de saida
		//7� timeout, 8� security atributes
		hPipe = CreateNamedPipe(PIPE_WRITE, PIPE_ACCESS_DUPLEX, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 2, sizeof(GameInfo), sizeof(GameInfo), 1000, NULL);

		if (hPipe == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("[ERRO] Criar Named Pipe! (CreateNamedPipe)"));
			exit(-1);
		}


		_tprintf(TEXT("[ESCRITOR] Esperar liga��o de um leitor... (ConnectNamedPipe)\n"));

		//o servidor espera at� ter um cliente conectado a esta inst�ncia
		//bloqueia aqui
		if (!ConnectNamedPipe(hPipe, NULL)) {
			_tprintf(TEXT("[ERRO] Liga��o ao leitor! (ConnectNamedPipe\n"));
			exit(-1);
		}
		else
			_tprintf(TEXT("Sapo ligado\n"));

		//bloqueamos no mutex
		WaitForSingleObject(dados->hMutex, INFINITE);

		//preenchemos o array
		dados->hPipe[dados->numClientes] = hPipe;
		dados->numClientes++;

		//libertamos o mutex
		ReleaseMutex(dados->hMutex);

	}
	return;

}

DWORD WINAPI ThreadLigacoesIN(LPVOID param) {
	ThreadDados* dados = (ThreadDados*)param;
	int i = 0;
	HANDLE hPipe;
	while (i < 1) {
		i++;
		// PIPE_ACCESS_OUTBOUND -> o ESCRITOR escreve e o LEITOR recebe
		//1� nome do named pipe, 2�configura��o da abertura, 3�configura��es do pipe,4� numero maximo de instancias,5� e 6�tamanho do buffer de entrada e de saida
		//7� timeout, 8� security atributes
		hPipe = CreateNamedPipe(PIPE_READ, PIPE_ACCESS_DUPLEX, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 2, 256 * sizeof(TCHAR), 256 * sizeof(TCHAR), 1000, NULL);

		if (hPipe == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("[ERRO] Criar Named Pipe! (CreateNamedPipe)"));
			exit(-1);
		}


		_tprintf(TEXT("[ESCRITOR] Esperar liga��o de um leitor... (ConnectNamedPipe)\n"));

		//o servidor espera at� ter um cliente conectado a esta inst�ncia
		//bloqueia aqui
		if (!ConnectNamedPipe(hPipe, NULL)) {
			_tprintf(TEXT("[ERRO] Liga��o ao leitor! (ConnectNamedPipe\n"));
			exit(-1);
		}
		else
			_tprintf(TEXT("Sapo ligado\n"));

		//bloqueamos no mutex
		WaitForSingleObject(dados->hMutex, INFINITE);

		//preenchemos o array
		dados->hPipe[dados->numClientes] = hPipe;
		dados->numClientes++;

		//libertamos o mutex
		ReleaseMutex(dados->hMutex);

	}
	return;

}

//-----------------------------------------------------------------------------------------------------

void newTab() {
	WaitForSingleObject(game.gameMutex, INFINITE);
		for (int i = 0; i < game.nFaixas; i++)
			for (int y = 0; y < COLUNAS; y++)
				game.faixa[i].col[y].val = 0;

		int random_number = rand() % 20;
		game.faixa[game.nFaixas - 1].col[random_number].val = 1;
		posSapo[0].posX = game.nFaixas - 1;
		posSapo[0].posY = random_number;

		do {
			random_number = rand() % 20;
		} while (game.faixa[game.nFaixas - 1].col[random_number].val == 1);

		game.faixa[game.nFaixas - 1].col[random_number].val = 1;
		posSapo[1].posX = game.nFaixas - 1;
		posSapo[1].posY = random_number;
	ReleaseMutex(game.gameMutex);
}

DWORD WINAPI ChangeTab(LPVOID param) {
	Faixa* faixa = (Faixa*)param;

	srand((unsigned)time(0) + GetCurrentThreadId());
	faixa->nCars = 0;

	//_tprintf(TEXT("\nSTATE:%d\n"), faixa->state);
	while (game.state != 3) {
		for (int i = 0; i < COLUNAS; i++)
			if (faixa->col[i].val == 2)
				faixa->nCars++;

		WaitForSingleObject(game.gameMutex, INFINITE);
		if (faixa->state == 1) {//normal
			for (int i = COLUNAS - 1; i > 0; i--) {

				if (faixa->col[i].val == 2 && i == COLUNAS - 1)
					faixa->col[i].val = 0;

				if(faixa->col[i-1].val != 1)
				if (faixa->col[i].val != 3 && faixa->col[i - 1].val != 3 && faixa->col[i].val != 2) {
					if (faixa->col[i].val == 1 && faixa->col[i-1].val == 2) {
						game.faixa[game.nFaixas - 1].col[posSapo[0].posY].val = 1;
						posSapo[0].posX = game.nFaixas - 1;
						faixa->col[i].val = faixa->col[i - 1].val;
						faixa->col[i - 1].val = 0;
					}
					if (faixa->col[i].val != 1)
					{
						faixa->col[i].val = faixa->col[i - 1].val;
						faixa->col[i - 1].val = 0;
					}
						

				}

			}
			if (random_60_40() == 2 && faixa->nCars < MAXCARROS)
				if (faixa->col[0].val != 3)
					faixa->col[0].val = 2;


			//_tprintf(TEXT("\nCARS:%d\n"), faixa->nCars);
		}
		else if (faixa->state == 2) {//inverso
			for (int i = 0; i < COLUNAS; i++) {

				if (faixa->col[i].val == 2 && i == 0)
					faixa->col[i].val = 0;

				if (faixa->col[i + 1].val != 1)
				if (faixa->col[i].val != 3 && faixa->col[i + 1].val != 3 && faixa->col[i].val != 2) {

					if (faixa->col[i].val == 1 && faixa->col[i + 1].val == 2) {
						game.faixa[game.nFaixas - 1].col[posSapo[0].posY].val = 1;
						posSapo[0].posX = game.nFaixas - 1;
						faixa->col[i].val = faixa->col[i + 1].val;
						faixa->col[i + 1].val = 0;
					}
					if (faixa->col[i].val != 1) {
						faixa->col[i].val = faixa->col[i + 1].val;
						faixa->col[i + 1].val = 0;
					}
					

				}

			}
			if (random_60_40() == 2 && faixa->nCars < MAXCARROS)
				if (faixa->col[COLUNAS - 1].val != 3)
					faixa->col[COLUNAS - 1].val = 2;


		}




		//x++;
		faixa->nCars = 0;
		ReleaseMutex(game.gameMutex);
		if (game.stop) {
			Sleep(game.stopTime * 1000);
			game.stop = FALSE;
		}
		else if (game.state == 1)
			do { Sleep(1000); } while (game.state == 1);
		else
			Sleep(game.velocCarros * 200);


	}
	//_tprintf(TEXT("C%d consumiu %d items.\n"), dados->id, soma);

	return 0;
}
//------------------------------------------ BUFFER CIRCULAR -------------------------------------------------------------
DWORD WINAPI ThreadServidor(LPVOID param) {
	DadosThreads* dados = (DadosThreads*)param;
	CelulaBuffer cel;
	//int contador = 0;
	//int soma = 0;

	COORD t, comands;
	t.X = 0; comands.X = 26;
	//t.Y = 25; 
	comands.Y = 1;
	//DWORD size;

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	while (!(dados->terminar)) {
		//FillConsoleOutputCharacter(hConsole, _T(' '), 80 * 1, t, &size);
		//SetConsoleCursorPosition(hConsole, t);
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

		GetConsoleScreenBufferInfo(hConsole, &csbi);
		//contador++;
		//soma += cel.val;

		//_tprintf(TEXT("O%d comando: %d, %d.\n"), cel.id, cel.comando[0], cel.comando[1]);
		//t.X++;
		if (cel.comando[0] == 1) {
			
				game.stop = TRUE;
				game.stopTime = cel.comando[1];

			
			SetConsoleCursorPosition(hConsole, comands);
			_tprintf(TEXT("O%d: stop %d segundos\n"), cel.id, cel.comando[1]);
			comands.Y++;
			SetConsoleCursorPosition(hConsole, geral);

		}
		else if (cel.comando[0] == 2) { //inverter sentido
			if (game.faixa[cel.comando[1]].state == 1)
				game.faixa[cel.comando[1]].state = 2;
			else
				game.faixa[cel.comando[1]].state = 1;

			SetConsoleCursorPosition(hConsole, comands);
			_tprintf(TEXT("O%d: invert faixa %d\n"), cel.id, cel.comando[1]);
			comands.Y++;
			SetConsoleCursorPosition(hConsole, geral);

		}
		else if (cel.comando[0] == 3) { //inserir obstaculos
			game.faixa[cel.comando[1]].col[cel.comando[2]].val = 3;

			SetConsoleCursorPosition(hConsole, comands);
			_tprintf(TEXT("O%d: insert faixa %d, coluna %d\n"), cel.id, cel.comando[1], cel.comando[2]);
			comands.Y++;
			SetConsoleCursorPosition(hConsole, geral);

		}


	}
	//_tprintf(TEXT("C%d consumiu %d items.\n"), dados->id, soma);

	return 0;
}
//-------------------------------------------------------------------------------------

//------------------------------------------ MEMORIA PARTILHADA -------------------------------------------------------------
DWORD WINAPI EnviaTabuleiro(LPVOID param) {
	Sleep(1000);
	//TCHAR msg[NUM_CHAR];
	ThreadTab* dados = (ThreadTab*)param;
	COORD t, y;
	t.X = 0; y.X = 9;
	t.Y = 0; y.Y = 12;
	DWORD size;

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	while (!(dados->terminar)) {

		GetConsoleScreenBufferInfo(hConsole, &csbi);

		FillConsoleOutputCharacter(hConsole, _T(' '), 80 * 1, t, &size);
		SetConsoleCursorPosition(hConsole, t);

		_tprintf(TEXT("  "));
		for (int i = 0; i < 10; i++)
			_tprintf(TEXT("%d"), i);
		for (int i = 0; i < 10; i++)
			_tprintf(TEXT("%d"), i);
		_tprintf(TEXT("\n"));

		for (int i = 0; i < game.nFaixas; i++) {
			_tprintf(TEXT("%d|"), i);
			for (int y = 0; y < COLUNAS; y++)
				if (game.faixa[i].col[y].val == 1) {
					SetConsoleTextAttribute(hConsole, (FOREGROUND_GREEN));
					_tprintf(TEXT("s"));
					SetConsoleTextAttribute(hConsole, (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED));
				}
				else if (game.faixa[i].col[y].val == 2) {
					SetConsoleTextAttribute(hConsole, (FOREGROUND_RED));
					_tprintf(TEXT("c"));
					SetConsoleTextAttribute(hConsole, (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED));
				}
				else if (game.faixa[i].col[y].val == 3) {
					SetConsoleTextAttribute(hConsole, (FOREGROUND_BLUE));
					_tprintf(TEXT("B"));
					SetConsoleTextAttribute(hConsole, (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED));
				}
				else
					_tprintf(TEXT(" "));
			_tprintf(TEXT("|\n"));

		}
		geral = csbi.dwCursorPosition;
		SetConsoleCursorPosition(hConsole, geral);
		//Sleep(1000);

	//fa�o lock ao mutex
		WaitForSingleObject(dados->hMutex, INFINITE);

		//limpa memoria antes de fazer a copia
		ZeroMemory(dados->fileViewMap, sizeof(GameInfo));
		//WaitForSingleObject(game.gameMutex, INFINITE);
		//copia memoria de um sitio para outro (aqui copia a mensagem escrita no terminal para o fileViewMap)
		CopyMemory(dados->fileViewMap, &game, sizeof(GameInfo));
		//ReleaseMutex(game.gameMutex);
		//liberto mutex
		ReleaseMutex(dados->hMutex);

		//criamos evento
		SetEvent(dados->hEvent);
		if (game.stop) {
			Sleep(game.stopTime * 1000);
			//game.stop = FALSE;
		}
		else if (game.state == 1)
			do { Sleep(1000); } while (game.state == 1);
		else
			Sleep(game.velocCarros * 200);

		ResetEvent(dados->hEvent); //torna o evento novamente n�o assinalado
	}

	return 0;
}
//-------------------------------------------------------------------------------------

int _tmain(int argc, LPTSTR argv[]) {

	//DadosThreads dados;

	srand((unsigned)time(0) + GetCurrentThreadId());


	TCHAR COM[50]; //COMANDOS
	//GameState gamestate;
	game.nFaixas = 0;
	game.velocCarros = 0;
	game.stop = FALSE;
	game.stopTime = 0;
	game.state = 0;


	HKEY key; //Handle para a chave depois aberta/criada
	TCHAR key_name[TAM] = TEXT("Software\\TP\\KEYFrogger");
	DWORD result; // o que aconteceu com a chave
	DWORD size;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	//-------------------------------------------------------------------------------------------------------------
	HANDLE hFileMapBC; //handle para o file map
	HANDLE hThread[6];
	BOOL  primeiroProcesso = FALSE;


	//o openfilemapping vai abrir um filemapping com o nome que passamos no lpName
	//se devolver um HANDLE ja existe entao fechamos
	//se devolver NULL nao existe e vamos fazer a inicializacao

	if (OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, TEXT("SO2_MEM_PARTILHADA"))) {
		_tprintf(TEXT("Servidor j� se encontra aberto\n"));
		return -1;
	}

	primeiroProcesso = TRUE;
	//criamos o bloco de memoria partilhada
	hFileMapBC = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(BufferCircular), //tamanho da memoria partilhada
		TEXT("SO2_MEM_PARTILHADA"));//nome do filemapping. nome que vai ser usado para partilha entre processos

	if (hFileMapBC == NULL) {
		_tprintf(TEXT("Erro no CreateFileMapping\n"));
		return -1;
	}
	//-----------------------------------------------------------------------------------------------------------

	do {
		_tprintf(TEXT("Fazer especifica��o inicial? "));
		_fgetts(COM, 50, stdin);
		COM[_tcslen(COM) - 1] = '\0';


	} while (_tcscmp(COM, TEXT("yes")) != 0 && _tcscmp(COM, TEXT("no")) != 0);

	if (_tcscmp(COM, TEXT("yes")) == 0) {
		do {
			_tprintf(TEXT("N�mero de faixas: "));
			_tscanf_s(TEXT("%d"), &game.nFaixas);
		} while (game.nFaixas > 10 || game.nFaixas < 3);

		_tprintf(TEXT("Velocidade Inicial carros: "));
		_tscanf_s(TEXT("%d"), &game.velocCarros);
		//fflush(stdin);
	}

	//-----------------------------------------------------------------------------------REGISTRY---------------------------------------------------------------------
	//Criar ou abrir uma chave do Registry
	if (RegCreateKeyEx(HKEY_CURRENT_USER, key_name, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &result) != ERROR_SUCCESS) {
		_tprintf(TEXT("Chave nao foi criada nem aberta ! ERRO!"));
		return -1;
	}

	if (result == REG_CREATED_NEW_KEY) {

		_tprintf(TEXT("A chave foi criada: %s"), key_name);

		if (game.velocCarros == 0 && game.nFaixas == 0) {
			do {
				_tprintf(TEXT("\nN�mero de faixas: "));
				_tscanf_s(TEXT("%d"), &game.nFaixas);
			} while (game.nFaixas > 10 && game.nFaixas < 3);

			_tprintf(TEXT("\nVelocidade Inicial carros: "));
			_tcscanf_s(TEXT("%d"), &game.velocCarros);
		}

		if (RegSetValueEx(key, TEXT("NFaixas"), 0, REG_DWORD, (LPBYTE)&game.nFaixas, sizeof(game.nFaixas)) == ERROR_SUCCESS)
			_tprintf(TEXT("\nNamed value 'NFaixas' was created"));

		if (RegSetValueEx(key, TEXT("VCarrosI"), 0, REG_DWORD, (LPBYTE)&game.velocCarros, sizeof(game.velocCarros)) == ERROR_SUCCESS)
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
	for (int i = 0; i < game.nFaixas; i++) {
		game.faixa[i].id = i;
		game.faixa[i].state = random_0_1_2();
		//_tprintf(TEXT("\nyooooooooooo1:%d\n"), game.faixa[i].state);
	}

	Sleep(2000); //EM PROGRESSO <-----------------------------

	//------------------------------------------------------------------BUFFER CIRCULAR-----------------------------------------------------------------------

	DadosThreads dados;



	//criar semaforo que conta as escritas
	dados.hSemEscrita = CreateSemaphore(NULL, TAM_BUFFER, TAM_BUFFER, TEXT("SO2_SEMAFORO_ESCRITA"));


	//criar semaforo que conta as leituras
	//0 porque nao ha nada para ser lido e depois podemos ir at� um maximo de 10 posicoes para serem lidas
	dados.hSemLeitura = CreateSemaphore(NULL, 0, TAM_BUFFER, TEXT("SO2_SEMAFORO_LEITURA"));

	//criar mutex para os SERVIDORES
	dados.hMutex = CreateMutex(NULL, FALSE, TEXT("SO2_MUTEX_SERVIDOR"));

	if (dados.hSemEscrita == NULL || dados.hSemLeitura == NULL) {
		_tprintf(TEXT("Erro no CreateSemaphore\n"));
		return -1;
	}


	//mapeamos o bloco de memoria para o espaco de endera�amento do nosso processo
	dados.memPar = (BufferCircular*)MapViewOfFile(hFileMapBC, FILE_MAP_ALL_ACCESS, 0, 0, 0);

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
	hThread[0] = CreateThread(NULL, 0, ThreadServidor, &dados, 0, NULL);

	if (hThread[0] == NULL)
		_tprintf(TEXT("Problemas com a thread do buffer circular ...\n"));
	else
		_tprintf(TEXT("thread buffer circular inicializada ...\n"));




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
		TEXT("TP_MEM_OPERADOR")); //nome do file mapping, tem de ser �nico

	if (hFileMap == NULL) {
		_tprintf(TEXT("Erro no CreateFileMapping\n"));
		//CloseHandle(hFile); //recebe um handle e fecha esse handle , no entanto o handle � limpo sempre que o processo termina
		return 1;
	}

	//mapeia bloco de memoria para espa�o de endere�amento
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



	//--------------------------------

	COORD t;
	t.X = 0;
	t.Y = 0;

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



	FillConsoleOutputCharacter(hConsole, _T(' '), 1, t, &size);

	game.gameMutex = CreateMutex(NULL, FALSE, TEXT("GAME_MUTEX"));

	newTab();


	HANDLE ThreadChangeTab[10];

	for (int i = 1; i < game.nFaixas - 1; i++) {
		ThreadChangeTab[i] = CreateThread(NULL, 0, ChangeTab, &game.faixa[i], 0, NULL);

		if (ThreadChangeTab[i] == NULL)
			_tprintf(TEXT("Problemas com a thread de mudar a tabela ...\n"));
	}

	hThread[1] = CreateThread(NULL, 0, EnviaTabuleiro, &tab, 0, NULL);

	if (hThread[1] == NULL)
		_tprintf(TEXT("Problemas com a thread da tabela ...\n"));




	//Sleep(2000);


	//-----------------------------------------------------------------------------------------------------------------------------------------------------------

	//-------------------------------------------------------------------PIPES-----------------------------------------------------------------------------------
		
	HANDLE hPipe[2];
	ThreadDados sapo[2];

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	sapo[0].numClientes = 0;
	sapo[0].terminar = 0;
	sapo[0].hMutex = CreateMutex(NULL, FALSE, NULL); //Cria��o do mutex

	if (sapo[0].hMutex == NULL) {
		_tprintf(TEXT("[Erro] ao criar mutex!\n"));
		return -1;
	}

	sapo[1].numClientes = 0;
	sapo[1].terminar = 0;
	sapo[1].hMutex = CreateMutex(NULL, FALSE, NULL); //Cria��o do mutex

	if (sapo[1].hMutex == NULL) {
		_tprintf(TEXT("[Erro] ao criar mutex!\n"));
		return -1;
	}

	//criacao da thread
	hThread[2] = CreateThread(NULL, 0, ThreadEscrevePipe, &sapo[0], 0, NULL);
	if (hThread[2] == NULL) {
		_tprintf(TEXT("[Erro] ao criar thread!\n"));
		return -1;
	}

	_tprintf(TEXT("[ESCRITOR] Criar uma c�pia do pipe '%s' ... (CreateNamedPipe)\n"), PIPE_WRITE);



	hThread[3] = CreateThread(NULL, 0, ThreadLigacoesOUT, &sapo[0], 0, NULL);
	if (hThread[3] == NULL) {
		_tprintf(TEXT("[Erro] ao criar thread!\n"));
		return -1;
	}

	hThread[4] = CreateThread(NULL, 0, ThreadLePipe, &sapo[1], 0, NULL);
	if (hThread[4] == NULL) {
		_tprintf(TEXT("[Erro] ao criar thread!\n"));
		return -1;
	}

	hThread[5] = CreateThread(NULL, 0, ThreadLigacoesIN, &sapo[1], 0, NULL);
	if (hThread[5] == NULL) {
		_tprintf(TEXT("[Erro] ao criar thread!\n"));
		return -1;
	}
	


	//desligamos todos os clientes que se ligaram
	for (int i = 0; i < sapo[0].numClientes; i++) {
		_tprintf(TEXT("[ESCRITOR] Desligar o pipe (DisconnectNamedPipe)\n"));

		//desliga a instancia ao named pipe
		if (!DisconnectNamedPipe(sapo[0].hPipe[i])) {
			_tprintf(TEXT("[ERRO] Desligar o pipe! (DisconnectNamedPipe)"));
			exit(-1);
		}
	}

	

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------

	COORD pos;
	pos.X = 0;
	pos.Y = 12;


	do {


		FillConsoleOutputCharacter(hConsole, _T(' '), 80 * 1, pos, &size);
		SetConsoleCursorPosition(hConsole, pos);



		_tprintf(TEXT("COMANDO: "));
		_fgetts(COM, 50, stdin);

		COM[_tcslen(COM) - 1] = '\0';

		if (_tcscmp(COM, TEXT("pause")) == 0) {

			if (game.state == 1)
				game.state = 0;
			else
				game.state = 1;

		}
		else if (_tcscmp(COM, TEXT("restart")) == 0)
			newTab();



		//_tprintf(TEXT("Estado: %d\n"), game.state);



	} while (_tcscmp(COM, TEXT("close")) != 0 && game.state != 3);

	game.state = 3;

	dados.terminar = 1;
	tab.terminar = 1;

	//esperar que a thread termine
	WaitForMultipleObjects(6, hThread, TRUE, INFINITE);
	WaitForMultipleObjects(game.nFaixas-2, ThreadChangeTab, TRUE, INFINITE);
	UnmapViewOfFile(dados.memPar);

	//----------------------------------------pipes-------------------------------
	for (int i = 0; i < sapo[1].numClientes; i++) {
		_tprintf(TEXT("[ESCRITOR] Desligar o pipe (DisconnectNamedPipe)\n"));

		//desliga a instancia ao named pipe
		if (!DisconnectNamedPipe(sapo[1].hPipe[i])) {
			_tprintf(TEXT("[ERRO] Desligar o pipe! (DisconnectNamedPipe)"));
			exit(-1);
		}
	}
	//----------------------------------------.....-------------------------------
	//CloseHandles ... mas � feito automaticamente quando o processo termina


	return 0;
}