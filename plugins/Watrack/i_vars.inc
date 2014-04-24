{variables}
var
  SongInfoA:tSongInfoA;
  SongInfo :tSongInfo;
  WorkSI   :tSongInfo;
var
  hEvent:THANDLE;
  hHookWATStatus,
  hHookWATLoaded:THANDLE;
  inshotkey:cardinal;
  globhotkey:cardinal;
  hTimer:cardinal;
  ttbState,
  hMenuDisable:THANDLE;
var
  CoverPaths:PWideChar;
  KeepOld,
  UseImplant,
  CheckAll,
  CheckUnknown,
  CheckTime:dword;
  mmkeyemu: dword;
  mTimer:dword;
