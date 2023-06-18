#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

#include "..\header.h"


//-----------------------------------------------------------------------------------
#define PIPE_READ TEXT("\\\\.\\pipe\\SER-SAP")
#define PIPE_WRITE TEXT("\\\\.\\pipe\\SAP-SER")

typedef struct {
	HANDLE hPipe;
	HANDLE hMutex; //para controlar o numClientes
	int numClientes;
	int terminar;
}ThreadDados;

GameInfo game;
//-----------------------------------------------------------------------------------

//------------------------------------------threads----------------------------------------------------
DWORD WINAPI ThreadEscrevePipe(LPVOID param) {
	ThreadDados* dados = (ThreadDados*)param;
	TCHAR buf[256];
	DWORD n;

	//aqui , o servidor já recebeu um cliente
	do {
		//vai buscar informação à consola
		_tprintf(TEXT("[SAPO] Frase: "));
		_fgetts(buf, 256, stdin);
		buf[_tcslen(buf) - 1] = '\0';
		//_tprintf(TEXT("kkkkkkkkkkkkkkkkkk"));
		//bloqueamos aqui porque é uma regiao critica
		WaitForSingleObject(dados->hMutex, INFINITE);

		//escreve no named pipe

		if (!WriteFile(dados->hPipe, buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
			_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
			exit(-1);
		}

		_tprintf(TEXT("[ESCRITOR] Enviei %d bytes ao servidor... (WriteFile)\n"), n);
		//libertamos o mutex
		ReleaseMutex(dados->hMutex);


	} while (_tcscmp(buf, TEXT("fim")));

	dados->terminar = 1;

	return 0;
}

DWORD WINAPI ThreadLePipe(LPVOID param) {

	ThreadDados* dados = (ThreadDados*)param;
	TCHAR buf[256];
	DWORD n;
	BOOL ret;

	while (dados->terminar != 1) {
		//bloqueamos aqui porque é uma regiao critica
		WaitForSingleObject(dados->hMutex, INFINITE);
		//le as mensagens enviadas pelo servidor
		ret = ReadFile(dados->hPipe, &game, sizeof(GameInfo), &n, NULL);

		//termina corretamente a string
		//buf[n / sizeof(TCHAR)] = '\0';

		if (!ret || !n) {
			_tprintf(TEXT("[LEITOR] %d %d... (ReadFile)\n"), ret, n);
			break;
		}

		//_tprintf(TEXT("[LEITOR] Recebi %d bytes: '%s'... (ReadFile)\n"), n, buf);
		// Libera a permissão de escrita
		ReleaseMutex(dados->hMutex);
	}

	CloseHandle(dados->hPipe);
	return;
}
//----------------------------------------------------------------------------------------------------

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

TCHAR szProgName[] = TEXT("Base");

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow) {


	ThreadDados dados[2];
	HANDLE hThread[2];
	dados[0].terminar = 0;
	dados[1].terminar = 0;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	dados[0].hMutex = CreateMutex(NULL, FALSE, NULL); //Criação do mutex

	if (dados[0].hMutex == NULL) {
		_tprintf(TEXT("[Erro] ao criar mutex!\n"));
		return -1;
	}

	//espera que exista um named pipe para ler do mesmo
	//bloqueia aqui
	if (!WaitNamedPipe(PIPE_READ, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), PIPE_READ);
		exit(-1);
	}

	dados[1].hMutex = CreateMutex(NULL, FALSE, NULL); //Criação do mutex

	if (dados[1].hMutex == NULL) {
		_tprintf(TEXT("[Erro] ao criar mutex!\n"));
		return -1;
	}

	//espera que exista um named pipe para ler do mesmo
	//bloqueia aqui
	if (!WaitNamedPipe(PIPE_WRITE, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), PIPE_WRITE);
		exit(-1);
	}

	//_tprintf(TEXT("[LEITOR] Ligação ao pipe do escritor... (CreateFile)\n"));

	//ligamo-nos ao named pipe que ja existe nesta altura
	//1º nome do named pipe, 2ºpermissoes (têm de ser iguais ao CreateNamedPipe do servidor), 3ºshared mode 0 aqui,
	//4º security atributes, 5ºflags de criação OPEN_EXISTING, 6º o default é FILE_ATTRIBUTE_NORMAL e o 7º é o template é NULL
	dados[0].hPipe = CreateFile(PIPE_READ, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (dados[0].hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_READ);
		exit(-1);
	}

	//_tprintf(TEXT("[LEITOR] Liguei-me...\n"));

	dados[1].hPipe = CreateFile(PIPE_WRITE, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (dados[1].hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_WRITE);
		exit(-1);
	}

	//_tprintf(TEXT("[LEITOR] Liguei-me...\n"));


	hThread[0] = CreateThread(NULL, 0, ThreadLePipe, &dados[0], 0, NULL);
	if (hThread[0] == NULL) {
		_tprintf(TEXT("[Erro] ao criar thread!\n"));
		return -1;
	}

	hThread[1] = CreateThread(NULL, 0, ThreadEscrevePipe, &dados[1], 0, NULL);
	if (hThread[1] == NULL) {
		_tprintf(TEXT("[Erro] ao criar thread!\n"));
		return -1;
	}

	//valores para testar bitmaps
	/*game.nFaixas = 6;
	game.faixa[2].col[6].val = 3;
	game.faixa[3].col[3].val = 2;
	game.faixa[4].col[5].val = 2;
	game.faixa[4].col[4].val = 2;
	game.faixa[2].col[1].val = 2;
	game.faixa[5].col[3].val = 1;
	*/
	HANDLE hThreadMsg;

	int x, y;
	DWORD n;
	char c;

	HWND hWnd;
	MSG lpMsg;
	WNDCLASSEX wcApp;

	// ============================================================================
	// 1. Definição das características da janela "wcApp" 
	//    (Valores dos elementos da estrutura "wcApp" do tipo WNDCLASSEX)
	// ============================================================================
	wcApp.cbSize = sizeof(WNDCLASSEX);
	wcApp.hInstance = hInst;
	wcApp.lpszClassName = szProgName;
	wcApp.lpfnWndProc = TrataEventos;
	wcApp.style = CS_HREDRAW | CS_VREDRAW;
	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcApp.hIconSm = LoadIcon(NULL, IDI_INFORMATION);
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcApp.lpszMenuName = NULL;
	wcApp.cbClsExtra = 0;
	wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcApp.cbWndExtra = sizeof(GameInfo*);

	if (!RegisterClassEx(&wcApp))
		return(0);

	hWnd = CreateWindow(szProgName, TEXT("Sapo"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, (HWND)HWND_DESKTOP, (HMENU)NULL, (HINSTANCE)hInst, 0);

	game.hWnd = hWnd;	//colocar o handle da janela na estrutura
	LONG_PTR res = SetWindowLongPtr(hWnd, 0, (LONG_PTR)&game);

	//Mostrar a janela
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//loop de mensagens
	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}



	return((int)lpMsg.wParam);
}

typedef struct {
	TCHAR c;
	int xPos, yPos;
} PosChar;

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {

	static GameInfo* pont;
	static HANDLE hMutexG;
	static TCHAR key = '?';
	int xPos, yPos;
	HDC hdc;
	RECT rect;
	PAINTSTRUCT ps;
	int aux = 1;
	int x, y;

	static PosChar posicoes[100];
	static int totalPos = 0;

	hMutexG = CreateMutex(NULL, FALSE, NULL);
	//pont->hMutexG = hMutexG; //não sei pq é que isto não compila aqui, este mutex é para ler do teclado no WN_CHAR

	HBITMAP hFrog, hCarLR, hCarRL, hBlank, hBlock;
	static BITMAP bmp;
	static HDC bmpFrog, bmpCarLR, bmpCarRL, bmpBlank, bmpBlock;
	static int xBitmap;
	static int yBitmap;
	DWORD n;

	switch (messg) {

	case WM_CREATE:


		break;

	case WM_CLOSE:
		if (MessageBox(hWnd, _T("Sair?"), _T("Deseja mesmo sair?"), MB_ICONQUESTION | MB_YESNO | MB_HELP) == IDYES) {
			DestroyWindow(hWnd);
		}
		break;

	case WM_HELP:
		MessageBox(hWnd, _T("O objetivo do jogo e chegar ao outro lado da estrada evitando carros e obstaculos\nBoa sorte!!"), _T("Sair"), MB_OK);
		break;


	case WM_KEYDOWN:				//Apanhar evento do teclado
		key = wParam;

		WaitForSingleObject(hMutexG, INFINITE);

		if (key == VK_UP) {
			pont->faixa[4].col[3].val = 1;
			pont->faixa[5].col[3].val = 0;
			//UpdateWindow(hWnd);
		}

		else if (key == VK_DOWN) {
			pont->faixa[4].col[3].val = 1;
			pont->faixa[5].col[3].val = 0;
		}

		else if (key == VK_LEFT) {
			//mandar pelo pipe 3, servidor move o sapo para esquerda
			//receber a mensagem de volta do pipe
		}

		else if (key == VK_RIGHT) {
			//mandar pelo pipe 4, servidor move o sapo para direita
			//receber a mensagem de volta do pipe
		}

		ReleaseMutex(hMutexG);

		InvalidateRect(hWnd, NULL, TRUE);	//chamada ao WM_PAINT

		break;

	case WM_PAINT:
	{
		pont = (GameInfo*)GetWindowLongPtr(hWnd, 0);

		hFrog = (HBITMAP)LoadImage(NULL, TEXT("Frog.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE);
		GetObject(hFrog, sizeof(bmp), &bmp);

		hCarLR = (HBITMAP)LoadImage(NULL, TEXT("carrolr.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE);
		GetObject(hCarLR, sizeof(bmp), &bmp);

		hCarRL = (HBITMAP)LoadImage(NULL, TEXT("carrorl.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE);
		GetObject(hCarRL, sizeof(bmp), &bmp);

		hBlank = (HBITMAP)LoadImage(NULL, TEXT("blnk.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE);
		GetObject(hBlank, sizeof(bmp), &bmp);

		hBlock = (HBITMAP)LoadImage(NULL, TEXT("block.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE);
		GetObject(hBlock, sizeof(bmp), &bmp);

		hdc = GetDC(hWnd);

		bmpFrog = CreateCompatibleDC(hdc);
		bmpCarLR = CreateCompatibleDC(hdc);
		bmpCarRL = CreateCompatibleDC(hdc);
		bmpBlank = CreateCompatibleDC(hdc);
		bmpBlock = CreateCompatibleDC(hdc);

		SelectObject(bmpFrog, hFrog);
		SelectObject(bmpCarLR, hCarLR);
		SelectObject(bmpCarRL, hCarRL);
		SelectObject(bmpBlank, hBlank);
		SelectObject(bmpBlock, hBlock);

		ReleaseDC(hWnd, hdc);
		GetClientRect(hWnd, &rect);

		xBitmap = 20;
		yBitmap = 20;

		hdc = BeginPaint(hWnd, &ps);
		hdc = GetDC(hWnd);
		GetClientRect(hWnd, &rect);
		SetTextColor(hdc, RGB(0, 0, 0));
		SetBkMode(hdc, TRANSPARENT);

		for (int i = 0; i < pont->nFaixas; i++) {
			for (int j = 0; j < COLUNAS; j++) {
				if (pont->faixa[i].col[j].val == 1) {
					BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpFrog, 0, 0, SRCCOPY);
					xBitmap += 20;
				}
				else if (pont->faixa[i].col[j].val == 2) {
					BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpCarRL, 0, 0, SRCCOPY);
					xBitmap += 20;
				}
				else if (pont->faixa[i].col[j].val == 3) {
					BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpBlock, 0, 0, SRCCOPY);
					xBitmap += 20;
				}

				else {
					BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpBlank, 0, 0, SRCCOPY);
					xBitmap += 20;
				}
			}
			yBitmap += 20;
			xBitmap = 20;

			/*for (int i = 0; i < totalPos; i++) {
				rect.left = posicoes[i].xPos;
				rect.top = posicoes[i].yPos;
			}
			EndPaint(hdc, &ps);*/

		}
		break;
	}

	case WM_DESTROY:			// Destruir a janela e terminar o programa
		PostQuitMessage(0);
		break;
	default:
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;
	}
	return(0);
}