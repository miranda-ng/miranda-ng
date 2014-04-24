{to Variables plugin and Help dialog}
unit macros;

interface

type
  pvar = ^tvar;
  tvar = packed record
    name :PWideChar;
    alias:PWideChar;
    help :PAnsiChar;
  end;

// --- data ---
const
  numvars = 35;

  mn_wndtext    = 0;
  mn_artist     = 1;
  mn_title      = 2;
  mn_album      = 3;
  mn_genre      = 4;
  mn_file       = 5;
  mn_kbps       = 6;
  mn_bitrate    = 7;
  mn_track      = 8;
  mn_channels   = 9;
  mn_mono       = 10;
  mn_khz        = 11;
  mn_samplerate = 12;
  mn_total      = 13;
  mn_length     = 14;
  mn_year       = 15;
  mn_time       = 16;
  mn_percent    = 17;
  mn_comment    = 18;
  mn_player     = 19;
  mn_version    = 20;
  mn_size       = 21;
  mn_type       = 22;
  mn_vbr        = 23;
  mn_status     = 24;
  mn_fps        = 25;
  mn_codec      = 26;
  mn_width      = 27;
  mn_height     = 28;
  mn_txtver     = 29;
  mn_lyric      = 30;
  mn_cover      = 31;
  mn_volume     = 32;
  mn_playerhome = 33;
  mn_nstatus    = 34;
  vars:array [0..numvars-1] of tvar = (
{00} (name:'wndtext'   ;alias:nil;help:'player window title'),
{01} (name:'artist'    ;alias:nil;help:'artist'),
{02} (name:'title'     ;alias:nil;help:'song title'),
{03} (name:'album'     ;alias:nil;help:'album'),
{04} (name:'genre'     ;alias:nil;help:'genre'),
{05} (name:'file'      ;alias:nil;help:'media file name'),
{06} (name:'kbps'      ;alias:nil;help:'bitrate'),
{07} (name:'bitrate'   ;alias:nil;help:nil),
{08} (name:'track'     ;alias:nil;help:'track number'),
{09} (name:'channels'  ;alias:nil;help:'number of channels'),
{10} (name:'mono'      ;alias:nil;help:'"mono"/"stereo"'),
{11} (name:'khz'       ;alias:nil;help:'samplerate'),
{12} (name:'samplerate';alias:nil;help:nil),
{13} (name:'total'     ;alias:nil;help:'total song length (sec)'),
{14} (name:'length'    ;alias:nil;help:nil),
{15} (name:'year'      ;alias:nil;help:'song year (date)'),
{16} (name:'time'      ;alias:nil;help:'current song position (sec)'),
{17} (name:'percent'   ;alias:nil;help:'time/length * 100%'),
{18} (name:'comment'   ;alias:nil;help:'comment from tag'),
{19} (name:'player'    ;alias:nil;help:'player name'),
{20} (name:'version'   ;alias:nil;help:'player version'),
{21} (name:'size'      ;alias:nil;help:'media file size'),
{22} (name:'type'      ;alias:nil;help:'media file type'),
{23} (name:'vbr'       ;alias:nil;help:'VBR or not (empty)'),
{24} (name:'status'    ;alias:nil;help:'player status (stopped,playing,paused)'),
{25} (name:'fps'       ;alias:nil;help:'FPS (frames per second), video only'),
{26} (name:'codec'     ;alias:nil;help:'codec, video only'),
{27} (name:'width'     ;alias:nil;help:'width, video only'),
{28} (name:'height'    ;alias:nil;help:'height, video only'),
{29} (name:'txtver'    ;alias:nil;help:'player version in text format'),
{30} (name:'lyric'     ;alias:nil;help:'Lyric from ID3v2 tag'),
{31} (name:'cover'     ;alias:nil;help:'Cover file path'),
{32} (name:'volume'    ;alias:nil;help:'Player volume (0-15)'),
{33} (name:'playerhome';alias:nil;help:'Player homepage URL'),
{34} (name:'nstatus'   ;alias:nil;help:'player status (not translated)')
  );

implementation

end.
