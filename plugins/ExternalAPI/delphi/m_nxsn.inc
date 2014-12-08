{$IFNDEF M_NXSN}
{$DEFINE M_NXSN}

const
// tabSRMM stuff (logging to message window)
  EVENTTYPE_STATUSCHANGE = 25368;

{
MCONTACT hContact = wParam;
WORD oldStatus = LOWORD(lParam);
WORD newStatus = HIWORD(lParam);
oldStatus is the status the contact was before the change.
newStatus is the status the contact is now.
Cast them to (int) if you need them that way.
}
  ME_STATUSCHANGE_CONTACTSTATUSCHANGED:PAnsiChar = 'Miranda/StatusChange/ContactStatusChanged';

  MS_STATUSCHANGE_MENUCOMMAND:PAnsiChar = 'NewStatusNotify/EnableDisableMenuCommand';

{$ENDIF}
