#ifndef __DECODE_H
#define __DECODE_H

#define DOTLINE(s)	((((s)[-2]=='\r') || ((s)[-2]=='\n')) && ((s)[-1]=='.') && (((s)[0]=='\r') || ((s)[0]=='\n') || ((s)[0]=='\0')))	// be careful, it's different to ESR's pop3.c ;-)
#define ENDLINE(s)	(((s)[0]=='\r') || ((s)[0]=='\n'))	//endline
#define WS(s)		(((s)[0]==' ') || ((s)[0]=='\t'))	//whitespace
#define ENDLINEWS(s)	((((s)[0]=='\r') || ((s)[0]=='\n')) && (((((s)[1]=='\r') || ((s)[1]=='\n')) && (((s)[2]==' ') || ((s)[2]=='\t'))) || (((s)[1]==' ') || ((s)[1]=='\t'))))	//endline+whitespace: enters(CR or LF and their combinations) followed by space or tab
#define EOS(s)		((s)[0]==0)				//end of string (stream)

#define CODES(s)	((s[0]=='=') && (s[1]=='?'))			//start of coded string
#define CODEE(s)	((s[0]=='?') && (s[1]=='='))			//end of coded string
#define CODED(s)	(s[0]=='?')					//code delimiter

#define MIME_PLAIN	1
#define	MIME_MAIL	2

struct cptable
{
	char *name;
	unsigned int ID;
};

#endif
