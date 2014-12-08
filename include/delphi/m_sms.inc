{$IFNDEF M_SMS}
{$DEFINE M_SMS}

const
  ACKTYPE_SMS = 1001;

  EVENTTYPE_SMS             = 2001;
  EVENTTYPE_SMSCONFIRMATION = 3001;

{
  Send an SMS
  wParam=(WPARAM)(const AnsiChar*)szPhoneNumber
  lParam=(LPARAM)(const AnsiChar*)szMessage
  Returns a HANDLE to the send on success, or NULL on failure
  szPhoneNumber should be the full number with international code and preceeded by a +
}
  MS_SMS_SEND = '/SendSMS';

{$ENDIF}
