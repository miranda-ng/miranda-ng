{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2007 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

-----------------------------------------------------------------------
Miranda Image services plugin / API definitions
Provides various services for image loading, saving and manipulations. 

This module is based on the freeimage library, copyrighted by the FreeImage
Project members.

Miranda plugin code (c) 2007 by Nightwish, silvercircle@gmail.com, all else (C)
by the FreeImage project (http://freeimage.sourceforge.net)
}

{$IFNDEF M_IMGSRVC}
{$DEFINE M_IMGSRVC}

{.$include m_freeimage.inc}
//#define _FI_MIMPLUGIN 1

const
  FI_IF_VERSION = $00000100; // interface version - must match

// memory i/o defs

{
  this struct defines a memio job.
  datalen and filename must match and must be populated to the size of the memory buffer (caution)
  data must point to the buffer.
  curpos is internal and should be initialized to 0
}

type
  fiio_mem_handle = record
    filelen,
    datalen,
    curpos:cardinal;
    data:pointer;
  end;
  fiio_mem_handle_s = fiio_mem_handle;
{
  it is up to the user to create a fiio_mem_handle and init datalen and data
  filelen will be pre-set to 0 by SaveToMem
  curpos will be pre-set to 0 by SaveToMem and LoadFromMem
  IMPORTANT: data should be set to NULL and datalen to 0,
             unless the user wants to manually malloc a larger buffer
}
(*
FIBITMAP *FreeImage_LoadFromMem(FREE_IMAGE_FORMAT fif, fiio_mem_handle *handle, int flags);
bool FreeImage_SaveToMem(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, fiio_mem_handle *handle, int flags);

void SetMemIO(FreeImageIO *io);
unsigned __stdcall fiio_mem_ReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle);
unsigned __stdcall fiio_mem_WriteProc(void *buffer, unsigned size, unsigned count, fi_handle handle);
int __stdcall fiio_mem_SeekProc(fi_handle handle, long offset, int origin);
long __stdcall fiio_mem_TellProc(fi_handle handle);
*)
{
  this interface directly exports most of FreeImage routines
  you can use them in your plugin after obtaining the interfasce using MS_IMG_GETINTERFACE
}
(*
typedef struct _tagFI_interface {

    DWORD version;

    FIBITMAP *(DLL_CALLCONV *FI_Allocate)(int width, int height, int bpp, unsigned red_mask FI_DEFAULT(0), unsigned green_mask FI_DEFAULT(0), unsigned blue_mask FI_DEFAULT(0));
    FIBITMAP *(DLL_CALLCONV *FI__AllocateT)(FREE_IMAGE_TYPE type, int width, int height, int bpp FI_DEFAULT(8), unsigned red_mask FI_DEFAULT(0), unsigned green_mask FI_DEFAULT(0), unsigned blue_mask FI_DEFAULT(0));
    FIBITMAP *(DLL_CALLCONV *FI_Clone)(FIBITMAP *dib);
    void (DLL_CALLCONV *FI_Unload)(FIBITMAP *dib);
    
    // Load / Save routines -----------------------------------------------------
    
    FIBITMAP *(DLL_CALLCONV *FI_Load)(FREE_IMAGE_FORMAT fif, const AnsiChar *filename, int flags FI_DEFAULT(0));
    FIBITMAP *(DLL_CALLCONV *FI_LoadU)(FREE_IMAGE_FORMAT fif, const wchar_t *filename, int flags FI_DEFAULT(0));
    FIBITMAP *(DLL_CALLCONV *FI_LoadFromHandle)(FREE_IMAGE_FORMAT fif, FreeImageIO *io, fi_handle handle, int flags FI_DEFAULT(0));
    bool (DLL_CALLCONV *FI_Save)(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, const AnsiChar *filename, int flags FI_DEFAULT(0));
    bool (DLL_CALLCONV *FI_SaveU)(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, const wchar_t *filename, int flags FI_DEFAULT(0));
    bool (DLL_CALLCONV *FI_SaveToHandle)(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, FreeImageIO *io, fi_handle handle, int flags FI_DEFAULT(0));

// Memory I/O stream routines -----------------------------------------------

    FIMEMORY *(DLL_CALLCONV *FI_OpenMemory)(byte *data FI_DEFAULT(0), DWORD size_in_bytes FI_DEFAULT(0));
    void (DLL_CALLCONV *FI_CloseMemory)(FIMEMORY *stream);
    FIBITMAP *(DLL_CALLCONV *FI_LoadFromMemory)(FREE_IMAGE_FORMAT fif, FIMEMORY *stream, int flags FI_DEFAULT(0));
    bool (DLL_CALLCONV *FI_SaveToMemory)(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, FIMEMORY *stream, int flags FI_DEFAULT(0));
    long (DLL_CALLCONV *FI_TellMemory)(FIMEMORY *stream);
    bool (DLL_CALLCONV *FI_SeekMemory)(FIMEMORY *stream, long offset, int origin);
    bool (DLL_CALLCONV *FI_AcquireMemory)(FIMEMORY *stream, byte **data, DWORD *size_in_bytes);
    unsigned (DLL_CALLCONV *FI_ReadMemory)(void *buffer, unsigned size, unsigned count, FIMEMORY *stream);
    unsigned (DLL_CALLCONV *FI_WriteMemory)(const void *buffer, unsigned size, unsigned count, FIMEMORY *stream);
    FIMULTIBITMAP *(DLL_CALLCONV *FI_LoadMultiBitmapFromMemory)(FREE_IMAGE_FORMAT fif, FIMEMORY *stream, int flags FI_DEFAULT(0));

// Plugin Interface ---------------------------------------------------------

    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_RegisterLocalPlugin)(FI_InitProc proc_address, const AnsiChar *format FI_DEFAULT(0), const AnsiChar *description FI_DEFAULT(0), const AnsiChar *extension FI_DEFAULT(0), const AnsiChar *regexpr FI_DEFAULT(0));
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_RegisterExternalPlugin)(const AnsiChar *path, const AnsiChar *format FI_DEFAULT(0), const AnsiChar *description FI_DEFAULT(0), const AnsiChar *extension FI_DEFAULT(0), const AnsiChar *regexpr FI_DEFAULT(0));
    int (DLL_CALLCONV *FI_GetFIFCount)(void);
    int (DLL_CALLCONV *FI_SetPluginEnabled)(FREE_IMAGE_FORMAT fif, bool enable);
    int (DLL_CALLCONV *FI_IsPluginEnabled)(FREE_IMAGE_FORMAT fif);
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFIFFromFormat)(const AnsiChar *format);
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFIFFromMime)(const AnsiChar *mime);
    const AnsiChar *(DLL_CALLCONV *FI_GetFormatFromFIF)(FREE_IMAGE_FORMAT fif);
    const AnsiChar *(DLL_CALLCONV *FI_GetFIFExtensionList)(FREE_IMAGE_FORMAT fif);
    const AnsiChar *(DLL_CALLCONV *FI_GetFIFDescription)(FREE_IMAGE_FORMAT fif);
    const AnsiChar *(DLL_CALLCONV *FI_GetFIFRegExpr)(FREE_IMAGE_FORMAT fif);
    const AnsiChar *(DLL_CALLCONV *FI_GetFIFMimeType)(FREE_IMAGE_FORMAT fif);
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFIFFromFilename)(const AnsiChar *filename);
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFIFFromFilenameU)(const wchar_t *filename);
    bool (DLL_CALLCONV *FI_FIFSupportsReading)(FREE_IMAGE_FORMAT fif);
    bool (DLL_CALLCONV *FI_FIFSupportsWriting)(FREE_IMAGE_FORMAT fif);
    bool (DLL_CALLCONV *FI_FIFSupportsExportBPP)(FREE_IMAGE_FORMAT fif, int bpp);
    bool (DLL_CALLCONV *FI_FIFSupportsExportType)(FREE_IMAGE_FORMAT fif, FREE_IMAGE_TYPE type);
    bool (DLL_CALLCONV *FI_FIFSupportsICCProfiles)(FREE_IMAGE_FORMAT fif);

// Multipaging interface ----------------------------------------------------

    FIMULTIBITMAP *(DLL_CALLCONV *FI_OpenMultiBitmap)(FREE_IMAGE_FORMAT fif, const AnsiChar *filename, bool create_new, bool read_only, bool keep_cache_in_memory FI_DEFAULT(FALSE), int flags FI_DEFAULT(0));
    bool (DLL_CALLCONV *FI_CloseMultiBitmap)(FIMULTIBITMAP *bitmap, int flags FI_DEFAULT(0));
    int (DLL_CALLCONV *FI_GetPageCount)(FIMULTIBITMAP *bitmap);
    void (DLL_CALLCONV *FI_AppendPage)(FIMULTIBITMAP *bitmap, FIBITMAP *data);
    void (DLL_CALLCONV *FI_InsertPage)(FIMULTIBITMAP *bitmap, int page, FIBITMAP *data);
    void (DLL_CALLCONV *FI_DeletePage)(FIMULTIBITMAP *bitmap, int page);
    FIBITMAP *(DLL_CALLCONV *FI_LockPage)(FIMULTIBITMAP *bitmap, int page);
    void (DLL_CALLCONV *FI_UnlockPage)(FIMULTIBITMAP *bitmap, FIBITMAP *data, bool changed);
    bool (DLL_CALLCONV *FI_MovePage)(FIMULTIBITMAP *bitmap, int target, int source);
    bool (DLL_CALLCONV *FI_GetLockedPageNumbers)(FIMULTIBITMAP *bitmap, int *pages, int *count);

// Filetype request routines ------------------------------------------------

    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFileType)(const AnsiChar *filename, int size FI_DEFAULT(0));
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFileTypeU)(const wchar_t *filename, int size FI_DEFAULT(0));
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFileTypeFromHandle)(FreeImageIO *io, fi_handle handle, int size FI_DEFAULT(0));
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFileTypeFromMemory)(FIMEMORY *stream, int size FI_DEFAULT(0));

// FreeImage helper routines ------------------------- MISSING !!!! TODO

    bool (DLL_CALLCONV *FI_IsLittleEndian)(void);
    bool (DLL_CALLCONV *FI_LookupX11Color)(const AnsiChar *szColor, byte *nRed, byte *nGreen, byte *nBlue);
    bool (DLL_CALLCONV *FI_LookupSVGColor)(const AnsiChar *szColor, byte *nRed, byte *nGreen, byte *nBlue);

// Image type request routine -----------------------------------------------

    FREE_IMAGE_TYPE (DLL_CALLCONV *FI_GetImageType)(FIBITMAP *dib);

// Pixel access routines ----------------------------------------------------

    byte *(DLL_CALLCONV *FI_GetBits)(FIBITMAP *dib);
    byte *(DLL_CALLCONV *FI_GetScanLine)(FIBITMAP *dib, int scanline);

    bool (DLL_CALLCONV *FI_GetPixelIndex)(FIBITMAP *dib, unsigned x, unsigned y, byte *value);
    bool (DLL_CALLCONV *FI_GetPixelColor)(FIBITMAP *dib, unsigned x, unsigned y, RGBQUAD *value);
    bool (DLL_CALLCONV *FI_SetPixelIndex)(FIBITMAP *dib, unsigned x, unsigned y, byte *value);
    bool (DLL_CALLCONV *FI_SetPixelColor)(FIBITMAP *dib, unsigned x, unsigned y, RGBQUAD *value);

// DIB info routines --------------------------------------------------------

    unsigned (DLL_CALLCONV *FI_GetColorsUsed)(FIBITMAP *dib);
    unsigned (DLL_CALLCONV *FI_GetBPP)(FIBITMAP *dib);
    unsigned (DLL_CALLCONV *FI_GetWidth)(FIBITMAP *dib);
    unsigned (DLL_CALLCONV *FI_GetHeight)(FIBITMAP *dib);
    unsigned (DLL_CALLCONV *FI_GetLine)(FIBITMAP *dib);
    unsigned (DLL_CALLCONV *FI_GetPitch)(FIBITMAP *dib);
    unsigned (DLL_CALLCONV *FI_GetDIBSize)(FIBITMAP *dib);
    RGBQUAD *(DLL_CALLCONV *FI_GetPalette)(FIBITMAP *dib);

    unsigned (DLL_CALLCONV *FI_GetDotsPerMeterX)(FIBITMAP *dib);
    unsigned (DLL_CALLCONV *FI_GetDotsPerMeterY)(FIBITMAP *dib);
    void (DLL_CALLCONV *FI_SetDotsPerMeterX)(FIBITMAP *dib, unsigned res);
    void (DLL_CALLCONV *FI_SetDotsPerMeterY)(FIBITMAP *dib, unsigned res);

    BITMAPINFOHEADER *(DLL_CALLCONV *FI_GetInfoHeader)(FIBITMAP *dib);
    BITMAPINFO *(DLL_CALLCONV *FI_GetInfo)(FIBITMAP *dib);
    FREE_IMAGE_COLOR_TYPE (DLL_CALLCONV *FI_GetColorType)(FIBITMAP *dib);

    unsigned (DLL_CALLCONV *FI_GetRedMask)(FIBITMAP *dib);
    unsigned (DLL_CALLCONV *FI_GetGreenMask)(FIBITMAP *dib);
    unsigned (DLL_CALLCONV *FI_GetBlueMask)(FIBITMAP *dib);

    unsigned (DLL_CALLCONV *FI_GetTransparencyCount)(FIBITMAP *dib);
    byte *(DLL_CALLCONV *FI_GetTransparencyTable)(FIBITMAP *dib);
    void (DLL_CALLCONV *FI_SetTransparent)(FIBITMAP *dib, bool enabled);
    void (DLL_CALLCONV *FI_SetTransparencyTable)(FIBITMAP *dib, byte *table, int count);
    bool (DLL_CALLCONV *FI_IsTransparent)(FIBITMAP *dib);

    bool (DLL_CALLCONV *FI_HasBackgroundColor)(FIBITMAP *dib);
    bool (DLL_CALLCONV *FI_GetBackgroundColor)(FIBITMAP *dib, RGBQUAD *bkcolor);
    bool (DLL_CALLCONV *FI_SetBackgroundColor)(FIBITMAP *dib, RGBQUAD *bkcolor);

    // ICC profile routines ------------------------------- MISSING !!! TODO

    FIICCPROFILE *(DLL_CALLCONV *FI_GetICCProfile)(FIBITMAP *dib);
    FIICCPROFILE *(DLL_CALLCONV *FI_CreateICCProfile)(FIBITMAP *dib, void *data, long size);
    void (DLL_CALLCONV *FI_DestroyICCProfile)(FIBITMAP *dib);

    // Line conversion routines ----------------------------MISSING !!! TODO

// Smart conversion routines ------------------------------------------------

    FIBITMAP *(DLL_CALLCONV *FI_ConvertTo4Bits)(FIBITMAP *dib);
    FIBITMAP *(DLL_CALLCONV *FI_ConvertTo8Bits)(FIBITMAP *dib);
    FIBITMAP *(DLL_CALLCONV *FI_ConvertToGreyscale)(FIBITMAP *dib);
    FIBITMAP *(DLL_CALLCONV *FI_ConvertTo16Bits555)(FIBITMAP *dib);
    FIBITMAP *(DLL_CALLCONV *FI_ConvertTo16Bits565)(FIBITMAP *dib);
    FIBITMAP *(DLL_CALLCONV *FI_ConvertTo24Bits)(FIBITMAP *dib);
    FIBITMAP *(DLL_CALLCONV *FI_ConvertTo32Bits)(FIBITMAP *dib);
  FIBITMAP *(DLL_CALLCONV *FI_ColorQuantize)(FIBITMAP *dib, FREE_IMAGE_QUANTIZE quantize);

    FIBITMAP *(DLL_CALLCONV *FI_ColorQuantizeEx)(FIBITMAP *dib, FREE_IMAGE_QUANTIZE quantize /*FI_DEFAULT(FIQ_WUQUANT) */, int PaletteSize FI_DEFAULT(256), int ReserveSize FI_DEFAULT(0), RGBQUAD *ReservePalette FI_DEFAULT(NULL));

    FIBITMAP *(DLL_CALLCONV *FI_Threshold)(FIBITMAP *dib, byte T);
    FIBITMAP *(DLL_CALLCONV *FI_Dither)(FIBITMAP *dib, FREE_IMAGE_DITHER algorithm);
    FIBITMAP *(DLL_CALLCONV *FI_ConvertFromRawBits)(byte *bits, int width, int height, int pitch, unsigned bpp, unsigned red_mask, unsigned green_mask, unsigned blue_mask, bool topdown FI_DEFAULT(FALSE));
    void (DLL_CALLCONV *FI_ConvertToRawBits)(byte *bits, FIBITMAP *dib, int pitch, unsigned bpp, unsigned red_mask, unsigned green_mask, unsigned blue_mask, bool topdown FI_DEFAULT(FALSE));

    FIBITMAP *(DLL_CALLCONV *FI_ConvertToRGBF)(FIBITMAP *dib);
    FIBITMAP *(DLL_CALLCONV *FI_ConvertToStandardType)(FIBITMAP *src, bool scale_linear FI_DEFAULT(TRUE));
    FIBITMAP *(DLL_CALLCONV *FI_ConvertToType)(FIBITMAP *src, FREE_IMAGE_TYPE dst_type, bool scale_linear FI_DEFAULT(TRUE));

// tone mapping operators
    FIBITMAP *(DLL_CALLCONV *FI_ToneMapping)(FIBITMAP *dib, FREE_IMAGE_TMO tmo, double first_param FI_DEFAULT(0), double second_param FI_DEFAULT(0));
    FIBITMAP *(DLL_CALLCONV *FI_TmoDrago03)(FIBITMAP *src, double gamma FI_DEFAULT(2.2), double exposure FI_DEFAULT(0));
    FIBITMAP *(DLL_CALLCONV *FI_TmoReinhard05)(FIBITMAP *src, double intensity FI_DEFAULT(0), double contrast FI_DEFAULT(0));

// ZLib interface -----------------------------------------------------------

    DWORD (DLL_CALLCONV *FI_ZLibCompress)(byte *target, DWORD target_size, byte *source, DWORD source_size);
    DWORD (DLL_CALLCONV *FI_ZLibUncompress)(byte *target, DWORD target_size, byte *source, DWORD source_size);
    DWORD (DLL_CALLCONV *FI_ZLibGZip)(byte *target, DWORD target_size, byte *source, DWORD source_size);
    DWORD (DLL_CALLCONV *FI_ZLibGUnzip)(byte *target, DWORD target_size, byte *source, DWORD source_size);
    DWORD (DLL_CALLCONV *FI_ZLibCRC32)(DWORD crc, byte *source, DWORD source_size);


// --------------------------------------------------------------------------
// Metadata routines --------------------------------------------------------
// --------------------------------------------------------------------------

// tag creation / destruction
    FITAG *(DLL_CALLCONV *FI_CreateTag)();
    void (DLL_CALLCONV *FI_DeleteTag)(FITAG *tag);
    FITAG *(DLL_CALLCONV *FI_CloneTag)(FITAG *tag);

// tag getters and setters
    const AnsiChar *(DLL_CALLCONV *FI_GetTagKey)(FITAG *tag);
    const AnsiChar *(DLL_CALLCONV *FI_GetTagDescription)(FITAG *tag);
    word (DLL_CALLCONV *FI_GetTagID)(FITAG *tag);
    FREE_IMAGE_MDTYPE (DLL_CALLCONV *FI_GetTagType)(FITAG *tag);
    DWORD (DLL_CALLCONV *FI_GetTagCount)(FITAG *tag);
    DWORD (DLL_CALLCONV *FI_GetTagLength)(FITAG *tag);
    const void *(DLL_CALLCONV *FI_GetTagValue)(FITAG *tag);

    bool (DLL_CALLCONV *FI_SetTagKey)(FITAG *tag, const AnsiChar *key);
    bool (DLL_CALLCONV *FI_SetTagDescription)(FITAG *tag, const AnsiChar *description);
    bool (DLL_CALLCONV *FI_SetTagID)(FITAG *tag, word id);
    bool (DLL_CALLCONV *FI_SetTagType)(FITAG *tag, FREE_IMAGE_MDTYPE type);
    bool (DLL_CALLCONV *FI_SetTagCount)(FITAG *tag, DWORD count);
    bool (DLL_CALLCONV *FI_SetTagLength)(FITAG *tag, DWORD length);
    bool (DLL_CALLCONV *FI_SetTagValue)(FITAG *tag, const void *value);

// iterator
    FIMETADATA *(DLL_CALLCONV *FI_FindFirstMetadata)(FREE_IMAGE_MDMODEL model, FIBITMAP *dib, FITAG **tag);
    bool (DLL_CALLCONV *FI_FindNextMetadata)(FIMETADATA *mdhandle, FITAG **tag);
    void (DLL_CALLCONV *FI_FindCloseMetadata)(FIMETADATA *mdhandle);

// metadata setter and getter
    bool (DLL_CALLCONV *FI_SetMetadata)(FREE_IMAGE_MDMODEL model, FIBITMAP *dib, const AnsiChar *key, FITAG *tag);
    bool (DLL_CALLCONV *FI_GetMetadata)(FREE_IMAGE_MDMODEL model, FIBITMAP *dib, const AnsiChar *key, FITAG **tag);

// helpers
    unsigned (DLL_CALLCONV *FI_GetMetadataCount)(FREE_IMAGE_MDMODEL model, FIBITMAP *dib);

// tag to C string conversion
    const AnsiChar *(DLL_CALLCONV *FI_TagToString)(FREE_IMAGE_MDMODEL model, FITAG *tag, AnsiChar *Make FI_DEFAULT(NULL));


// --------------------------------------------------------------------------
// Image manipulation toolkit -----------------------------------------------
// --------------------------------------------------------------------------

// rotation and flipping
    FIBITMAP *(DLL_CALLCONV *FI_RotateClassic)(FIBITMAP *dib, double angle);
    FIBITMAP *(DLL_CALLCONV *FI_RotateEx)(FIBITMAP *dib, double angle, double x_shift, double y_shift, double x_origin, double y_origin, bool use_mask);
    bool (DLL_CALLCONV *FI_FlipHorizontal)(FIBITMAP *dib);
    bool (DLL_CALLCONV *FI_FlipVertical)(FIBITMAP *dib);
    bool (DLL_CALLCONV *FI_JPEGTransform)(const AnsiChar *src_file, const AnsiChar *dst_file, FREE_IMAGE_JPEG_OPERATION operation, bool perfect FI_DEFAULT(FALSE));

// upsampling / downsampling
    FIBITMAP *(DLL_CALLCONV *FI_Rescale)(FIBITMAP *dib, int dst_width, int dst_height, FREE_IMAGE_FILTER filter);
    FIBITMAP *(DLL_CALLCONV *FI_MakeThumbnail)(FIBITMAP *dib, int max_pixel_size, bool convert FI_DEFAULT(TRUE));

// color manipulation routines (point operations)
  bool (DLL_CALLCONV *FI_AdjustCurve)(FIBITMAP *dib, byte *LUT, FREE_IMAGE_COLOR_CHANNEL channel);
    bool (DLL_CALLCONV *FI_AdjustGamma)(FIBITMAP *dib, double gamma);
    bool (DLL_CALLCONV *FI_AdjustBrightness)(FIBITMAP *dib, double percentage);
    bool (DLL_CALLCONV *FI_AdjustContrast)(FIBITMAP *dib, double percentage);
    bool (DLL_CALLCONV *FI_Invert)(FIBITMAP *dib);
    bool (DLL_CALLCONV *FI_GetHistogram)(FIBITMAP *dib, DWORD *histo, FREE_IMAGE_COLOR_CHANNEL channel);

// channel processing routines
    FIBITMAP *(DLL_CALLCONV *FI_GetChannel)(FIBITMAP *dib, FREE_IMAGE_COLOR_CHANNEL channel);
    bool (DLL_CALLCONV *FI_SetChannel)(FIBITMAP *dib, FIBITMAP *dib8, FREE_IMAGE_COLOR_CHANNEL channel);
    FIBITMAP *(DLL_CALLCONV *FI_GetComplexChannel)(FIBITMAP *src, FREE_IMAGE_COLOR_CHANNEL channel);
    bool (DLL_CALLCONV *FI_SetComplexChannel)(FIBITMAP *dst, FIBITMAP *src, FREE_IMAGE_COLOR_CHANNEL channel);

// copy / paste / composite routines
    FIBITMAP *(DLL_CALLCONV *FI_Copy)(FIBITMAP *dib, int left, int top, int right, int bottom);
    bool (DLL_CALLCONV *FI_Paste)(FIBITMAP *dst, FIBITMAP *src, int left, int top, int alpha);
    FIBITMAP *(DLL_CALLCONV *FI_Composite)(FIBITMAP *fg, bool useFileBkg FI_DEFAULT(FALSE), RGBQUAD *appBkColor FI_DEFAULT(NULL), FIBITMAP *bg FI_DEFAULT(NULL));
    bool (DLL_CALLCONV *FI_JPEGCrop)(const AnsiChar *src_file, const AnsiChar *dst_file, int left, int top, int right, int bottom);

// own functions

    // memory I/O
    FIBITMAP *(*FI_LoadFromMem)(FREE_IMAGE_FORMAT fif, fiio_mem_handle *handle, int flags);
    bool (*FI_SaveToMem)(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, fiio_mem_handle *handle, int flags);

    // helpers
    FIBITMAP *(*FI_CreateDIBFromHBITMAP)(HBITMAP hBmp);
    HBITMAP  (*FI_CreateHBITMAPFromDIB)(FIBITMAP *dib);
    bool     (*FI_Premultiply)(HBITMAP hBmp);                               // premultiplies alpha channel for usage with AlphaBlend()
                                                                            // original HBITMAP stays valid and must be 32bit RGBA
    int      (*FI_BmpFilterResizeBitmap)(WPARAM wParam,LPARAM lParam);      // more generic resizer for avatar images
    void     (*FI_CorrectBitmap32Alpha)(HBITMAP hBitmap, bool force);       // corrects broken images (when all alpha values are 0)

    FIMULTIBITMAP *(DLL_CALLCONV *FI_OpenMultiBitmapU)(FREE_IMAGE_FORMAT fif, const wchar_t *filename, bool create_new, bool read_only, bool keep_cache_in_memory FI_DEFAULT(FALSE), int flags FI_DEFAULT(0));

    void*   reserved[49];            // future usage
} FI_INTERFACE;
*)
{
  image services
  
  only basic functionality is wrapped around Miranda services, because otherwise we
  would get a huge load of services with complex parameter marshalling requirements.
}
const
// get the interface version number
// wParam = lParam = 0
// returns FI_IF_VERSION

  MS_IMG_GETIFVERSION:PAnsiChar = 'IMG/GetVersion';

{
  obtain the full FreeImage interface from the library. This interface provides
  full access to freeimage internal functions, thus enabling devs to fully utilize
  the FreeImage API. Only popular functions will be exported as miranda services.
  wParam = (DWORD)version Number // the caller MUST provide the desired version number
  lParam = **FI_INTERFACE
  returns S_OK on success, any other value indicates a problem.
  the interface is populated during the _DllMain() handler, so you can assume it
  is ready when Miranda calls the Load() handler of your plugin and you can return
  1 in your Load() to disable your plugin when it depends on the freeimage interface
  and the freeimage plugin is missing
  
  example:
  var
    fif:PFI_INTERFACE;
   fif:=nil;
  result:=CallService(MS_IMG_GETINTERFACE, FI_IF_VERSION,dword(fii));
  
  if result<>S_OK then
    failed, in this case, fei will be NULL and your plugin will crash when using the interface,
    so ALWAYS check it   
}
  MS_IMG_GETINTERFACE:PAnsiChar = 'IMG/GetInterface';

  IMGL_RETURNDIB = 1; // will NOT return a HBITMAP but a FIBITMAP * instead (useful,
                      // if you want to do further image manipulations before
                      // converting to a Win32 bitmap caller MUST then free the 
                      // FIBITMAP * using fii->FI_Unload() or MS_IMG_UNLOAD (see below)
  IMGL_WCHAR     = 2; // filename is Unicode

{
  load an image from disk
  wParam = full path and filename to the image
  lParam = IMGL_* flags
  returns a valid HBITMAP or 0 if image cannot be loaded
  if IMGL_RETURNDIB is set, it returns a pointer to a freeimage bitmap (FIBITMAP *)
}
  MS_IMG_LOAD:PAnsiChar = 'IMG/Load';

{
  control structure for loading images from memory buffers (e.g. network buffers,
  memory mapped files).
}

//type
//!!
//  FREE_IMAGE_FORMAT = type integer;
const
  FIF_UNKNOWN = -1;
  FIF_BMP     = 0;
  FIF_ICO     = 1;
  FIF_JPEG    = 2;
  FIF_JNG     = 3;
  FIF_KOALA   = 4;
  FIF_LBM     = 5;
  FIF_IFF     = FIF_LBM;
  FIF_MNG     = 6;
  FIF_PBM     = 7;
  FIF_PBMRAW  = 8;
  FIF_PCD     = 9;
  FIF_PCX     = 10;
  FIF_PGM     = 11;
  FIF_PGMRAW  = 12;
  FIF_PNG     = 13;
  FIF_PPM     = 14;
  FIF_PPMRAW  = 15;
  FIF_RAS     = 16;
  FIF_TARGA   = 17;
  FIF_TIFF    = 18;
  FIF_WBMP    = 19;
  FIF_PSD     = 20;
  FIF_CUT     = 21;
  FIF_XBM     = 22;
  FIF_XPM     = 23;
  FIF_DDS     = 24;
  FIF_GIF     = 25;
  FIF_HDR     = 26;
  FIF_FAXG3   = 27;
  FIF_SGI     = 28;
  FIF_EXR     = 29;
  FIF_J2K     = 30;
  FIF_JP2     = 31;
  FIF_PFM     = 32;
  FIF_PICT    = 33;
  FIF_RAW     = 34;

type
  TIMGSRVC_MEMIO = record
    iLen:cardinal; // length of the buffer
    pBuf:pointer;  // the buffer itself (you are responsible for allocating and free'ing it)
    fif:integer;   // FREE_IMAGE_FORMAT,-1 to detect the format or one of the FIF_* image
                   // format constant. Make sure to provide the right one.
    flags:dword;   // flags to pass to FreeImage_LoadFromMem()  (see freeimage docs)
  end;
  _tagIMGSRVC_MEMIO = TIMGSRVC_MEMIO;

const
{
  load an image from a memory buffer
  wParam = IMGSRVC_MEMIO *
  lParam = flags (see IMG/Load), valid are IMGL_RETURNDIB
  you must popupate iLen (buffer length) and pBuf (pointer to memory buffer)
  you must also specify the format in IMGSRVC_MEMIO.fif using one of the FIF_* constants
}
  MS_IMG_LOADFROMMEM:PAnsiChar = 'IMG/LoadFromMem';

// flags for IMGSRVC_INFO.dwMask

  IMGI_FBITMAP = 1;  // the dib member is valid
  IMGI_HBITMAP = 2;  // the hbm member is valid

{
  generic structure for various img functions
  you must populate the fields as required, set the mask bits to indicate which member is valid
}

type
//!!
{
  FIBITMAP = record
    data : pointer;
  end;
  PFIBITMAP = ^FIBITMAP;
}
  TIMGSRVC_INFO = record
    cbSize:dword;
    szName:TChar;
    hbm:HBITMAP;
    dib:pointer; // PFIBITMAP
    dwMask:dword;
    fif:integer; // FREE_IMAGE_FORMAT
  end;
  _tagIMGSRVC_INFO = TIMGSRVC_INFO;

const
{
  save image to disk
  wParam = pointer to IMGSRVC_INFO (szName/wszName, hbm OR dib, cbSize, dwMask
           mandatory. fif optional, if FIF_UNKNOWN is given it will be determined
           from the filename).
  lParam = low word: IMG_* flags (IMGL_WCHAR is the only valid - filename will be assumed
           to be wchar_t and wszName must be used)
           high word: FreeImage_Save flags
  set IMGSRVC_INFO.dwMask to indicate whether the HBITMAP of FIBITMAP member is valid
}
  MS_IMG_SAVE:PAnsiChar = 'IMG/Save';

{
  unload a FIFBITMAP
  wParam = FIFBITMAP *
  lParam = 0;
  this service is useful when you have loaded a bitmap with IMGL_RETURNDIB in
  which case you do not get a HBITMAP but instead a FBITMAP * which describes
  the freeimage-internal representation of a bitmap.
}
  MS_IMG_UNLOAD:PAnsiChar = 'IMG/Unload';

{
 resizer from loadavatars moved to image service plugin
}

  RESIZEBITMAP_STRETCH          = 0;   // Distort bitmap to size in (max_width, max_height)
  RESIZEBITMAP_KEEP_PROPORTIONS = 1;   // Keep bitmap proportions (probabily only
                                       // one of the max_width/max_height will be
                                       // respected, and the other will be smaller)
  RESIZEBITMAP_CROP             = 2;   // Keep bitmap proportions but crop it to
                                       // fix exactly in (max_width, max_height)
                                       // Some image info outside will be lost
  RESIZEBITMAP_MAKE_SQUARE      = 3;   // Image will be allways square. Image will
                                       // be croped and the size returned will be
                                       // min(max_width, max_height)
  RESIZEBITMAP_FLAG_DONT_GROW  = $1000; // If set, the image will not grow. Else,
                                       // it will grow to fit the max width/height

type
  TResizeBitmap = record
    size      :size_t;   // sizeof(ResizeBitmap);
    hBmp      :HBITMAP;
    max_width :int;
    max_height:int;
    fit       :int;      // One of RESIZEBITMAP_* with an OR with RESIZEBITMAP_FLAG_DONT_GROW if needed
  end;

const
{
  Returns a copy of the bitmap with the size especified or the original bitmap
  if nothing has to be changed
  wParam = ResizeBitmap *
  lParam = NULL
  return NULL on error, ResizeBitmap->hBmp if don't need to resize or a new HBITMAP if resized
  You are responsible for calling DestroyObject() on the original HBITMAP
}
  MS_IMG_RESIZE:PAnsiChar = 'IMG/ResizeBitmap';

{
 * format conversion helpers
 * 
 * these helper macros allow converting HBITMAP to FIBITMAP * format and vice vera. In any case,
 * the caller is responsible for freeing or deleting the original object.
 * These macros wrap around the FI_CreateHBITMAPFromDib() and FI_CreateDIBFromHBITMAP() interface 
 * functions.
}

//#define FI_HBM2DIB(x) (FI_CreateDIBFromHBITMAP((x)))
//#define FI_DIB2HBM(x) (FI_CreateHBITMAPFromDIB((x)))

{$ENDIF}
