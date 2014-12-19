#ifndef M_ACTMAN
#define M_ACTMAN

#define ACCF_DISABLED 0x10000000 // action disabled
#define ACCF_EXPORT   0x08000000 // action to export
#define ACCF_VOLATILE 0x04000000 // don't save in DB
#define ACCF_IMPORTED ACF_EXPORT
#define ACCF_FLAGS    (ACCF_DISABLED | ACCF_EXPORT | ACCF_IMPORTED | ACCF_VOLATILE)
#define ACCF_ID       0x02000000 // for MS_ACT_SELECT, lParam is ID (else name)
#define ACCF_CLEAR    0x01000000 // clear other flags, else - set


typedef struct{
	WCHAR* Descr;
	DWORD  ID;
	DWORD  flags; // ACCF_* flags
	} TChain, *PChain;

//	Service to get list of all configured actions;
//	wParam : 0
//	lParam : address of destination list variable (address of pointer to TChain)
//	         Notes: first 4 bytes of list = size of TChain structure (to add new fields in future)
//	Return value: count of elements;
#define MS_ACT_GETLIST "Actions/GetList"

//	Service to free list of all configured actions got with MS_ACT_GETLIST service call;
//	wParam : 0
//	lParam : list address (pointer to ACTION returned by MS_ACT_GETLIST)
#define MS_ACT_FREELIST "Actions/FreeList"

//	Service to call action defined in wParam;
//	wParam: ID of an action (see ACTION.ActID) when calling MS_ACT_RUN
//		or description of an action (see ACTION.ActDescr) when calling MS_ACT_RUNGROUP
//	lParam: parameter (will be passed to action called)
#define MS_ACT_RUNBYID   "Actions/RunById"
#define MS_ACT_RUNBYNAME "Actions/RunByName"

//	Event: action group list was changed: something was added or deleted
//	wParam: set of ACTM_* flags
//	lParam : 0
#define ME_ACT_CHANGED "Actions/Changed"

//	Starts action with 2 parameters
//	wParam: 0
//	lParam: pointer to TAct_Param

#define MS_ACT_RUNPARAMS "Actions/RunWithParams"
typedef struct TAct_Param
	{
    DWORD flags; // 0 - ID, 1 - Name
    DWORD ID; // Id or name
    WPARAM wParam;
    LPARAM lParam;
	} TAct_Param, *PAct_Param;

#define ACTM_NEW      0x00000001
#define ACTM_DELETE   0x00000002
#define ACTM_RELOAD   0x00000004
#define ACTM_RENAME   0x00000008
#define ACTM_SORT     0x00000010
#define ACTM_ACT      0x10000000 // do not check, internal
#define ACTM_ACTS     0x20000000 // do not check, internal
#define ACTM_LOADED   0x80000000


#define ACIO_EXPORT   0x00000001 // export, else - import
#define ACIO_APPEND   0x00000002 // append file on export
#define ACIO_ASKEXIST 0x00000004 // ask, if action exists on import
#define ACIO_SELECTED 0x00000008 // export selected actions only

//	wParam: ACIO_* flags
//	lParam: Unicode file name
//	Return - true, if totally succesful
#define MS_ACT_INOUT "Actions/ImpExp"


//Event: Export actions
//		   wParam - ACIO_* flags
//		   lParam - unicode filename
#define ME_ACT_INOUT "Actions/InOut"


//      Select/unselect specified action
//      wParam: set of ACCF_* consts
//      lParam: unicode action name / number
//      Return - -1 if unsuccesful
#define MS_ACT_SELECT "Actions/Select"


//    Event: Action started/finished
//    wParam - Action status: 0 - started, 1 - finished
//    lParam - action id

#define ME_ACT_ACTION "Actions/Action"

#endif
