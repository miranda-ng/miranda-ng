#pragma once

#include "stack"
#include "string"

struct STRIPHTML_DATA {
	std::stack<char*> stack;
	std::string buffer;
};

char * striphtml(char *html);