{$IFNDEF M_ALARMS}
{$DEFINE M_ALARMS}

const
// flags for alarm action
  AAF_POPUP   = 1; // show a popup window (or a popup from popups plugin, if installed and activated via options)
  AAF_SOUND   = 2; // play a sound
  AAF_COMMAND = 4; // run a command
  AAF_SYSTRAY = 8; // flash systray icon (not implemented)

type
  TOccurrence = (
    OC_ONCE,     // all fields in time (see below) are valid
    OC_DAILY,    // only wHour, wMinute, and wSecond are valid
    OC_WEEKLY,   // wHour, wMinute, wSecond, and wDayOfWeek are valid
    OC_WEEKDAYS, // only wHour, wMinute, and wSecond are valid
    OC_MONTHLY,  // wHour, wMinute, wSecond, and wDay are valid
    OC_YEARLY    // all fields except wYear are valid
  );

const
// flags
  ALF_HIDDEN     = $01; // do not show in GUI (either options or reminder frame)
  ALF_NOREMINDER = $02; // do not show in reminder frame
  ALF_SUSPENDED  = $04; // do not trigger next occurence
  ALF_NOSTARTUP  = $08; // do not trigger on startup if it was due when miranda was not running
  ALF_NOSNOOZE   = $10; // do not allow snoozing of this alarm

type
  TALARMINFO = record
    szTitle        :TChar;
    szDesc         :TChar;
    occurrence     :TOccurrence;
    snoozer        :LongBool;    // this alarm is a 'once-off', the result of the user pressing the 'snooze'
                                 // button - the time field no longer contains the original alarm time
    time           :TSYSTEMTIME; // the time the alarm is triggered at - different fields are valid depending on
                                 // what the 'occurence' value is set to (see definition of Occurence type above)
    action         :word;        // bitwise OR of AAF_* constants above
    szCommand      :TChar;       // passed to ShellExecute (if action & AAF_COMMAND) when the alarm is triggered
    szCommandParams:TChar;       // passed as parameters for above command
    sound_num      :byte;        // use alarm sound 1, 2, or 3 (if action & AAF_SOUND) (4 == speak, version 0.0.7.0+)
    flags          :int;         // ALF_* above
  end;

const
// set an alarm
// wparam = 0
// lparam = (ALARMINFO *)&alarm
  MS_ALARMS_ADDALARM:PAnsiChar = 'Alarms/AddAlarm';

// event sent when an alarm is triggered
// wparam=0
// lparam=(ALARMINFO *)&alarm
// returning non-zero from your hook will prevent the alarm actions from being carried out
  ME_ALARMS_TRIGGERED:PAnsiChar = 'Alarms/Triggered';

{$ENDIF}
