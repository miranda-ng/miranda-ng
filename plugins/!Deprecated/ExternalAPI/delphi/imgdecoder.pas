unit ImgDecoder;

interface
uses Windows;

function IsImgDecoderAvailable:boolean;
function ImgNewDecoder(var pDecoder:pointer):dword;
function ImgDeleteDecoder(pDecoder:pointer):dword;
function ImgNewDIBFromFile(pDecoder:pointer;Filename:PAnsiChar;var pImg:pointer):dword;
function ImgDeleteDIBSection(pImg:pointer):dword;
function ImgGetHandle(pImg:pointer;var pBitmap:HBITMAP;var ppDIBBits:pointer):dword;

implementation

const
  hModule:THANDLE=0;
type
  tImgNewDecoder      =function(var pDecoder:pointer):dword; stdcall;
  tImgDeleteDecoder   =function(pDecoder:pointer):dword; stdcall;
  tImgNewDIBFromFile  =function(pDecoder:pointer;Filename:PAnsiChar;var pImg):dword; stdcall;
  tImgDeleteDIBSection=function(pImg:pointer):dword; stdcall;
  tImgGetHandle       =function(pImg:pointer;var pBitmap:HBITMAP;var ppDIBBits:pointer):dword; stdcall;

const
  pImgNewDecoder      :tImgNewDecoder       = nil;
  pImgDeleteDecoder   :tImgDeleteDecoder    = nil;
  pImgNewDIBFromFile  :tImgNewDIBFromFile   = nil;
  pImgDeleteDIBSection:tImgDeleteDIBSection = nil;
  pImgGetHandle       :tImgGetHandle        = nil;

function IsImgDecoderAvailable:boolean;
begin
  result:=hModule<>0;
end;

function ImgNewDecoder(var pDecoder:pointer):dword;
begin
  if @pImgNewDecoder<>nil then
    result:=pImgNewDecoder(pDecoder)
  else
    result:=0;
end;

function ImgDeleteDecoder(pDecoder:pointer):dword;
begin
  if @pImgDeleteDecoder<>nil then
    result:=pImgDeleteDecoder(pDecoder)
  else
    result:=0;
end;

function ImgNewDIBFromFile(pDecoder:pointer;Filename:PAnsiChar;var pImg:pointer):dword;
begin
  if @pImgNewDecoder<>nil then
    result:=pImgNewDIBFromFile(pDecoder,Filename,pImg)
  else
    result:=0;
end;

function ImgDeleteDIBSection(pImg:pointer):dword;
begin
  if @pImgNewDecoder<>nil then
    result:=pImgDeleteDIBSection(pImg)
  else
    result:=0;
end;

function ImgGetHandle(pImg:pointer;var pBitmap:HBITMAP;var ppDIBBits:pointer):dword;
begin
  if @pImgGetHandle<>nil then
    result:=pImgGetHandle(pImg,pBitmap,ppDIBBits)
  else
    result:=0;
end;

initialization
  hModule:=LoadLibrary('imgdecoder.dll');
  if hModule=0 then
    hModule:=LoadLibrary('plugins\imgdecoder.dll');

  if hModule<>0 then
  begin
    pImgNewDecoder      :=GetProcAddress(hModule, 'ImgNewDecoder');
    pImgDeleteDecoder   :=GetProcAddress(hModule, 'ImgDeleteDecoder');
    pImgNewDIBFromFile  :=GetProcAddress(hModule, 'ImgNewDIBFromFile');
    pImgDeleteDIBSection:=GetProcAddress(hModule, 'ImgDeleteDIBSection');
    pImgGetHandle       :=GetProcAddress(hModule, 'ImgGetHandle');
  end;

finalization
  if hModule<>0 then
    FreeLibrary(hModule);
end.
{ Sample of using

var
  pDecoder:pointer;
  pImg:pointer;
  bitmap:HBITMAP;
  pBits:pointer;
begin
  if IsImgDecoderAvailable then
  begin
    ImgNewDecoder(pDecoder);
    if ImgNewDIBFromFile(pDecoder,PAnsiChar(ParamStr(1)),pImg)<>0 then
    begin
      ImgGetHandle(pImg,bitmap,pBits);
// action
      ImgDeleteDIBSection(pImg);
    end;
    ImgDeleteDecoder(pDecoder);
  end;
end.
}
