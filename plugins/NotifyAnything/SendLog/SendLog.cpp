#include <windows.h>
#include <winsock.h>

#include <iostream>
#include <string>

void implementation(int argc, char **argv, bool console);

int main(int argc, char **argv)
try
{
	if (argc == 2 && !strcmp("/?", argv[1])) {
		std::cout << "\n"
			"Syntax: sendlog [-T] [-H host] [-P port] [-p prefix]\n"
			"\tSends each input line as a separate message, with optional prefix.\n"
			"\n"
			"Syntax: sendlog text\n"
			"\tSends single text message.\n"
			"\n"
			<< std::flush;
		return 0;
	}

	implementation(argc, argv, true);
}
catch (DWORD err) {
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);

	std::cerr << (LPCTSTR)lpMsgBuf << std::endl;
	if (!std::cerr)
		MessageBox(NULL, (LPCTSTR)lpMsgBuf, "sendlog", MB_OK);

	LocalFree(lpMsgBuf);

	return 1;
}
