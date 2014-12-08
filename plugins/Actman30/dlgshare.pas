unit dlgshare;

interface

uses windows,lowlevelc;

var
  MacroListWindow,
  ActionListWindow:HWND;
var
  EditMacroList:tMacroList;

const
  ACI_APPLY   = 0;
  ACI_NEW     = 1;
  ACI_DELETE  = 2;
  ACI_UP      = 3;
  ACI_DOWN    = 4;
  ACI_TEST    = 5;
  ACI_IMPORT  = 6;
  ACI_EXPORT  = 7;
  ACI_REFRESH = 8;

procedure RegisterIcons;
function OptSetButtonIcon(btn:HWND;num:integer):HICON;
procedure OptFillContactList(wnd:HWND);

implementation

uses messages, m_api, dbsettings, mircontacts, common, global;

{$include i_cnst_dlgshare.inc}
{$resource dlgshare.res}

const
  IconAmount = 9;
const
  Icons:array [0..IconAmount-1] of tIconItem = (
    (szDescr: 'Apply'         ; szName: 'Apply' ; defIconID: IDI_APPLY ; size: 0; hIcolib: 0;),
    (szDescr: 'New'           ; szName: 'New'   ; defIconID: IDI_NEW   ; size: 0; hIcolib: 0;),
    (szDescr: 'Delete'        ; szName: 'Delete'; defIconID: IDI_DELETE; size: 0; hIcolib: 0;),
    (szDescr: 'Up'            ; szName: 'Up'    ; defIconID: IDI_UP    ; size: 0; hIcolib: 0;),
    (szDescr: 'Down'          ; szName: 'Down'  ; defIconID: IDI_DOWN  ; size: 0; hIcolib: 0;),
    (szDescr: 'Test'          ; szName: 'Test'  ; defIconID: IDI_TEST  ; size: 0; hIcolib: 0;),
    (szDescr: 'Import'        ; szName: 'Import'; defIconID: IDI_IMPORT; size: 0; hIcolib: 0;),
    (szDescr: 'Export'        ; szName: 'Export'; defIconID: IDI_EXPORT; size: 0; hIcolib: 0;),
    (szDescr: 'Reload/Refresh'; szName: 'Reload'; defIconID: IDI_RELOAD; size: 0; hIcolib: 0;)
  );


procedure RegisterIcons;
begin
  Icon_Register(hInstance,'Actions',@Icons,IconAmount,'ACI');
end;

function OptSetButtonIcon(btn:HWND;num:integer):HICON;
begin
  result:=CallService(MS_SKIN2_GETICONBYHANDLE,0,LPARAM(Icons[num].hIcolib));
  SendMessage(btn,BM_SETIMAGE,IMAGE_ICON,result);
end;

procedure OptFillContactList(wnd:HWND);
var
  fCLformat:pWideChar;
  fCLfilter:byte;
begin
  fCLfilter:=DBReadByte   (0,DBBranch,'CLfilter',BST_UNCHECKED);
  fCLformat:=DBReadUnicode(0,DBBranch,'CLformat');
  FillContactList(wnd, fCLfilter<>BST_UNCHECKED, fCLformat);
  mFreeMem(fCLformat);
end;

end.
