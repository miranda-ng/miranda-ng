{
  BASSWMA 2.4 Delphi unit
  Copyright (c) 2002-2008 Un4seen Developments Ltd.

  See the BASSWMA.CHM file for more detailed documentation
}

unit DynBassWMA;

interface

uses Windows, Dynamic_Bass;

const
  // Additional error codes returned by BASS_ErrorGetCode
  BASS_ERROR_WMA_LICENSE     = 1000; // the file is protected
  BASS_ERROR_WMA             = 1001; // Windows Media (9 or above) is not installed
  BASS_ERROR_WMA_WM9         = BASS_ERROR_WMA;
  BASS_ERROR_WMA_DENIED      = 1002; // access denied (user/pass is invalid)
  BASS_ERROR_WMA_INDIVIDUAL  = 1004; // individualization is needed

  // Additional BASS_SetConfig options
  BASS_CONFIG_WMA_PRECHECK   = $10100;
  BASS_CONFIG_WMA_PREBUF     = $10101;
//  BASS_CONFIG_WMA_ASX        = $10102;
  BASS_CONFIG_WMA_BASSFILE   = $10103;
  BASS_CONFIG_WMA_NETSEEK    = $10104;
  BASS_CONFIG_WMA_VIDEO      = $10105;

  // additional WMA sync types
  BASS_SYNC_WMA_CHANGE       = $10100;
  BASS_SYNC_WMA_META         = $10101;

  // additional BASS_StreamGetFilePosition WMA mode
  BASS_FILEPOS_WMA_BUFFER    = 1000; // internet buffering progress (0-100%)

  // Additional flags for use with BASS_WMA_EncodeOpen/File/Network/Publish
  BASS_WMA_ENCODE_STANDARD   = $2000;  // standard WMA
  BASS_WMA_ENCODE_PRO        = $4000;  // WMA Pro
  BASS_WMA_ENCODE_24BIT      = $8000;  // 24-bit
  BASS_WMA_ENCODE_PCM        = $10000; // uncompressed PCM
  BASS_WMA_ENCODE_SCRIPT     = $20000; // set script (mid-stream tags) in the WMA encoding

  // Additional flag for use with BASS_WMA_EncodeGetRates
  BASS_WMA_ENCODE_RATES_VBR  = $10000; // get available VBR quality settings

  // WMENCODEPROC "type" values
  BASS_WMA_ENCODE_HEAD       = 0;
  BASS_WMA_ENCODE_DATA       = 1;
  BASS_WMA_ENCODE_DONE       = 2;

  // BASS_WMA_EncodeSetTag "form" values
  BASS_WMA_TAG_ANSI          = 0;
  BASS_WMA_TAG_UNICODE       = 1;
  BASS_WMA_TAG_UTF8          = 2;
  BASS_WMA_TAG_BINARY        = $100; // FLAG: binary tag (HIWORD=length)

  // BASS_CHANNELINFO type
  BASS_CTYPE_STREAM_WMA      = $10300;
  BASS_CTYPE_STREAM_WMA_MP3  = $10301;

  // Additional BASS_ChannelGetTags type
  BASS_TAG_WMA               = 8;  // WMA header tags : series of null-terminated UTF-8 strings
  BASS_TAG_WMA_META          = 11; // WMA mid-stream tag : UTF-8 string
  BASS_TAG_WMA_CODEC         = 12; // WMA codec


type
  HWMENCODE = DWORD;		// WMA encoding handle

  CLIENTCONNECTPROC = procedure(handle:HWMENCODE; connect:BOOL; ip:PChar; user:Pointer); stdcall;
  {
    Client connection notification callback function.
    handle : The encoder
    connect: TRUE=client is connecting, FALSE=disconnecting
    ip     : The client's IP (xxx.xxx.xxx.xxx:port)
    user   : The 'user' parameter value given when calling BASS_WMA_EncodeSetNotify
  }

  WMENCODEPROC = procedure(handle:HWMENCODE; dtype:DWORD; buffer:Pointer; length:DWORD; user:Pointer); stdcall;
  {
    Encoder callback function.
    handle : The encoder handle
    dtype  : The type of data, one of BASS_WMA_ENCODE_xxx values
    buffer : The encoded data
    length : Length of the data
    user   : The 'user' parameter value given when calling BASS_WMA_EncodeOpen
  }


const
  basswmadll = 'basswma.dll';

var BASS_WMA_StreamCreateFile      :function(mem:BOOL; fl:pointer; offset,length:QWORD; flags:DWORD): HSTREAM; stdcall;
var BASS_WMA_StreamCreateFileAuth  :function(mem:BOOL; fl:pointer; offset,length:QWORD; flags:DWORD; user,pass:PChar): HSTREAM; stdcall;
var BASS_WMA_StreamCreateFileUser  :function(system,flags:DWORD; var procs:BASS_FILEPROCS; user:Pointer): HSTREAM; stdcall;

var BASS_WMA_GetTags               :function(fname:PChar; flags:DWORD): PAnsiChar; stdcall;

var BASS_WMA_EncodeGetRates        :function(freq,chans,flags:DWORD): PDWORD; stdcall;
var BASS_WMA_EncodeOpen            :function(freq,chans,flags,bitrate:DWORD; proc:WMENCODEPROC; user:Pointer): HWMENCODE; stdcall;
var BASS_WMA_EncodeOpenFile        :function(freq,chans,flags,bitrate:DWORD; fname:PChar): HWMENCODE; stdcall;
var BASS_WMA_EncodeOpenNetwork     :function(freq,chans,flags,bitrate,port,clients:DWORD): HWMENCODE; stdcall;
var BASS_WMA_EncodeOpenNetworkMulti:function(freq,chans,flags:DWORD; bitrates:PDWORD; port,clients:DWORD): HWMENCODE; stdcall;
var BASS_WMA_EncodeOpenPublish     :function(freq,chans,flags,bitrate:DWORD; url,user,pass:PChar): HWMENCODE; stdcall;
var BASS_WMA_EncodeOpenPublishMulti:function(freq,chans,flags:DWORD; bitrates:PDWORD; url,user,pass:PChar): HWMENCODE; stdcall;
var BASS_WMA_EncodeGetPort         :function(handle:HWMENCODE): DWORD; stdcall;
var BASS_WMA_EncodeSetNotify       :function(handle:HWMENCODE; proc:CLIENTCONNECTPROC; user:Pointer): BOOL; stdcall;
var BASS_WMA_EncodeGetClients      :function(handle:HWMENCODE): DWORD; stdcall;
var BASS_WMA_EncodeSetTag          :function(handle:HWMENCODE; tag,text:PChar; ttype:DWORD): BOOL; stdcall;
var BASS_WMA_EncodeWrite           :function(handle:HWMENCODE; buffer:Pointer; length:DWORD): BOOL; stdcall;
var BASS_WMA_EncodeClose           :function(handle:HWMENCODE): BOOL; stdcall;

var BASS_WMA_GetWMObject           :function(handle:DWORD): Pointer; stdcall;

function InitWMA:bool;
Function Load_WMADLL(dllfilename:PAnsiChar):boolean; overload;
Function Load_WMADLL(dllfilename:PWideChar):boolean; overload;

implementation

const
  WMA_Handle:THANDLE = 0;
  from:integer = 0;

procedure SetProcs(handle:THANDLE);
begin
  @BASS_WMA_StreamCreateFile      :=GetProcAddress(handle,'BASS_WMA_StreamCreateFile');
  @BASS_WMA_StreamCreateFileAuth  :=GetProcAddress(handle,'BASS_WMA_StreamCreateFileAuth');
  @BASS_WMA_StreamCreateFileUser  :=GetProcAddress(handle,'BASS_WMA_StreamCreateFileUser');

  @BASS_WMA_GetTags               :=GetProcAddress(handle,'BASS_WMA_GetTags');

  @BASS_WMA_EncodeGetRates        :=GetProcAddress(handle,'BASS_WMA_EncodeGetRates');
  @BASS_WMA_EncodeOpen            :=GetProcAddress(handle,'BASS_WMA_EncodeGetOpen');
  @BASS_WMA_EncodeOpenFile        :=GetProcAddress(handle,'BASS_WMA_EncodeOpenFile');
  @BASS_WMA_EncodeOpenNetwork     :=GetProcAddress(handle,'BASS_WMA_EncodeOpenNetwork');
  @BASS_WMA_EncodeOpenNetworkMulti:=GetProcAddress(handle,'BASS_WMA_EncodeOpenNetworkMulti');
  @BASS_WMA_EncodeOpenPublish     :=GetProcAddress(handle,'BASS_WMA_EncodeOpenPublish');
  @BASS_WMA_EncodeOpenPublishMulti:=GetProcAddress(handle,'BASS_WMA_EncodeOpenPublishMulti');
  @BASS_WMA_EncodeGetPort         :=GetProcAddress(handle,'BASS_WMA_EncodeGetPort');
  @BASS_WMA_EncodeSetNotify       :=GetProcAddress(handle,'BASS_WMA_EncodeSetNotify');
  @BASS_WMA_EncodeGetClients      :=GetProcAddress(handle,'BASS_WMA_EncodeGetClients');
  @BASS_WMA_EncodeSetTag          :=GetProcAddress(handle,'BASS_WMA_EncodeSetTag');
  @BASS_WMA_EncodeWrite           :=GetProcAddress(handle,'BASS_WMA_EncodeWrite');
  @BASS_WMA_EncodeClose           :=GetProcAddress(handle,'BASS_WMA_EncodeClose');

  @BASS_WMA_GetWMObject:=GetProcAddress(handle,'BASS_WMA_GetWMObject');
end;

function InitWMA:bool;
var
  info:PBASS_PLUGININFO;
  i:dword;
  pHPlugin:^HPLUGIN;
begin
  if WMA_Handle<>0 then
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
      if info^.formats^[i].ctype=BASS_CTYPE_STREAM_WMA then
      begin
        WMA_Handle:=pHPlugin^;
        SetProcs(pHPlugin^);
        BASS_SetConfig(BASS_CONFIG_WMA_BASSFILE,1);
        from:=2;
        result:=true;
        exit;
      end;
      inc(i);
    end;
    inc(pHPlugin);
  end;
end;

Function Load_WMADLL(dllfilename:PAnsiChar):boolean;
var
  oldmode:integer;
begin
  if WMA_Handle<>0 then result:=true
  else
  begin
    oldmode:=SetErrorMode($8001);
    WMA_Handle:=LoadLibraryA(dllfilename);
    SetErrorMode(oldmode);
    result:=WMA_Handle<>0;
    if result then
    begin
      from:=1;
      SetProcs(WMA_Handle);
    end;
  end;
end;

Function Load_WMADLL(dllfilename:PWideChar):boolean;
var
  oldmode:integer;
begin
  if WMA_Handle<>0 then result:=true
  else
  begin
    oldmode:=SetErrorMode($8001);
    WMA_Handle:=LoadLibraryW(dllfilename);
    SetErrorMode(oldmode);
    result:=WMA_Handle<>0;
    if result then
    begin
      from:=1;
      SetProcs(WMA_Handle);
    end;
  end;
end;

Procedure Unload_WMADLL;
begin
  if WMA_Handle<>0 then
  begin
    if from=2 then
      BASS_PluginFree(WMA_Handle)
    else //if from=1 then
      FreeLibrary(WMA_Handle);
    WMA_Handle:=0;
  end;
  from:=0;
end;

var
  mWMA:tBASSRegRec;

procedure Init;
begin
  mWMA.Next:=BASSRegRec;
  mWMA.Init:=@InitWMA;
  BASSRegRec:=@mWMA;
end;

begin
  Init;
end.
