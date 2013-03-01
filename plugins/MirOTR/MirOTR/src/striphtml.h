#pragma once

struct STRIPHTML_DATA {
	std::stack<char*> stack;
	std::string buffer;
};

char * striphtml(char *html);