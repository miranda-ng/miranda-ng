{variables}
var
  SongInfoA:tSongInfoA;
  SongInfo :tSongInfo;
  WorkSI   :tSongInfo;
var
  hEvent:THANDLE;
  hGFI,
  hWI,
  hGMI,
  hPS,
  hPB,
  hWATI,
  hWC,
  hFMT,
  hPLR,
  hRGS,
  hHookWATLoaded:THANDLE;
  inshotkey:cardinal;
  globhotkey:cardinal;
  hTimer:cardinal;
  ttbState,
  hMenuDisable:THANDLE;
var
  CoverPaths:PWideChar;
  MTHCheck,
  KeepOld,
  UseImplant,
  CheckAll,
  CheckTime:dword;
  mmkeyemu: dword;
  mTimer:dword;
  TimeoutForThread:cardinal;
