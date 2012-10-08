// defined in interfaces.inc
//const MIID_ACTMANAGER:MUUID='{9584DA04-FB4F-40c1-9325-E4F9CAAFCB5D}';

// hotkey and action (common) flags
const
  ACF_DISABLED = $10000000;  // action disabled
  ACF_USEDNOW  = $20000000;  // action in use (reserved)
  ACF_DOBREAK  = $40000000;  // special, make break;
  ACF_ASSIGNED = $80000000;  // action assigned
  ACF_EXPORT   = $08000000;  // action to export
  ACF_IMPORT   = ACF_EXPORT; // imported
  ACF_VOLATILE = $04000000;  // don't save in DB
  ACF_PARALLEL = $02000000;  // parallel action work (no wait thread)
  ACF_OVERLOAD = $01000000;  // imported action overwrite old

// action flags
const
  // ACT_CONTACT
  ACF_KEEPONLY = $00000001; // keep contact handle in Last, don't show window
  
  // ACT_SERVICE
  ACF_WPARNUM  = $00000001; // wParam is number
  ACF_LPARNUM  = $00000002; // lParam is number
  ACF_WUNICODE = $00000004; // wParam is Unicode string
  ACF_LUNICODE = $00000008; // lParam is Unicode string
  ACF_WCURRENT = $00000010; // wParam is ignored, used current user handle
                            // from current message window
  ACF_LCURRENT = $00000020; // lParam is ignored, used current user handle
                            // from current message window
  ACF_WPARHEX  = $00000040; //!! Show as hex
  ACF_LPARHEX  = $00000080; //!! Show as hex

  ACF_WRESULT  = $00010000; // wParam is previous action result
  ACF_LRESULT  = $00020000; // lParam is previous action result
  ACF_WPARAM   = $00040000; // wParam is Call parameter
  ACF_LPARAM   = $00080000; // lParam is Call parameter
  ACF_WSTRUCT  = $00100000;
  ACF_LSTRUCT  = $00200000;
  ACF_WPARTYPE = ACF_WPARNUM or ACF_WUNICODE or ACF_WCURRENT or ACF_WPARAM or ACF_WSTRUCT;
  ACF_LPARTYPE = ACF_LPARNUM or ACF_LUNICODE or ACF_LCURRENT or ACF_LPARAM or ACF_LSTRUCT;

  ACF_INSERT   = $00000100; // Insert result in message
  ACF_MESSAGE  = $00000200; // Show service result as message
  ACF_POPUP    = $00000400; // Show service result as popup
  ACF_STRING   = $00000800; // Service result is string
  ACF_UNICODE  = $00001000; // Service result is Widestring
  ACF_HEX      = $00002000; // Result as Hex
  ACF_SIGNED   = $00004000; // Result as signed value
//!!
  ACF_STRUCT   = $00008000; // Service result in structure
  
  // ACT_TEXT
  ACF_CLIPBRD  = $00000002; // Clipboard operations, not window
  ACF_ANSI     = $00000004; // File: ANSI or Unicode (UTF8/UTF16) text
  ACF_COPYTO   = $00000008; // Clipboard operations: 'copy to' or 'paste from'

  ACF_FILE     = $00000010; // File operations
  ACF_FWRITE   = $00000020; // read/write file
  ACF_FAPPEND  = $00000040; // append file

  ACF_UTF8     = $00000080; // File: UTF8 or UTF16
  ACF_SIGN     = $00000100; // File: with signature or not

  // ACT_PROGRAM
  ACF_CURPATH  = $00000002; // Current (not program) path
  ACF_PRTHREAD = $00000004; // parallel Program

  // ACT_ADVANCED
  ACF_VARASINT = $00000001; // if variables script, translate to int

  // ACT_DBRW
  ACF_DBWRITE  = $00000001; // write to (not read from) DB 
  ACF_DBBYTE   = $00000002; // read/write byte (def. dword)
  ACF_DBWORD   = $00000004; // read/write word (def. dword)
  ACF_PARAM    = $00000008; // hContact from parameter
  ACF_CURRENT  = $00000010; // hContact is 0 (user settings)
  ACF_RESULT   = $00000020; // hContact is last result value
  ACF_LAST     = $00000040; // use last result for DB writing
  ACF_DBUTEXT  = $00000080; // read/write Unicode string
  ACF_DBANSI   = $00000082; // read/write ANSI string
  ACF_DBDELETE = $00000100; // delete setting
  ACF_NOCNTCT  = ACF_PARAM or ACF_CURRENT or ACF_RESULT;

  // ACT_CHAIN
  ACF_BYNAME   = $00000001; // Address action link by name, not Id

  // ACT_MESSAGE
  ACF_MSG_KEEP = $00000001; // Keep past 'last result'

const
  // Variables use
  ACF2_SRV_WPAR = $00000001;
  ACF2_SRV_LPAR = $00000002;
  ACF2_SRV_SRVC = $00000004;
  ACF2_SRV_WHEX = $00000008;
  ACF2_SRV_LHEX = $00000010;
  ACF2_PRG_PRG  = $00000001;
  ACF2_PRG_ARG  = $00000002;
  ACF2_TXT_FILE = $00000001;
  ACF2_TXT_TEXT = $00000002;
  ACF2_RW_MVAR  = $00000001;
  ACF2_RW_SVAR  = $00000002;
  ACF2_RW_TVAR  = $00000004;
  ACF2_RW_HEX   = $00000008;
  ACF2_MSG_TTL  = $00000001;
  ACF2_MSG_TXT  = $00000002;

  ACF2_FREEMEM  = $00000100;

const
  ADV_COND_NOP  = 0;
  ADV_COND_GT   = 1;
  ADV_COND_LT   = 2;
  ADV_COND_EQ   = 3;
  ADV_COND_NOT  = $80;

  ADV_ACT_NOP   = 0; // two lower bits $03 mask
  ADV_ACT_BREAK = 1;
  ADV_ACT_JUMP  = 2;

  ADV_ACT_POST  = $0F;

  ADV_ACT_MATH  = $10; // bit masks
  ADV_ACT_VARS  = $20;

  ADV_ACTION    = $F0;

const
  ACT_UNKNOWN = 0;
  ACT_CONTACT = 1;
  ACT_SERVICE = 2;
  ACT_PROGRAM = 3;
  ACT_TEXT    = 4;
  ACT_ADVANCE = 5;
  ACT_CHAIN   = 6;
  ACT_RW      = 7;
  ACT_MESSAGE = 8;
  ACT_MAXTYPE = 8;
  ACT_SPECIAL = ACT_MAXTYPE+1;
  ACT_FINISH  = ACT_SPECIAL+1;

type
  pHKRecord = ^tHKRecord;
  tHKRecord = record
    descr      :pWideChar; // like name
    id         :dword;
    flags      :dword;     // Assigned or not
    firstAction:dword;     // array [0..0] of dword
    active     :pointer;
  end;

  pHKAction = ^tHKAction;
  tHKAction = record
    flags :dword; // See ACF_* constants
    flags2:dword; // See ACF2_* constants (Variables use etc.)
    next  :dword;
    descr :PWideChar;
    case actionType:dword of
      ACT_CONTACT:
        (contact:THANDLE);
      ACT_SERVICE:
        (service:PAnsiChar;
        wparam  :WPARAM;
        lparam  :LPARAM);
      ACT_PROGRAM:
        (prgname:pWideChar;
        args    :pWideChar;
        show    :dword;
        time    :dword);
      ACT_TEXT:
        (text :pWideChar;
        tfile :pWideChar);
      ACT_ADVANCE:
        (condition:dword;
        value     :uint_ptr;
        action    :dword;
        operval   :pWideChar;
        oper      :dword;
        mathval   :dword;
        varval    :pWideChar);
      ACT_CHAIN:
        (id    :dword;
        actname:pWideChar);
      ACT_RW:
        (dbcontact:THANDLE;
        dbmodule  :PAnsiChar;
        dbsetting :PAnsiChar;
        dbvalue   :uint_ptr);
      ACT_MESSAGE:
        (boxopts:dword;
        msgtitle:pWideChar;
        msgtext :pWideChar);
  end;
