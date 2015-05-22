#include <windows.h>
#include <winsock.h>

#include <iostream>
#include <string>

void implementation(int argc, char **argv, bool console);

int main(int argc, char **argv)
try
{
	if (argc == 1 || argc == 2 && !mir_strcmp("/?", argv[1])) {
		MessageBox(NULL, "Syntax: sendlog32 [-T] [-H host] [-P port] text\n"
			"\tSends single text message.", "Sendlog32", MB_OK);
		return 0;
	}

	implementation(argc, argv, false);
}
catch (DWORD err) {
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, "sendlog32", MB_OK);

	LocalFree(lpMsgBuf);

	return 1;
}
