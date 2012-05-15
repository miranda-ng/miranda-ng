extern HINSTANCE hInst;
extern NudgeElementList* NudgeList;
extern int nProtocol;
extern CNudgeElement DefaultNudge;
extern CShake shake;
extern CNudge GlobalNudge;

static INT_PTR CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcNudgeOpt(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK DlgProcShakeOpt(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

void UpdateControls(HWND hwnd);
int GetSelProto(HWND hwnd, HTREEITEM hItem);
void CheckChange(HWND hwnd, HTREEITEM hItem);
