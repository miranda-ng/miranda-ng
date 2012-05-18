#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH 260
#define DEFAULT_OUTPUT "output.inc"

void PrintUsage(char *programPath)
{
	printf("Usage\n");
	printf("%s input.doc [output.inc]\n", programPath);
	printf("\nConverts a rtf text found in input.doc to a string that contains the rtf text and stores it in output.inc");
}

void Add(char *result, char *what)
{
	strcat(result, what);
}

void Add(char *result, char chr)
{
	int len = strlen(result);
	result[len++] = chr;
	result[len] = '\0';
}

void Convert(char *input, char *output)
{
	int len = strlen(input);
	int i;
	output[0] = '\0';
	Add(output, '\"');
	for (i = 0; i < len; i++)
		{
			switch (input[i])
				{
					case '\"':
						Add(output, "\"\"");
						break;
					case '\\':
						Add(output, "\\\\");
						break;
					case '\n':
						Add(output, "\\n");
						break;
					default:
						Add(output, input[i]);
				}
		}
	Add(output, "\"\n");
}

void DoConversion(char *inFile, char *outFile)
{
	FILE *fin = fopen(inFile, "rt");
	FILE *fout = fopen(outFile, "wt");
	char buffer[2048];
	char out[4096];
	if ((fin) && (fout))
		{
			while (!feof(fin))
				{
					fgets(buffer, sizeof(buffer), fin);
					if (strlen(buffer) > 0)
						{
							Convert(buffer, out);
							fputs(out, fout);
						}
				}
		}
	if (fin)
		{
			fclose(fin);
		}
	if (fout)
		{
			fclose(fout);
		}
}

int main(int argc, char *argv[])
{
	char input[MAX_PATH];
	char output[MAX_PATH];
	if ((argc < 2) || (argc > 3))
		{
			PrintUsage(argv[0]);
			return 0;
		}
	strcpy(input, argv[1]);
	if (argc == 3)
		{
			strcpy(output, argv[2]);
		}
		else{
			strcpy(output, DEFAULT_OUTPUT);
		}
	DoConversion(input, output);
	return 0;
}

