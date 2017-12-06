For what:
  Insert Played music info in message window or Status text. ICQ extended
  status supported.

Requirements:
  Windows 2K/XP (SP2 better)
  Miranda 0.6.x

Supported Players:
  Winamp                  (http://www.winamp.com/)
  Apollo                  (http://www.iki.fi/hy/apollo/)
  1by1                    (http://www.mpesch3.de/)
  Media Player Classic    (http://gabest.org/)
  Window Media Player     (http://www.microsoft.com/windows/windowsmedia/players.aspx)
  FooBar2000              (http://www.foobar2000.org/)
  LightAlloy              (http://www.softella.com/)
  Cowon JetAudio          (http://www.jetaudio.com/)
  Quintessential Player   (http://quinnware.com/)
  iTunes                  (http://www.itunes.com/)
  MediaMonkey             (http://www.mediamonkey.com/)
  Real Player             (http://www.real.com/)
  MusikCube               (http://www.musikcube.com/)
  BSPlayer                (http://www.bsplayer.org/)
  MusicCube One           (http://www.rodi.dk/musiccubeone)
  Zoom Player             (http://www.inmatrix.com/)
  Pluton                  (http://pluton.oss.ru/)
  J. River Media Center   (http://www.jrmediacenter.com/)
  Musicmatch Jukebox      (http://wwwp.musicmatch.com/)
  VideoLAN media player   (http://www.videolan.org/)
  mRadio miranda plugin   (http://miranda.kom.pl/dev/bankrut/)
  ALSong&ALShow           (http://www.altools.net/)

  and others. List of other players see in player.ini

Supported Formats:
  MP3, OGG, WMA, WAV, APE, TTA, AAC, FLA/FLAC, MPC, OFR/OFS, SPX, MP4, M4A,
  ASF, WMV, AVI, MKV, OGM, RA/RM/RAM, FLV, MOV, 3GP, MPEG/MPG

In Chat (and maybe message) window, you can use text formatting:
{b}text{/b} - 'bold' text
{i}text{/i} - 'italic' text
{u}text{/u} - 'undeline' text
{cf##}text{/cf} - text with color ## (0-15)
{bg##}text{/bg} - text with background color ## (0-15)
Sample:/me {b}listen{/b} {cf5}{i}%artist%{/i}{/cf} - {bg10}{u}"%title%"{/u}{/bg}

Macros:
 %album%      - album
 %artist%     - artist
 %bitrate%    - bitrate
 %channels%   - number of channels
 %codec%      - video codec like 0x30355844 (DX50)
 %comment%    - comment from tag
 %cover%      - cover file name
 %file%       - media file name
 %fps%        - 100*FPS (Frames Per Second) for video files
 %height%     - video height in pixels
 %genre%      - genre
 %kbps%       - bitrate
 %khz%        - samplerate
 %length%     - total song length (sec)
 %lyric%      - lyric text
 %mono%       - "mono"/"stereo"
 %nstatus%'   - player status (stopped,playing,paused) - nontranslated
 %percent%    - %time% / %total% * 100%
 %player%     - player
 %samplerate% - samplerate
 %size%       - media file size
 %status%'    - player status (stopped,playing,paused)
 %time%       - current song position (sec)
 %title%      - song title
 %total%      - total song length (sec)
 %track%      - track number
 %txtver%     - player version in text format
 %type%       - media file type (extension)
 %vbr%        - 'VBR' if VBR :)
 %version%    - player version
 %width%      - video width in pixels
 %wndtext%    - title from player window (usually "artist" - "title")
 %year%       - song year (from tag)

Notes:
 - Volume field has a original volume value in hiword and scaled to 0-15 range
   in loword. Not all players supported.
 - Frame background picture transparence is not implemented
 - To obtain more information from foobar2000 player, you must download plugin
   http://foosion.foobar2000.org/0.9/foo_comserver2-0.7-setup.exe
   or use foo_winampspam plugin
 - FileInfo (called from popup) not shows Video file properties
 - FPS saves as FPS*100
 - Frame text output is Left-to-Right only now

Known BUGs:
 - if Frame text uses %time% macro, text can't scroll
 - Foobar seeking with foo_winamp_spam may not work
 - Miranda can freeze if Foobar OLE interface used
 - Player can start again if OLE checks while it shutdown
 - ICQLite and ICQ2003 users cannot see XStatus text changes
 - in MP3 with VBR total song time sometime is not properly calculated
 - default color of formated text is color of text at insert position
 - some macros don't work with some players. Replaced by '' and 0
 - 1by1 player can show wrong elapsed time value
 - some players version not properly displayed
 - bad MP3 headers not properly handled
 - plugin can show wrong song when Player scans directory for music files

PS. To compile plugin you must use something like this:
  dcc32 -$A+ -$H+ watrack.dpr

All comments, errors & wishes please send to awkward@land.ru or panda75@bk.ru
