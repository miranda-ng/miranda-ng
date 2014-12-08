{$IFNDEF M_DBEDITOR}
{$DEFINE M_DBEDITOR}

const
{
  wParam: hContact
  lParam: 0
  Affect: Open user tree in DBE++
}
  MS_DBEDIT_MENUCOMMAND:PAnsiChar = 'DBEditorpp/MenuCommand';

{
  Import settings\contacts from file
  wParam=hContact
  lParam=(char*)FilePath
  always returns 0
}
  MS_DBEDIT_IMPORT:PAnsiChar = 'DBEditorpp/Import';

{$ENDIF}
