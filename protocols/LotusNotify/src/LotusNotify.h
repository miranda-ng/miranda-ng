#pragma once

struct HISTORIA
{
	uint32_t noteID;
	BOOL clicked;
	BOOL again;
	struct HISTORIA *next;
	struct POPUPSQUEUE *pq;
};

struct POPUPSQUEUE
{
	HWND hWnd;
	struct POPUPSQUEUE *next;
};

// structure contines only LN msg id to send to popup 

typedef struct {
	uint32_t id;
	char strNote[4*16]; 
} POPUPATT;

__declspec(dllexport) STATUS LNPUBLIC MainEntryPoint (void);

void ErMsgT(wchar_t* msg);

void checkthread(void*);

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};
