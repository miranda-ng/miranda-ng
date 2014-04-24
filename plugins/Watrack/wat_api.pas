unit wat_api;

interface

uses windows;

{$Include m_music.inc}

function GenreName(idx:cardinal):pWideChar;

// support procedures
procedure ClearSongInfoData(var dst:tSongInfo);
procedure ClearPlayerInfo  (var dst:tSongInfo);
procedure ClearFileInfo    (var dst:tSongInfo);
procedure ClearChangingInfo(var dst:tSongInfo);
procedure ClearTrackInfo   (var dst:tSongInfo);

procedure CopyPlayerInfo  (const src:tSongInfo;var dst:tSongInfo);
procedure CopyFileInfo    (const src:tSongInfo;var dst:tSongInfo);
procedure CopyChangingInfo(const src:tSongInfo;var dst:tSongInfo);
procedure CopyTrackInfo   (const src:tSongInfo;var dst:tSongInfo);


implementation

uses common;

const
  MAX_MUSIC_GENRES = 148;

Genres:array [0..MAX_MUSIC_GENRES-1] of PWideChar = (
{0} 'Blues',
{1} 'Classic Rock',
{2} 'Country',
{3} 'Dance',
{4} 'Disco',
{5} 'Funk',
{6} 'Grunge',
{7} 'Hip-Hop',
{8} 'Jazz',
{9} 'Metal',
{10} 'New Age',
{11} 'Oldies',
{12} 'Other',
{13} 'Pop',
{14} 'R&B',
{15} 'Rap',
{16} 'Reggae',
{17} 'Rock',
{18} 'Techno',
{19} 'Industrial',
{20} 'Alternative',
{21} 'Ska',
{22} 'Death Metal',
{23} 'Pranks',
{24} 'Soundtrack',
{25} 'Euro-Techno',
{26} 'Ambient',
{27} 'Trip-Hop',
{28} 'Vocal',
{29} 'Jazz+Funk',
{30} 'Fusion',
{31} 'Trance',
{32} 'Classical',
{33} 'Instrumental',
{34} 'Acid',
{35} 'House',
{36} 'Game',
{37} 'Sound Clip',
{38} 'Gospel',
{39} 'Noise',
{40} 'AlternRock',
{41} 'Bass',
{42} 'Soul',
{43} 'Punk',
{44} 'Space',
{45} 'Meditative',
{46} 'Instrumental Pop',
{47} 'Instrumental Rock',
{48} 'Ethnic',
{49} 'Gothic',
{50} 'Darkwave',
{51} 'Techno-Industrial',
{52} 'Electronic',
{53} 'Pop-Folk',
{54} 'Eurodance',
{55} 'Dream',
{56} 'Southern Rock',
{57} 'Comedy',
{58} 'Cult',
{59} 'Gangsta',
{60} 'Top 40',
{61} 'Christian Rap',
{62} 'Pop/Funk',
{63} 'Jungle',
{64} 'Native American',
{65} 'Cabaret',
{66} 'New Wave',
{67} 'Psychadelic',
{68} 'Rave',
{69} 'Showtunes',
{70} 'Trailer',
{71} 'Lo-Fi',
{72} 'Tribal',
{73} 'Acid Punk',
{74} 'Acid Jazz',
{75} 'Polka',
{76} 'Retro',
{77} 'Musical',
{78} 'Rock & Roll',
{79} 'Hard Rock',
{80} 'Folk',
{81} 'Folk-Rock',
{82} 'National Folk',
{83} 'Swing',
{84} 'Fast Fusion',
{85} 'Bebob',
{86} 'Latin',
{87} 'Revival',
{88} 'Celtic',
{89} 'Bluegrass',
{90} 'Avantgarde',
{91} 'Gothic Rock',
{92} 'Progressive Rock',
{93} 'Psychedelic Rock',
{94} 'Symphonic Rock',
{95} 'Slow Rock',
{96} 'Big Band',
{97} 'Chorus',
{98} 'Easy Listening',
{99} 'Acoustic',
{100} 'Humour',
{101} 'Speech',
{102} 'Chanson',
{103} 'Opera',
{104} 'Chamber Music',
{105} 'Sonata',
{106} 'Symphony',
{107} 'Booty Brass',
{108} 'Primus',
{109} 'Porn Groove',
{110} 'Satire',
{111} 'Slow Jam',
{112} 'Club',
{113} 'Tango',
{114} 'Samba',
{115} 'Folklore',
{116} 'Ballad',
{117} 'Poweer Ballad',
{118} 'Rhytmic Soul',
{119} 'Freestyle',
{120} 'Duet',
{121} 'Punk Rock',
{122} 'Drum Solo',
{123} 'A Capela',
{124} 'Euro-House',
{125} 'Dance Hall',
{126} 'Goa',
{127} 'Drum & Bass',
{128} 'Club-House',
{129} 'Hardcore',
{130} 'Terror',
{131} 'Indie',
{132} 'BritPop',
{133} 'Negerpunk',
{134} 'Polsk Punk',
{135} 'Beat',
{136} 'Christian Gangsta Rap',
{137} 'Heavy Metal',
{138} 'Black Metal',
{139} 'Crossover',
{140} 'Contemporary Christian',
{141} 'Christian Rock',
{142} 'Merengue',
{143} 'Salsa',
{144} 'Trash Metal',
{145} 'Anime',
{146} 'JPop',
{147} 'Synthpop');

function GenreName(idx:cardinal):pWideChar;
begin
  if idx<MAX_MUSIC_GENRES then
  begin
    StrDupW(result,Genres[idx]);
{
    mGetMem(result,64*SizeOf(WideChar));
    LoadStringW(hInstance,idx,result,64);
}
//  result:=Genres[idx];
  end
  else
    result:=nil;
end;

//----- support procedures -----

// changing data
procedure ClearChangingInfo(var dst:tSongInfo);
begin
  dst.time  :=0;
  dst.volume:=0;

  mFreeMem(dst.wndtext);
end;

procedure CopyChangingInfo(const src:tSongInfo;var dst:tSongInfo);
begin
  dst.time   :=src.time;
  dst.volume :=src.volume;

  StrDupW(dst.wndtext,src.wndtext);
end;

// file data
procedure ClearFileInfo(var dst:tSongInfo);
begin
  mFreeMem(dst.mfile);

  dst.fsize:=0;
  dst.date :=0;
end;

procedure CopyFileInfo(const src:tSongInfo;var dst:tSongInfo);
begin
  StrDupW(dst.mfile,src.mfile);

  dst.fsize:=src.fsize;
  dst.date :=src.date;
end;

// player data
procedure ClearPlayerInfo(var dst:tSongInfo);
begin
  mFreeMem(dst.player);
  mFreeMem(dst.txtver);
  mFreeMem(dst.url);

  if dst.icon<>0 then
    DestroyIcon(dst.icon);
  dst.icon     :=0;

  dst.plyver   :=0;
  dst.plwnd    :=0;
  dst.winampwnd:=0;
end;

procedure CopyPlayerInfo(const src:tSongInfo;var dst:tSongInfo);
begin
  StrDupW(dst.player,src.player);
  StrDupW(dst.txtver,src.txtver);
  StrDupW(dst.url   ,src.url);

  if src.icon<>0 then
    dst.icon:=CopyIcon(src.icon);

  dst.plyver   :=src.plyver;
  dst.plwnd    :=src.plwnd;
  dst.winampwnd:=src.winampwnd;
end;

// track data
procedure ClearTrackInfo(var dst:tSongInfo);
begin
  mFreeMem(dst.artist);
  mFreeMem(dst.title);
  mFreeMem(dst.album);
  mFreeMem(dst.genre);
  mFreeMem(dst.comment);
  mFreeMem(dst.year);
  mFreeMem(dst.lyric);
  mFreeMem(dst.cover);

  dst.kbps    :=0;
  dst.khz     :=0;
  dst.channels:=0;
  dst.track   :=0;
  dst.total   :=0;
  dst.vbr     :=0;
  dst.codec   :=0;
  dst.width   :=0;
  dst.height  :=0;
  dst.fps     :=0;
end;

procedure CopyTrackInfo(const src:tSongInfo;var dst:tSongInfo);
begin
  StrDupW(dst.artist ,src.artist);
  StrDupW(dst.title  ,src.title);
  StrDupW(dst.album  ,src.album);
  StrDupW(dst.genre  ,src.genre);
  StrDupW(dst.comment,src.comment);
  StrDupW(dst.year   ,src.year);
  StrDupW(dst.lyric  ,src.lyric);
  StrDupW(dst.cover  ,src.cover);

  dst.kbps    :=src.kbps;
  dst.khz     :=src.khz;
  dst.channels:=src.channels;
  dst.track   :=src.track;
  dst.total   :=src.total;
  dst.vbr     :=src.vbr;
  dst.codec   :=src.codec;
  dst.width   :=src.width;
  dst.height  :=src.height;
  dst.fps     :=src.fps;
end;


procedure ClearSongInfoData(var dst:tSongInfo);
begin
  ClearPlayerInfo  (dst);
  ClearChangingInfo(dst);
  ClearFileInfo    (dst);
  ClearTrackInfo   (dst);
end;


end.
