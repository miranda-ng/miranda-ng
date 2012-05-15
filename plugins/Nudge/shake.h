#ifndef SHAKE_H
#define SHAKE_H

class CShake
{
public:
	bool Shaking;
	bool ShakingChat;
	int nScaleClist;
	int nScaleChat;
	int nMoveClist;
	int nMoveChat;

	void Load(void);
	void Save(void);
	int ShakeClist(HWND hWnd);
	int ShakeChat(HWND hWnd);
};

INT_PTR ShakeClist(WPARAM,LPARAM);
INT_PTR ShakeChat(WPARAM,LPARAM);
/*
int TriggerShakeChat(WPARAM,LPARAM);
int TriggerShakeClist(WPARAM,LPARAM);
int TriggerAction(WPARAM,LPARAM);*/

#endif