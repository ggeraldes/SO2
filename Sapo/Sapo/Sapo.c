#include "../../header.h"


#define PIPE_NAME TEXT("\\\\.\\pipe\\teste")
//#define PIPE_NAME TEXT("\\\\127.0.0.1\\pipe\\teste")
//#define PIPE_NAME TEXT("\\\\localhost\\pipe\\teste")
//#define PIPE_NAME TEXT("\\\\.\\10.65.134.87\\teste")
//#define PIPE_NAME TEXT("\\\\DESKTOP-HVTC9K7\\pipe\\teste")

int _tmain(int argc, LPTSTR argv[]) {
	

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	

	return 0;
}