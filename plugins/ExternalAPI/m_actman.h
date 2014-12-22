#ifndef M_ACTMAN
#define M_ACTMAN

#define AutoStartName "#Autostart"

//----- Macro executing services -----
  
//	Service to call action defined in wParam;
//	wParam: ID of an action (see ACTION.ActID) when calling MS_ACT_RUN
//		or description of an action (see ACTION.ActDescr) when calling MS_ACT_RUNGROUP
//	lParam: parameter (will be passed to action called)
#define MS_ACT_RUNBYID   "Actions/RunById"
#define MS_ACT_RUNBYNAME "Actions/RunByName"

//	Starts action with 2 parameters
//	wParam: 0
//	lParam: pointer to TAct_Param

#define MS_ACT_RUNPARAMS "Actions/RunWithParams"

#define ACTP_BYNAME      1 // id points on unicode name
#define ACTP_WAIT        2 // waiting for macro finish
#define ACTP_NOTIFY      4 // notify (raise event) for start/finish macro
#define ACTP_SAMETHREAD  8 // execute macro in same thread (with finish waiting)
#define ACTP_KEEPRESULT 16 // (internal) keep last result

typedef struct TAct_Param
	{
    uint_ptr id;     // Id or name
    WPARAM   wParam;
    LPARAM   lParam;
    DWORD    flags;  // ACTP_*
    DWORD    lPType; // last result (in lParam) type
	} TAct_Param, *PAct_Param;


//----- Macro list operations -----

#define ACCF_EXPORT   0x08000000 // action to export
#define ACCF_IMPORT ACF_EXPORT

typedef struct{
	WCHAR* Descr;
	DWORD  ID;
	DWORD  flags; // ACCF_* flags
	DWORD  order;
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

#define ACTM_NEW      0x00000001
#define ACTM_DELETE   0x00000002
#define ACTM_RENAME   0x00000008
#define ACTM_SORT     0x00000010
#define ACTM_LOADED   0x80000000

//	Event: action group list was changed: something was added or deleted
//	wParam: set of ACTM_* flags
//	lParam : 0
#define ME_ACT_CHANGED "Actions/Changed"

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

//----- Scheduling part services -----

//    Enable or disable tasks
//    wParam - 1/0 (enable/disable)
//    lParam - unicode task name
//    Note - works for all tasks with same started name
#define MS_ACT_TASKENABLE "Actions/TaskEnable"

//    Delete task
//    wParam - 0
//    lParam - unicode task name
//    Note - works for all tasks with same started name
#define MS_ACT_TASKDELETE "Actions/TaskDelete"

//    Set task repeat count
//    wParam - repeat count
//    lParam - unicode task name
//    Return - old repeat count value
//    Note - works for all tasks with same started name
#define MS_ACT_TASKCOUNT "Actions/TaskCount"

//    Event for task start
//    wParam - counter of call (from 0 to repeat count)
//    lParam - unicode task name
#define ME_ACT_BELL "Actions/Bell"

#endif
