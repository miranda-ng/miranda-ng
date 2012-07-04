{$IFNDEF M_DBEDITOR}
{$DEFINE M_DBEDITOR}

const
{
  wParam: Module name
  lParam: 0
  Affect: Register single module as 'known'
  Note: must be used after or in ME_SYSTEM_MODULESLOADED
}
  MS_DBEDIT_REGISTERSINGLEMODULE:PAnsiChar = 'DBEditorpp/RegisterSingleModule';

{
  wParam: array with module names
  lParam: count of module names
  Affect: Register modules as 'known'
  Note: must be used after or in ME_SYSTEM_MODULESLOADED
  Example:
   var mods:array [0..2] = ('Module1','Module2','Module3');
   ...
   CallService(MS_DBEDIT_REGISTERMODULE,WPARAM(@mods),3);

}
  MS_DBEDIT_REGISTERMODULE:PAnsiChar = 'DBEditorpp/RegisterModule';

{
  wParam: hContact
  lParam: 0
  Affect: Open user tree in DBE++
}
  MS_DBEDIT_MENUCOMMAND:PAnsiChar = 'DBEditorpp/MenuCommand';

{
  Import settings\contacts from file
  wParam=(HANDLE)hContact
  lParam=(char*)FilePath
  always returns 0
}
  MS_DBEDIT_IMPORT:PAnsiChar = 'DBEditorpp/Import';

{$ENDIF}
