{$IFNDEF M_NUDGE}
{$DEFINE M_NUDGE}
const
// protocol service that sends a nudge
  PS_SEND_NUDGE = '/SendNudge';

// event to be create in protocol with its m_szModuleName
  PE_NUDGE = '/Nudge';

const
  MS_SHAKE_CLIST        :PAnsiChar = 'SHAKE/Service/ShakeClist';
  MS_SHAKE_CHAT         :PAnsiChar = 'SHAKE/Service/ShakeChat';
  MS_SHAKE_CLIST_TRIGGER:PAnsiChar = 'SHAKE/Service/TriggerShakeClist';
  MS_SHAKE_CHAT_TRIGGER :PAnsiChar = 'SHAKE/Service/TirggerShakeChat';
  MS_NUDGE_SEND         :PAnsiChar = 'NUDGE/Send';

{$ENDIF}
