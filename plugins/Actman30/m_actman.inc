{$IFNDEF M_ACTMAN}
{$DEFINE M_ACTMAN}

// defined in interfaces.inc
//const MIID_ACTMANAGER:MUUID='{9584DA04-FB4F-40c1-9325-E4F9CAAFCB5D}';

const
  AutoStartName:PWideChar = '#Autostart';
const
  // Get list service (full share)
  ACCF_EXPORT   = $08000000;   // action to export (UA export)
  ACCF_IMPORT   = ACCF_EXPORT; // (UA import)

type
  pChain = ^tChain;
  tChain = record
    descr:pWideChar;
    id   :dword;
    flags:dword; // ACCF_* flags ?? right now - just selection/overload
    order:dword; // ??
  end;

const
  {
    wParam - 0
    lParam - address of destination list variable (address of pointer to tChain)
             if lParam=0, return just count of elements
    Return - count of elements
    Notes: first 4 bytes = size of TChain structure (to add new fields in future)
  }
  MS_ACT_GETLIST:PAnsiChar = 'Actions/GetList';
  {
    wParam - 0
    lParam - list address (pointer to data returned by MS_ACT_GETLIST)
  }
  MS_ACT_FREELIST:PAnsiChar = 'Actions/FreeList';
  {
    wParam - id: dword
    lParam - parameter
  }
  MS_ACT_RUNBYID  :PAnsiChar = 'Actions/RunById';
  {
    wParam - unicode action name
    lParam - parameter
  }
  MS_ACT_RUNBYNAME:PAnsiChar = 'Actions/RunByName';

{ Starts action with 2 parameters
  wParam: 0
  lParam: pointer to TAct_Param
}
  MS_ACT_RUNPARAMS:PAnsiChar = 'Actions/RunWithParams';
const
  ACTP_BYNAME     =  1; // id points on unicode name
  ACTP_WAIT       =  2; // waiting for macro finish
  ACTP_NOTIFY     =  4; // notify (raise event) for start/finish macro
  ACTP_SAMETHREAD =  8; // execute macro in same thread (with finish waiting)
  ACTP_KEEPRESULT = 16; // (internal) keep last result
type
  pAct_Param = ^tAct_Param;
  tAct_Param = record
    Id    :uint_ptr; // Id or name
    wParam:WPARAM;
    lParam:LPARAM;
    flags :dword; // ACTP_*
    lPType:dword; // last result (in lParam) type
  end;

const
  ACTM_NEW    = $00000001; // new macros was added
  ACTM_DELETE = $00000002; // some macros was deleted
  ACTM_RENAME = $00000008; // possible, some macro names was changed
  ACTM_SORT   = $00000010; // possible, macro order in list was changed
  ACTM_LOADED = $80000000; // All macros loaded and ready to work (at plugin start)

  {
    Event: action group list was changed: some was added or deleted
    wParam - set of ACTM_* flags
    lParam - 0
  }
  ME_ACT_CHANGED:PAnsiChar = 'Actions/Changed';

  ACIO_EXPORT   = $00000001; // export, else - import
  ACIO_APPEND   = $00000002; // append file on export
  ACIO_ASKEXIST = $00000004; // ask, if action exists on import
  ACIO_SELECTED = $00000008; // export selected actions only

  {
    wParam - ACIO_* flags
    lParam - Unicode file name
    Return - true, if totally succesful 
  }
  MS_ACT_INOUT:PAnsiChar = 'Actions/ImpExp';

  {
    Event: Export actions
    wParam - ACIO_* flags
    lParam - unicode filename
  }
  ME_ACT_INOUT:PAnsiChar = 'Actions/InOut';

  {
    Select/unselect specified action
    wParam - mask (bit 0 = ID if set/name; bit 1 = clear if set/set; bit 2 = get if set/set)
    lParam - unicode action name / number
    Return - 0 if unsuccesful
  }
  MS_ACT_SELECT:PAnsiChar = 'Actions/Select';

  {
    Event: Action started/finished
    wParam - Action status: 0 - started, 1 - finished
    lParam - action id
  }
  ME_ACT_ACTION:PAnsiChar = 'Actions/Action';

//----- Scheduling part services -----

const
  {
    Enable or disable tasks
    wParam - 1/0 (enable/disable)
    lParam - unicode task name
    Note - works for all tasks with same started name
  }
  MS_ACT_TASKENABLE:PAnsiChar = 'Actions/TaskEnable';

  {
    Delete task
    wParam - 0
    lParam - unicode task name
    Note - works for all tasks with same started name
  }
  MS_ACT_TASKDELETE:PAnsiChar = 'Actions/TaskDelete';

  {
    Set task repeat count
    wParam - repeat count
    lParam - unicode task name
    Return - old repeat count value
    Note - works for all tasks with same started name
  }
  MS_ACT_TASKCOUNT:PAnsiChar = 'Actions/TaskCount';

  {
    Event for task start
    wParam - counter of call (from 0 to repeat count)
    lParam - unicode task name
  }
  ME_ACT_BELL:PAnsiChar = 'Actions/Bell';

{$ENDIF}
