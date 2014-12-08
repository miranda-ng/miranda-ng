unit appcmdapi;
interface

uses windows;

const
  APPCOMMAND_BROWSER_BACKWARD       =  1; // Navigate backward.
  APPCOMMAND_BROWSER_FORWARD        =  2; // Navigate forward.
  APPCOMMAND_BROWSER_REFRESH        =  3; // Refresh page.
  APPCOMMAND_BROWSER_STOP           =  4; // Stop download.
  APPCOMMAND_BROWSER_SEARCH         =  5; // Open search.
  APPCOMMAND_BROWSER_FAVORITES      =  6; // Open favorites.
  APPCOMMAND_BROWSER_HOME           =  7; // Navigate home.
  APPCOMMAND_VOLUME_MUTE            =  8; // Mute the volume.
  APPCOMMAND_VOLUME_DOWN            =  9; // Lower the volume.
  APPCOMMAND_VOLUME_UP              = 10; // Raise the volume
  APPCOMMAND_MEDIA_NEXTTRACK        = 11; // Go to next track.
  APPCOMMAND_MEDIA_PREVIOUSTRACK    = 12; // Go to previous track.
  APPCOMMAND_MEDIA_STOP             = 13; // Stop playback.
  APPCOMMAND_MEDIA_PLAY_PAUSE       = 14; // Play or pause playback. If there are discrete Play
                                          // and Pause buttons, applications should take action
                                          // on this command as well as APPCOMMAND_MEDIA_PLAY and
                                          // APPCOMMAND_MEDIA_PAUSE.
  APPCOMMAND_LAUNCH_MAIL            = 15; // Open mail.
  APPCOMMAND_LAUNCH_MEDIA_SELECT    = 16; // Go to Media Select mode
  APPCOMMAND_MEDIA_SELECT           = APPCOMMAND_LAUNCH_MEDIA_SELECT;
  APPCOMMAND_LAUNCH_APP1            = 17; // Start App1.
  APPCOMMAND_LAUNCH_APP2            = 18; // Start App2.
  APPCOMMAND_BASS_DOWN              = 19; // Decrease the bass.
  APPCOMMAND_BASS_BOOST             = 20; // Toggle the bass boost on and off.
  APPCOMMAND_BASS_UP                = 21; // Increase the bass.
  APPCOMMAND_TREBLE_DOWN            = 22; // Decrease the treble.
  APPCOMMAND_TREBLE_UP              = 23; // Increase the treble.

  APPCOMMAND_MICROPHONE_VOLUME_MUTE = 24; // Windows XP: Mute the microphone.
  APPCOMMAND_MICROPHONE_VOLUME_DOWN = 25; // Windows XP: Decrease microphone volume.
  APPCOMMAND_MICROPHONE_VOLUME_UP   = 26; // Windows XP: Increase microphone volume.
  APPCOMMAND_HELP                   = 27; // Windows XP: Open the Help dialog.
  APPCOMMAND_FIND                   = 28; // Windows XP: Open the Find dialog.
  APPCOMMAND_NEW                    = 29; // Windows XP: Create a new window.
  APPCOMMAND_OPEN                   = 30; // Windows XP: Open a window.
  APPCOMMAND_CLOSE                  = 31; // Windows XP: Close the window (not the application).
  APPCOMMAND_SAVE                   = 32; // Windows XP: Save current document.
  APPCOMMAND_PRINT                  = 33; // Windows XP: Print current document.
  APPCOMMAND_UNDO                   = 34; // Windows XP: Undo last action.
  APPCOMMAND_REDO                   = 35; // Windows XP: Redo last action.
  APPCOMMAND_COPY                   = 36; // Windows XP: Copy the selection.
  APPCOMMAND_CUT                    = 37; // Windows XP: Cut the selection.
  APPCOMMAND_PASTE                  = 38; // Windows XP: Paste
  APPCOMMAND_REPLY_TO_MAIL          = 39; // Windows XP: Reply to a mail message.
  APPCOMMAND_FORWARD_MAIL           = 40; // Windows XP: Forward a mail message. 
  APPCOMMAND_SEND_MAIL              = 41; // Windows XP: Send a mail message.
  APPCOMMAND_SPELL_CHECK            = 42; // Windows XP: Initiate a spell check.
  APPCOMMAND_DICTATE_OR_COMMAND_CONTROL_TOGGLE = 43;
  // Windows XP:Toggles between two modes of speech input: dictation and command/control
  // (giving commands to an application or accessing menus). 
  APPCOMMAND_MIC_ON_OFF_TOGGLE      = 44; // Windows XP: Toggle the microphone.
  APPCOMMAND_CORRECTION_LIST        = 45; // Windows XP: Brings up the correction list when
                                          // a word is incorrectly identified during speech input.

  APPCOMMAND_MEDIA_PLAY             = 46; // Windows XP SP1: Begin playing at the current position.
                                          // If already paused, it will resume. This is a direct
                                          // PLAY command that has no state. If there are
                                          // discrete Play and Pause buttons, applications should
                                          // take action on this command as well as
                                          // APPCOMMAND_MEDIA_PLAY_PAUSE.
  APPCOMMAND_MEDIA_PAUSE            = 47; // Windows XP SP1: Pause. If already paused, take no
                                          // further action. This is a direct PAUSE command that
                                          // has no state. If there are discrete Play and Pause
                                          // buttons, applications should take action on this 
                                          // command as well as APPCOMMAND_MEDIA_PLAY_PAUSE. 
  APPCOMMAND_MEDIA_RECORD           = 48; // Windows XP SP1: Begin recording the current stream.
  APPCOMMAND_MEDIA_FAST_FORWARD     = 49; // Windows XP SP1: Increase the speed of stream playback.
                                          // This can be implemented in many ways, for example,
                                          // using a fixed speed or toggling through a series of
                                          // increasing speeds. 
  APPCOMMAND_MEDIA_REWIND           = 50; // Windows XP SP1: Go backward in a stream at a higher
                                          // rate of speed. This can be implemented in many ways,
                                          // for example, using a fixed speed or toggling through
                                          // a series of increasing speeds. 
  APPCOMMAND_MEDIA_CHANNEL_UP       = 51; // Windows XP SP1: Increment the channel value.
  APPCOMMAND_MEDIA_CHANNEL_DOWN     = 52; // Windows XP SP1: Decrement the channel value. 

function SendMMCommand(wnd:HWND; cmd:integer):integer;

implementation

const
  WM_APPCOMMAND = $0319;

function SendMMCommand(wnd:HWND; cmd:integer):integer;
begin
//  result:=ord(SendMessageW(wnd,WM_APPCOMMAND,wnd,cmd shl 16));
  result:=ord(SendNotifyMessageW(wnd,WM_APPCOMMAND,wnd,cmd shl 16));
end;

end.
