#include "../../header.h"


int _tmain(int argc, LPTSTR argv[]) {
	
	TCHAR COM[50]; //COMANDOS
	int time, faixa;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	do {

		_tprintf(TEXT("COMANDO: "));
		_fgetts(COM, 50, stdin);
		COM[_tcslen(COM) - 1] = '\0';

		if (_tcscmp(COM, TEXT("stop")) == 0) {

			_tprintf(TEXT("TEMPO: "));
			_tcscanf_s(TEXT("%d"), &time);

		}
		else if (_tcscmp(COM, TEXT("invert")) == 0) {

			_tprintf(TEXT("FAIXA: "));
			_tcscanf_s(TEXT("%d"), &faixa);

		}
		else if (_tcscmp(COM, TEXT("insert")) == 0) {

			_tprintf(TEXT("FAIXA: "));
			_tcscanf_s(TEXT("%d"), &faixa);

		}

	} while (_tcscmp(COM, TEXT("close")));
	
	return 0;
}
