#include <windows.h>
#include <CommCtrl.h>
#include <stdio.h>
#include <malloc.h>

#include <newpluginapi.h>
#include <m_clistint.h>
#include <m_cluiframes.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_findadd.h>
#include <m_fingerprint.h>
#include <m_fontservice.h>
#include <m_hotkeys.h>
#include <m_genmenu.h>
#include <m_gui.h>
#include <m_langpack.h>
#include <m_metacontacts.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_timezones.h>
#include <m_tipper.h>
#include <m_toptoolbar.h>
#include <m_utils.h>
#include <m_variables.h>
#include <m_xstatus.h>

#include "resource.h"
#include "version.h"

#define MODULENAME     "QuickSearch"
#define QS_SHOWSERVICE "QuickSearch/Show"

#define StatusSort       1000

#define QSTS_BYTE        0
#define QSTS_WORD        1
#define QSTS_DWORD       2
#define QSTS_STRING      3
#define QSTS_IP          4
#define QSTS_TIMESTAMP   5
#define QSTS_SIGNED      6
#define QSTS_HEXNUM      7

// must be non-zero for empty-column checking
#define QST_SETTING      100
#define QST_SCRIPT       1
#define QST_SERVICE      2
#define QST_CONTACTINFO  3
#define QST_OTHER        200

#define QSTO_LASTSEEN    0
#define QSTO_LASTEVENT   1
#define QSTO_METACONTACT 2
#define QSTO_EVENTCOUNT  3
#define QSTO_DISPLAYNAME 4
#define QSTO_ACCOUNT     5

#define QSO_SORTBYSTATUS 0x00000001 // Sort by status
#define QSO_DRAWGRID     0x00000002 // Draw listview grid
#define QSO_TOOLSTYLE    0x00000004 // QS window tool style
#define QSO_SAVEPATTERN  0x00000008 // Save filter pattern
#define QSO_AUTOCLOSE    0x00000010 // Close QS window after action
#define QSO_CLIENTICONS  0x00000020 // Show client icons (fingerprint)
#define QSO_MAINOPTIONS  0x0000FFFF // mask for common options

// QS window options
#define QSO_STAYONTOP    0x00010000 // Stay QS window on top
#define QSO_SHOWOFFLINE  0x00020000 // Show offline contacts
#define QSO_COLORIZE     0x00040000 // Colorize lines
#define QSO_SORTASC      0x00080000 // Sort column ascending

struct ServiceDescr
{
	char *service;
	LPARAM wParam, lParam;
	uint32_t wFlags, lFlags, flags;
};

struct ColumnItem : public MZeroedObject
{
	ColumnItem(const wchar_t *pszTitle, int _width = 64, int _setting_type = QST_SETTING);
	ColumnItem(const ColumnItem &src);
	~ColumnItem();

	int  HasImage(const wchar_t *pwsztext) const;
	void SetSpecialColumns();

	wchar_t *title;
	uint16_t setting_type = 0; // QST_* constants
	uint16_t width = 0;

	union
	{
		uint32_t dwFlags = 0;
		struct {
			bool bEnabled : 1;
			bool bInit : 1;
			bool bFilter : 1;
			bool isXstatus : 1;
			bool isGender : 1;
			bool isAccount : 1;
			bool isClient : 1;
			bool isGroup : 1;
			bool isContainer : 1;
		};
	};

	union {
		// 0: db setting
		struct {
			uint16_t datatype;   // QSTS_* constants
			char *module;
			char *setting;
		};

		// 1: script
		wchar_t *script;

		// 2: service
		ServiceDescr svc;

		// 3: contact info
		int cnftype;            // CNF_* constants

		// 4: other
		int other;              // QSTO_* constants
	};
};

struct ContactIntoColumn : public ColumnItem
{
	ContactIntoColumn(const wchar_t *pwszTitle, int width, int _type) :
		ColumnItem(pwszTitle, width, QST_CONTACTINFO)
	{
		cnftype = _type;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

#define so_mbottom         "mbottom"
#define so_mright          "mright"
#define so_mtop            "mtop"
#define so_mleft           "mleft"

#define so_columnsort      "columnsort"
#define so_flags           "flags"

#define so_numcolumns      "numcolumns"
#define so_item            "item"

#define so_title           "title"
#define so_width           "width"
#define so_flags           "flags"
#define so_setting_type    "setting_type"

#define so_cnftype         "cnftype"

#define so_datatype        "datatype"
#define so_module          "module"
#define so_setting         "setting"

#define so_script          "script"

#define so_service         "service"
#define so_restype         "flags"
#define so_wparam          "wparam"
#define so_lparam          "lparam"

#define so_other           "other"

/////////////////////////////////////////////////////////////////////////////////////////

enum ColorCode
{
	bkg_norm, fgr_norm,
	bkg_odd,  fgr_odd,
	bkg_dis,  fgr_dis,
	bkg_del,  fgr_del,
	bkg_hid,  fgr_hid,
	bkg_meta, fgr_meta,
	bkg_sub,  fgr_sub,
	color_max
};

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	COLORREF m_colors[color_max];
	RECT m_rect;
	OBJLIST<ColumnItem> m_columns;
	int m_sortOrder;
	uint32_t m_flags; // QSO_* constants

	void LoadOptWnd();
	void SaveOptWnd();

	int  LoadColumns(OBJLIST<ColumnItem>&);
	void LoadColumn(int n, ColumnItem &col);
	void LoadParamValue(char *buf, uint32_t &flags, LPARAM &value);

	void SaveOptions();
	void SaveColumn(int n, const ColumnItem &col);
	void SaveParamValue(char *buf, uint32_t flags, LPARAM value);

	int Load() override;
	int Unload() override;
};

extern bool g_bVarsInstalled, g_bTipperInstalled, g_bFingerInstalled;

/////////////////////////////////////////////////////////////////////////////////////////
// utils.cpp

int  ColumnToListView(int col);
int  ListViewToColumn(int col);
void LoadDefaultColumns(OBJLIST<ColumnItem> &dst);

void SnapToScreen(RECT &rc);

const wchar_t* cnf2str(int);

wchar_t* BuildLastSeenTime(uint32_t timestamp);
uint32_t    BuildLastSeenTimeInt(MCONTACT hContact, const char *szModule);
wchar_t* TimeToStrW(uint32_t timestamp);

/////////////////////////////////////////////////////////////////////////////////////////
// windows.cpp

struct QSFlags
{
	union {
		uint32_t flags = 0;
		struct {
			bool bInList : 1;  // in constant list
			bool bActive : 1;  // contact in listview
			bool bDeleted : 1; // contact deleted
			bool bPattern : 1; // pattern check passed
			bool bAccDel : 1;  // account deleted
			bool bAccOff : 1;  // account disabled
			bool bIsMeta : 1;  // contact is metacontact
			bool bIsSub : 1;   // contact is part of metacontact
		};
	};
};

struct CRowItem : public QSFlags
{
	CRowItem(MCONTACT _1, class QSMainDlg *pDlg);
	~CRowItem();

	MCONTACT hContact;
	const char *szProto;

	int status = ID_STATUS_OFFLINE;
	WPARAM wparam = 0;
	LPARAM lparam = 0;

	void GetCellColor(int idx, COLORREF &clrBack, COLORREF &clrText);

	struct Val
	{
		wchar_t *text = nullptr;
		UINT_PTR data = UINT_PTR(-1);

		wchar_t* getText() const { return (text) ? text : L""; }
		void LoadOneItem(MCONTACT hContact, const ColumnItem &pCol, class QSMainDlg *pDlg);
	}
	*pValues;
};

// status bar data
struct CStatusBarItem : public QSFlags
{
	CStatusBarItem(const char *_proto, int _flags) :
		szProto(_proto)
	{
		flags = _flags;
	}

	const char *szProto;

	int total = 0;
	int found = 0;
	int online = 0;
	int liston = 0;
};

#define WM_CONTACT_ADDED   (WM_USER+2)
#define WM_CONTACT_DELETED (WM_USER+3)
#define WM_STATUS_CHANGED  (WM_USER+4)

class QSMainDlg : public CDlgBase
{
	friend struct CRowItem;

	OBJLIST<CRowItem> m_rows;
	OBJLIST<CStatusBarItem> m_sbdata;

	UI_MESSAGE_MAP(QSMainDlg, CDlgBase);
		UI_MESSAGE(WM_SYSCOMMAND, OnSysCommand);
		UI_MESSAGE(WM_MOUSEMOVE, OnMouseMove);
		UI_MESSAGE(WM_KEYDOWN, OnKeydown);
		UI_MESSAGE(WM_CONTACT_ADDED, OnContactAdded);
		UI_MESSAGE(WM_CONTACT_DELETED, OnContactDeleted);
		UI_MESSAGE(WM_STATUS_CHANGED, OnStatusChanged);
	UI_MESSAGE_MAP_END();

	// internal methods
	void      AddColumn(int idx, ColumnItem *pCol);
	void      AddContactToList(MCONTACT hContact, CRowItem *pRow);
	void      AdvancedFilter();
	void      ChangeStatusPicture(CRowItem *pRow, MCONTACT hContact, LPARAM lParam);
	void      ConvertToMeta();
	void      CopyMultiLines();
	void      DeleteByList();
	void      DeleteOneContact(MCONTACT hContact);
	wchar_t*  DoMeta(MCONTACT hContact);
	void      FillGrid();
	void      FillProtoCombo();
	int       FindItem(CRowItem *pRow);
	int       FindMeta(MCONTACT hMeta, WPARAM &metaNum);
	CRowItem* FindRow(MCONTACT hContact);
	MCONTACT  GetFocusedContact();
	int       GetLVSubItem(int x, int y);
	void      MoveToContainer(const wchar_t *pwszName);
	void      MoveToGroup(const wchar_t *pwszName);
	void      PrepareTable(bool bReset = false);
	void      ProcessLine(CRowItem *pRow, bool test = true);
	void      SaveColumnOrder();
	void      ShowContactMenu(MCONTACT, int);
	void      ShowContactMsgDlg(MCONTACT hContact);
	void      ShowMultiPopup(int);
	void      Sort();
	void      UpdateLVCell(int item, int column, const wchar_t *pwszText = nullptr);

	__forceinline CRowItem* GetRow(int i) {
		return (CRowItem *)m_grid.GetItemData(i);
	}

	// status bar
	HWND hwndStatusBar = 0, HintWnd = 0;
	void DrawSB();
	void UpdateSB();

	// internal data
	int hLastMeta;
	bool TTShowed = false;
	int tableColumns = 0;
	HICON hIconF = 0, hIconM = 0;
	HANDLE hAdd, hDelete, hChange;
	HGENMENU mnuhandle = 0;

	// patterns
	struct Pattern {
		Pattern(const wchar_t *pwszText) :
			str(pwszText)
		{}

		const wchar_t *str;
		bool res = false;
	};

	OBJLIST<Pattern> m_patterns;
	ptrW m_wszPatternBuf;

	bool bShowOffline;
	char *szFilterProto = nullptr;

	bool CheckPattern(CRowItem *pRow);
	void MakePattern(const wchar_t *pwszPattern);

	// controls
	CTimer m_hover;
	CCtrlEdit edtFilter;
	CCtrlCheck chkShowOffline, chkColorize;
	CCtrlCombo cmbProto;
	CCtrlButton btnRefresh;
	CCtrlListView m_grid;

public:
	QSMainDlg(const wchar_t *pattern);

	bool OnInitDialog() override;
	void OnDestroy() override;
	int  Resizer(UTILRESIZECONTROL *urc) override;

	void ChangeCellValue(MCONTACT hContact, int col);
	bool PrepareToFill();
	void ToggleColumn(int col);

	INT_PTR OnSysCommand(UINT, WPARAM, LPARAM);
	INT_PTR OnMouseMove(UINT, WPARAM, LPARAM);
	INT_PTR OnKeydown(UINT, WPARAM, LPARAM);
	INT_PTR OnContactAdded(UINT, WPARAM, LPARAM);
	INT_PTR OnContactDeleted(UINT, WPARAM, LPARAM);
	INT_PTR OnStatusChanged(UINT, WPARAM, LPARAM);

	INT_PTR NewEditProc(UINT, WPARAM, LPARAM);
	INT_PTR NewLVProc(UINT, WPARAM, LPARAM);

	void onBuildMenu_Grid(CContextMenuPos *pos);
	void onColumnClick_Grid(CCtrlListView::TEventInfo *ev);
	void onCustomDraw_Grid(CCtrlListView::TEventInfo *ev);
	void onDblClick_Grid(CCtrlListView::TEventInfo *ev);

	void onSelChange_Proto(CCtrlCombo *);
	
	void onChange_Filter(CCtrlEdit *);
	void onChange_ShowOffline(CCtrlCheck *);
	void onChange_Colorize(CCtrlCheck *);
	
	void onClick_Refresh(CCtrlButton *);

	void onTimer_Hover(CTimer *);
};

int CloseSrWindow(bool = true);
int OpenSrWindow(const wchar_t *pwszPattern);
