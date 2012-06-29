unit shlicons;

interface

uses
  Windows;

type

  PVTable_IWICBitmap = ^TVTable_IWICBitmap;

  TVTable_IWICBitmap = record
    { IUnknown }
    QueryInterface: Pointer;
    AddRef: function(Self: Pointer): Cardinal; stdcall;
    Release: function(Self: Pointer): Cardinal; stdcall;
    { IWICBitmapSource }
    GetSize: function(Self: Pointer; var Width, Height: LongInt): HResult; stdcall;
    GetPixelFormat: Pointer;
    GetResolution: Pointer;
    CopyPalette: Pointer;
    CopyPixels: function(Self: Pointer; prc: Pointer; cbStride, cbBufferSize: LongWord;
      pbBuffer: PByte): HResult; stdcall;
    { IWICBitmap }
    // .... not used

  end;

  PWICBitmap_Interface = ^TWICBitmap_Interface;

  TWICBitmap_Interface = record
    ptrVTable: PVTable_IWICBitmap;
  end;

  // bare minmum interface to ImagingFactory

  PVTable_ImagingFactory = ^TVTable_ImagingFactory;

  TVTable_ImagingFactory = record
    { IUnknown }
    QueryInterface: Pointer;
    AddRef: function(Self: Pointer): Cardinal; stdcall;
    Release: function(Self: Pointer): Cardinal; stdcall;
    { ImagingFactory }
    CreateDecoderFromFilename: Pointer;
    CreateDecoderFromStream: Pointer;
    CreateDecoderFromFileHandle: Pointer;
    CreateComponentInfo: Pointer;
    CreateDecoder: Pointer;
    CreateEncoder: Pointer;
    CreatePalette: Pointer;
    CreateFormatConverter: Pointer;
    CreateBitmapScaler: Pointer;
    CreateBitmapClipper: Pointer;
    CreateBitmapFlipRotator: Pointer;
    CreateStream: Pointer;
    CreateColorContext: Pointer;
    CreateColorTransformer: Pointer;
    CreateBitmap: Pointer;
    CreateBitmapFromSource: Pointer;
    CreateBitmapFromSourceRect: Pointer;
    CreateBitmapFromMemory: Pointer;
    CreateBitmapFromHBITMAP: Pointer;
    CreateBitmapFromHICON: function(Self: Pointer; hIcon: Windows.hIcon; var foo: Pointer)
      : HResult; stdcall;
    { rest ommited }
  end;

  PImageFactory_Interface = ^TImageFactory_Interface;

  TImageFactory_Interface = record
    ptrVTable: PVTable_ImagingFactory;
  end;

function ARGB_GetWorker: PImageFactory_Interface;

function ARGB_BitmapFromIcon(Factory: PImageFactory_Interface; hdc: Windows.hdc; hIcon: hIcon): HBitmap;

implementation

{$DEFINE SHLCOM}
{$DEFINE COM_STRUCTS}
{$DEFINE COMAPI}
{$INCLUDE shlc.inc}
{$UNDEF SHLCOM}
{$UNDEF COM_STRUCTS}
{$UNDEF COMAPI}
{
  The following implementation has been ported from:

  http://web.archive.org/web/20080121112802/http://shellrevealed.com/blogs/shellblog/archive/2007/02/06/Vista-Style-Menus_2C00_-Part-1-_2D00_-Adding-icons-to-standard-menus.aspx

  It uses WIC (Windows Imaging Codec) to convert the given Icon into a bitmap in ARGB format, this is required
  by Windows for use as an icon (but in bitmap format), so that Windows draws everything (including theme)
  so we don't have to.

  Why didn't they just do this themselves? ...
}

{
  The object returned from this function has to be released using the QI COM interface, don't forget.
  Note this function won't work on anything where WIC isn't installed (XP can have it installed, but not by default)
  anything less won't work.
}
function ARGB_GetWorker: PImageFactory_Interface;
var
  hr: HResult;
begin
  hr := CoCreateInstance(CLSID_WICImagingFactory, nil, CLSCTX_INPROC_SERVER,
    IID_WICImagingFactory, Result);
end;

function ARGB_BitmapFromIcon(Factory: PImageFactory_Interface; hdc: Windows.hdc; hIcon: hIcon): HBitmap;
var
  bmi: BITMAPINFO;
  hr: HResult;
  bitmap: PWICBitmap_Interface;
  cx, cy: LongInt;
  pbBuffer: PByte;
  hBmp: HBitmap;
  cbStride, cbBuffer: LongInt;
begin
  { This code gives an icon to WIC and gets a bitmap object in return, it then creates a DIB section
    which is 32bits and the same H*W as the icon. It then asks the bitmap object to copy itself into the DIB }
  Result := 0;
  ZeroMemory(@bmi, sizeof(bmi));
  bmi.bmiHeader.biSize := sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biPlanes := 1;
  bmi.bmiHeader.biCompression := BI_RGB;

  bmi.bmiHeader.biBitCount := 32;

  hr := Factory^.ptrVTable^.CreateBitmapFromHICON(Factory, hIcon, pointer(bitmap));
  if hr = S_OK then
  begin
    hr := bitmap^.ptrVTable^.GetSize(bitmap, cx, cy);
    if hr = S_OK then
    begin

      bmi.bmiHeader.biWidth := cx;
      bmi.bmiHeader.biHeight := -cy;

      hBmp := CreateDIBSection(hdc, bmi, DIB_RGB_COLORS, pointer(pbBuffer), 0, 0);
      if hBmp <> 0 then
      begin
        cbStride := cx * sizeof(DWORD); // ARGB = DWORD
        cbBuffer := cy * cbStride;
        // note: the pbBuffer memory is owned by the DIB and will be freed when the bitmap is released
        hr := bitmap^.ptrVTable^.CopyPixels(bitmap, nil, cbStride, cbBuffer, pbBuffer);
        if hr = S_OK then
        begin
          Result := hBmp;
        end
        else
        begin
          // the copy failed, delete the DIB
          DeleteObject(hBmp);
        end;
      end;
    end;
    // release the bitmap object now
    bitmap^.ptrVTable^.Release(bitmap);
    bitmap := nil;
  end;

end;

end.
