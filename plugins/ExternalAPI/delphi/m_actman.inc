{$IFNDEF M_ACTMAN}
{$DEFINE M_ACTMAN}

// defined in interfaces.inc
//const MIID_ACTMANAGER:MUUID='{9584DA04-FB4F-40c1-9325-E4F9CAAFCB5D}';

const
  AutoStartName:PWideChar = '#Autostart';
const
  DBBranch = 'ActMan';
const
  ACCF_DISABLED = $10000000;  // action disabled
  ACCF_EXPORT   = $08000000;  // action to export
  ACCF_VOLATILE = $04000000;  // don't save in DB
  ACCF_IMPORTED = ACCF_EXPORT;
  ACCF_FLAGS    = ACCF_DISABLED or ACCF_EXPORT or ACCF_IMPORTED or ACCF_VOLATILE;
  ACCF_OVERLOAD = $01000000;  // imported action overwrite old

  ACCF_ID       = $02000000;  // for MS_ACT_SELECT, lParam is ID (else name)
  ACCF_CLEAR    = $01000000;  // clear other flags, else - set
type
  pChain = ^tChain;
  tChain = record
    descr:PWideChar;
    id   :dword;
    flags:dword; // ACCF_* flags
    order:dword;
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
  ACTP_BYNAME = 1;
  ACTP_WAIT   = 2;
type
  pAct_Param = ^tAct_Param;
  tAct_Param = record
    flags :dword; // ACTP_*
    Id    :uint_ptr; // Id or name
    wParam:WPARAM;
    lParam:LPARAM;
  end;

const
  ACTM_NEW    = $00000001;
  ACTM_DELETE = $00000002;
  ACTM_RELOAD = $00000004;
  ACTM_RENAME = $00000008;
  ACTM_SORT   = $00000010;
  ACTM_ACT    = $10000000; // do not check, internal
  ACTM_ACTS   = $20000000; // do not check, internal
  ACTM_LOADED = $80000000;

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
    wParam - set of ACCF_* consts
    lParam - unicode action name / number
    Return - -1 if unsuccesful
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
