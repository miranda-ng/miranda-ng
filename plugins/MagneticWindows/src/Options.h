
#define cDefaultSnapWidth 12

typedef
	struct TOptions {
		bool DoSnap;
		int SnapWidth;
		bool ScriverWorkAround;
} TOptions;

extern TOptions Options;

INT_PTR CALLBACK OptionsDlgProc(HWND, UINT, WPARAM, LPARAM);

int InitOptions(WPARAM, LPARAM);
void LoadOptions();