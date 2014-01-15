{$INFDEF NUDGE}
{$DEFINE NUDGE}
const
// protocol service that sends a nudge
  PS_SEND_NUDGE = '/SendNudge';

// event to be create in protocol with its m_szModuleName
  PE_NUDGE = '/Nudge';

const
  MS_SHAKE_CLIST         = 'SHAKE/Service/ShakeClist';
  MS_SHAKE_CHAT          = 'SHAKE/Service/ShakeChat';
  MS_SHAKE_CLIST_TRIGGER = 'SHAKE/Service/TriggerShakeClist';
  MS_SHAKE_CHAT_TRIGGER  = 'SHAKE/Service/TirggerShakeChat';
  MS_NUDGE_SEND          = 'NUDGE/Send';
{$ENDIF}
