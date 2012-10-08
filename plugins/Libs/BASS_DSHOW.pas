unit BASS_DSHOW;
{
  BASS_DSHOW 2.4 Delphi unit
  Copyright (c) 2009-2010 Cristea Aurel Ionut.
}

interface

uses
  Windows,dynamic_bass;

const
  {BASS_DSHOW Plugin CLSID}
  CLSID_DSHOWPLUGIN: TGUID = '{00000000-0000-0000-0000-000000000000}';
  BASS_DSHOW_VERSION = $20401;             // API version
  BASS_DSHOW_VERSIONTEXT = '2.4.1';         //TEXT version

type
  HENCODE = DWORD;
  HWINDOW = DWORD;
  HRECORD = DWORD;

//for Mix_StreamCreate function
  TMixingFiles = array[0..15] of PCHAR;

/////////////CALLBACKS///////////////////
///
  TCallBackEnumEncoderFilter = function(Filter : Pointer; FilterName: PChar) : BOOL; stdcall;
  TCallBackConnectedFilters  = function(Filter : Pointer; FilterName: PChar;pp:BOOL;user:pointer) : BOOL; stdcall;
  TCallBackEnumDevices       = function(device: PChar;user:Pointer) : BOOL; stdcall;
 /////////////////////////////////////////

//for BASS_DSHOW_ChannelGetInfo function
  PBASS_DSVIDEOINFO= ^TBASS_DSVIDEOINFO;
  TBASS_DSVIDEOINFO = record
    AvgTimePerFrame : Double;
    Height, Width : integer;
  end;
//for BASS_DSHOW_ChannelSetConfig function
  PTTextOverlayStruct = ^TTextOverlayStruct;
  TTextOverlayStruct = record
    x:     integer;       //x position
    y:     integer;       //y position
    red:   integer;
    green: integer;
    blue : integer;
  end;

  PTVideoColors = ^TVideoColors;
  TVideoColors=record
    HUE: integer;            //-180...180
    Contrast: integer;       //0...128
    Brightness: integer;     //-128...128.
    Saturation: integer;     //0...128
  end;

const                    /////flags
  DLLNAME                       =   'BASS_DSHOW.DLL';
  BASS_DSHOW_DECODE             =   BASS_STREAM_DECODE;

//for BASS_DSHOW_SetConfig function
  DSHOW_VMRWINDOW               =    95;         //VMR need an initial window so set a HWND to use properly VMR
  BASS_DSHOW_VideoRenderer      =    96;
  BASS_DSHOW_USEDefault         =    97;          //pass this to select default video render
  BASS_DSHOW_USEOverlay         =    98;          //pass this to select overlay video render
  BASS_DSHOW_USEVMR             =    99;          //pass this to setconfig option to turn on/off VMR
//for BASS_DSHOW_DVDSetOption
  DVD_TITLE                     =   100;
  DVD_ROOT                      =   101;          //go to DVD root
  DVD_NEXTCHAPTER               =   102;          //go to dvd next chapter
  DVD_PREVCHAPTER               =   103;          //go to dvd previous chapter
  BD_ShowVideoWindow            =   1001;         //set this to show/hide video

// BASS_DSHOW_ChannelSetOption function flags
  DSHOW_Overlay                 =   1002;
  DSHOW_OverlayText             =   1003;
  DSHOW_OverlayProp             =   1004;
  DSHOW_AVSync                  =   1005;
  DSHOW_CONFIG_PITCH            =   1007;
  DSHOW_CheckChannel            =   1009;         //for sync with a channel when first is a decoded one
  DSHOW_4p3                     =   1010;
  DSHOW_16p9                    =   1011;
  DSHOW_AspectRatio             =   1012;
  DSHOW_GetBitmap               =   1013;
  DSHOW_VideoColors             =   1014;
  DSHOW_EnablePitch             =   1015; //2.4.1
////////MIX FLAGS//////////////////////
  BASS_DSHOW_MixRect           =    2000;
  BASS_DSHOW_MixAlpha          =    2001;

//ERROR CODES

  BASS_DSHOW_OK                 =    104;          //all is ok
  BASS_DSHOW_INVALIDCHAN        =    113;          //invalid channel
  BASS_DSHOW_BADFILENAME        =    105;
  BASS_DSHOW_Unknown            =    106;
  BASS_DSHOW_ERROR1             =    107;  //this is returned by set dvd menu function
  BASS_DSHOW_ERROR2             =    108;  // next chapter failed
  BASS_DSHOW_ERROR3             =    109; //prev chapter failed
  BASS_DSHOW_ERROR4             =    110;  // title menu failed
  BASS_DSHOW_ERROR5             =    111;  //graph creation failed
  BASS_DSHOW_ERROR6             =    112;  //DVD Graph creation failed
  BASS_DSHOW_ERROR7             =    114;
  BASS_DSHOW_ERROR8             =    115; //NO DVD Decoder found

//Converter Flags///
  Convert_EncoderVideo          =  3000;
  Convert_EncoderAudio          =  3001;
  Convert_AudioCompressor       =  3002;
  Convert_VideoCompressor       =  3003;

  Convert_DisableAudio          =  3005; //convert only audio. Disables video
//Profiles
  Convert_ToAvi                 =  3007; //convert to avi
  Convert_ToWMV                 =  3008; //convert to WMV
  Convert_ToWAV                 =  3009; //convert to WAV

///Recorder Flags///
  Record_AudioDevice            =  5000;
  Record_VideoDevice            =  5001;

///
///
var BASS_DSHOW_StreamCreateURL :function(str: PCHAR;flags: DWORD): HSTREAM; stdcall;
var BASS_DSHOW_StreamCreateFile:function(str: PCHAR;flags: DWORD): HSTREAM; stdcall;
var BASS_DSHOW_StreamFree      :function(chan: HStream): bool; stdcall;
var BASS_DSHOW_StreamCreateDVD :function():HSTREAM; stdcall;

var BASS_DSHOW_Init:function(handle: HWND):bool; stdcall;
var BASS_DSHOW_Free:function(): BOOL; stdcall;

var BASS_DSHOW_ChannelSetPosition        :procedure(chan: HSTREAM;pos: QWORD); stdcall;
var BASS_DSHOW_ChannelGetLength          :function (chan: HSTREAM): QWORD; stdcall;
var BASS_DSHOW_ChannelGetPosition        :function (chan: HSTREAM): QWORD; stdcall;
var BASS_DSHOW_ChannelSetWindow          :procedure(chan: HSTREAM;handle: HWND); stdcall;
var BASS_DSHOW_ChannelResizeWindow       :procedure(chan: HSTREAM;left,top,right,bottom: integer); stdcall;
var BASS_DSHOW_ChannelSetFullscreen      :procedure(chan: HSTREAM;value: boolean); stdcall;
var BASS_DSHOW_ChannelPlay               :function (chan: HSTREAM):bool; stdcall;
var BASS_DSHOW_ChannelPause              :function (chan: HSTREAM):bool; stdcall;
var BASS_DSHOW_ChannelStop               :function (chan: HStream): bool; stdcall;
var BASS_DSHOW_ChannelGetInfo            :procedure(chan: HSTREAM;value: PBASS_DSVIDEOINFO);stdcall;
var BASS_DSHOW_ChannelSetOption          :procedure(chan:HSTREAM;option:DWORD;value:DWORD;value2: pointer); stdcall;
var BASS_DSHOW_ChannelGetConnectedFilters:procedure(chan: HSTREAM;callback :Pointer;user:Pointer); stdcall; //2.4.1
var BASS_DSHOW_ChannelSetTextOverlay     :procedure(chan: HSTREAM ;text:PCHAR;x, y, red, green, blue: integer); stdcall;
var BASS_DSHOW_ChannelAddWindow          :function(chan:HSTREAM;win:HWND): HWINDOW; stdcall;

var BASS_DSHOW_DVDSetOption:function(chan: HStream;option: DWORD): bool; stdcall;
var BASS_DSHOW_SetConfig   :procedure(config: integer;value: integer); stdcall;
var BASS_DSHOW_ErrorGetCode:function(): DWORD; stdcall;
var BASS_DSHOW_LoadPlugin  :procedure(str: pchar;guid :TGUID;name: PCHAR); stdcall;
var BASS_DSHOW_LoadPlugin2 :procedure(str: Pointer;guid :Pointer;name: Pointer;flags: DWORD); stdcall;
var BASS_DSHOW_GetVersion  :function(): DWORD; stdcall;

var BASS_DSHOW_ShowFilterPropertyPage:procedure(chan:HSTREAM;filter:DWORD;hndparent: HWND); stdcall; //2.4.1
var BASS_DSHOW_MIX_StreamCreateFile:function(files: TMixingFiles;fileno:integer;flags: DWORD): HSTREAM; stdcall;
var BASS_DSHOW_MIX_ChanOptions     :function(chan: HSTREAM;option:DWORD;value: DWORD;value2: DWORD;rect: TRECT): BOOL; stdcall;

////////  STILL TEsting encoding//////
var BASS_DSHOW_Encode_GetCodecs   :function(CodecsType:DWORD;callback: Pointer):integer; stdcall;
var BASS_DSHOW_Encode_GetProfiles :function(CodecsType:DWORD;callback: Pointer):integer; stdcall;
var BASS_DSHOW_Encode_StreamCreate:function(inFile: PCHAR;outFile:PChar):  HENCODE; stdcall;
var BASS_DSHOW_Encode_Start       :function(hnd: HENCODE;profile:DWORD;flags: DWORD): BOOL; stdcall;
var BASS_DSHOW_Encode_Stop        :function(hnd: HENCODE): BOOL; stdcall;
var BASS_DSHOW_Encode_GetPosition :function(hnd: HENCODE): DWORD; stdcall;
var BASS_DSHOW_Encode_SetEncoder  :function(hnd:HENCODE;enctype: DWORD;encoder: DWORD): BOOL; stdcall;

///////
var BASS_DSHOW_Record_GetDevices:function(devicetype: DWORD;callback: Pointer;user: Pointer): integer; stdcall;
var BASS_DSHOW_RecordStart      :function(audiodevice: Integer;videodevice: Integer;devicetype: DWORD;flags: DWORD): HRECORD; stdcall;
var BASS_DSHOW_RecordFree       :function(rec: HRECORD): BOOL; stdcall;

implementation
// END OF FILE /////////////////////////////////////////////////////////////////

procedure SetProcs(handle:THANDLE);
begin
  @BASS_DSHOW_StreamCreateURL :=GetProcAddress(handle, 'BASS_DSHOW_StreamCreateURL');
  @BASS_DSHOW_StreamCreateFile:=GetProcAddress(handle, 'BASS_DSHOW_StreamCreateFile');
  @BASS_DSHOW_StreamFree      :=GetProcAddress(handle, 'BASS_DSHOW_StreamFree');
  @BASS_DSHOW_StreamCreateDVD :=GetProcAddress(handle, 'BASS_DSHOW_StreamCreateDVD');

  @BASS_DSHOW_Init:=GetProcAddress(handle, 'BASS_DSHOW_Init');
  @BASS_DSHOW_Free:=GetProcAddress(handle, 'BASS_DSHOW_Free');

  @BASS_DSHOW_ChannelSetPosition        :=GetProcAddress(handle, 'BASS_DSHOW_ChannelSetPosition');
  @BASS_DSHOW_ChannelGetLength          :=GetProcAddress(handle, 'BASS_DSHOW_ChannelGetLength');
  @BASS_DSHOW_ChannelGetPosition        :=GetProcAddress(handle, 'BASS_DSHOW_ChannelGetPosition');
  @BASS_DSHOW_ChannelSetWindow          :=GetProcAddress(handle, 'BASS_DSHOW_ChannelSetWindow');
  @BASS_DSHOW_ChannelResizeWindow       :=GetProcAddress(handle, 'BASS_DSHOW_ChannelResizeWindow');
  @BASS_DSHOW_ChannelSetFullscreen      :=GetProcAddress(handle, 'BASS_DSHOW_ChannelSetFullscreen');
  @BASS_DSHOW_ChannelPlay               :=GetProcAddress(handle, 'BASS_DSHOW_ChannelPlay');
  @BASS_DSHOW_ChannelPause              :=GetProcAddress(handle, 'BASS_DSHOW_ChannelPause');
  @BASS_DSHOW_ChannelStop               :=GetProcAddress(handle, 'BASS_DSHOW_ChannelStop');
  @BASS_DSHOW_ChannelGetInfo            :=GetProcAddress(handle, 'BASS_DSHOW_ChannelGetInfo');
  @BASS_DSHOW_ChannelSetOption          :=GetProcAddress(handle, 'BASS_DSHOW_ChannelSetOption');
  @BASS_DSHOW_ChannelGetConnectedFilters:=GetProcAddress(handle, 'BASS_DSHOW_ChannelGetConnectedFilters');
  @BASS_DSHOW_ChannelSetTextOverlay     :=GetProcAddress(handle, 'BASS_DSHOW_ChannelSetTextOverlay');
  @BASS_DSHOW_ChannelAddWindow          :=GetProcAddress(handle, 'BASS_DSHOW_ChannelAddWindow');

  @BASS_DSHOW_DVDSetOption:=GetProcAddress(handle, 'BASS_DSHOW_DVDSetOption');
  @BASS_DSHOW_SetConfig   :=GetProcAddress(handle, 'BASS_DSHOW_SetConfig');
  @BASS_DSHOW_ErrorGetCode:=GetProcAddress(handle, 'BASS_DSHOW_ErrorGetCode');
  @BASS_DSHOW_LoadPlugin  :=GetProcAddress(handle, 'BASS_DSHOW_LoadPlugin');
  @BASS_DSHOW_LoadPlugin2 :=GetProcAddress(handle, 'BASS_DSHOW_LoadPlugin2');
  @BASS_DSHOW_GetVersion  :=GetProcAddress(handle, 'BASS_DSHOW_GetVersion');

  @BASS_DSHOW_ShowFilterPropertyPage:=GetProcAddress(handle, 'BASS_DSHOW_ShowFilterPropertyPage');
  @BASS_DSHOW_MIX_StreamCreateFile:=GetProcAddress(handle, 'BASS_DSHOW_MIX_StreamCreateFile');
  @BASS_DSHOW_MIX_ChanOptions     :=GetProcAddress(handle, 'BASS_DSHOW_MIX_ChanOptions');

  @BASS_DSHOW_Encode_GetCodecs   :=GetProcAddress(handle, 'BASS_DSHOW_Encode_GetCodecs');
  @BASS_DSHOW_Encode_GetProfiles :=GetProcAddress(handle, 'BASS_DSHOW_Encode_GetProfiles');
  @BASS_DSHOW_Encode_StreamCreate:=GetProcAddress(handle, 'BASS_DSHOW_Encode_StreamCreate');
  @BASS_DSHOW_Encode_Start       :=GetProcAddress(handle, 'BASS_DSHOW_Encode_Start');
  @BASS_DSHOW_Encode_Stop        :=GetProcAddress(handle, 'BASS_DSHOW_Encode_Stop');
  @BASS_DSHOW_Encode_GetPosition :=GetProcAddress(handle, 'BASS_DSHOW_Encode_GetPosition');
  @BASS_DSHOW_Encode_SetEncoder  :=GetProcAddress(handle, 'BASS_DSHOW_Encode_SetEncoder');

  @BASS_DSHOW_Record_GetDevices:=GetProcAddress(handle, 'BASS_DSHOW_Record_GetDevices');
  @BASS_DSHOW_RecordStart      :=GetProcAddress(handle, 'BASS_DSHOW_RecordStart');
  @BASS_DSHOW_RecordFree       :=GetProcAddress(handle, 'BASS_DSHOW_RecordFree');
  
end;

const
  DSHOW_Handle:THANDLE = 0;
  from:integer = 0;

function InitDSHOW:bool;
var
  info:PBASS_PLUGININFO;
  i:dword;
  pHPlugin:^HPLUGIN;
begin
  if DSHOW_Handle<>0 then
  begin
    result:=true;
    exit;
  end;
  result:=false;
  pHPlugin:=pointer(BASS_PluginGetInfo(0));
  if pHPlugin=nil then exit;
  while pHPlugin^<>0 do
  begin
    info:=BASS_PluginGetInfo(pHPlugin^);
    i:=0;
    while i<info^.formatc do
    begin
//!!      if info^.formats^[i].ctype=BASS_CTYPE_STREAM_WMA then
      begin
        DSHOW_Handle:=pHPlugin^;
        SetProcs(pHPlugin^);
        from:=2;
        result:=true;
        exit;
      end;
      inc(i);
    end;
    inc(pHPlugin);
  end;
end;

var
  mDSHOW:tBASSRegRec;

procedure Init;
begin
  mDSHOW.Next:=BASSRegRec;
  mDSHOW.Init:=@InitDSHOW;
  BASSRegRec:=@mDSHOW;
end;

begin
  Init;
end.
