/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-12 Miranda ICQ/IM project,
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

*/

#ifndef __M_IMGSRVC_H
#define __M_IMGSRVC_H

#define _FI_MIMPLUGIN 1

#include "m_freeimage.h"

#define FI_IF_VERSION (PLUGIN_MAKE_VERSION(0, 0, 1, 0))           // interface version - must match

// memory i/o defs

/*
 * this struct defines a memio job.
 * datalen and filename must match and must be populated to the size of the memory buffer (caution)
 * data must point to the buffer.
 * curpos is internal and should be initialized to 0
 */

typedef struct fiio_mem_handle_s {
	long filelen,datalen,curpos;
	void *data;
} fiio_mem_handle;

/* it is up to the user to create a fiio_mem_handle and init datalen and data
 * filelen will be pre-set to 0 by SaveToMem
 * curpos will be pre-set to 0 by SaveToMem and LoadFromMem
 * IMPORTANT: data should be set to NULL and datalen to 0,
 *            unless the user wants to manually malloc a larger buffer
 */
FIBITMAP *FreeImage_LoadFromMem(FREE_IMAGE_FORMAT fif, fiio_mem_handle *handle, int flags);
FIMEMORY *FreeImage_SaveToMem(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, fiio_mem_handle *handle, int flags);

void SetMemIO(FreeImageIO *io);
unsigned __stdcall fiio_mem_ReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle);
unsigned __stdcall fiio_mem_WriteProc(void *buffer, unsigned size, unsigned count, fi_handle handle);
int __stdcall fiio_mem_SeekProc(fi_handle handle, long offset, int origin);
long __stdcall fiio_mem_TellProc(fi_handle handle);

/*
 * this interface directly exports most of FreeImage routines
 * you can use them in your plugin after obtaining the interfasce using MS_IMG_GETINTERFACE
 */

typedef struct _tagFI_interface {

    DWORD version;

    FIBITMAP *(DLL_CALLCONV *FI_Allocate)(int width, int height, int bpp, unsigned red_mask FI_DEFAULT(0), unsigned green_mask FI_DEFAULT(0), unsigned blue_mask FI_DEFAULT(0));
    FIBITMAP *(DLL_CALLCONV *FI__AllocateT)(FREE_IMAGE_TYPE type, int width, int height, int bpp FI_DEFAULT(8), unsigned red_mask FI_DEFAULT(0), unsigned green_mask FI_DEFAULT(0), unsigned blue_mask FI_DEFAULT(0));
    FIBITMAP *(DLL_CALLCONV *FI_Clone)(FIBITMAP *dib);
    void (DLL_CALLCONV *FI_Unload)(FIBITMAP *dib);

    // Load / Save routines -----------------------------------------------------

    FIBITMAP *(DLL_CALLCONV *FI_Load)(FREE_IMAGE_FORMAT fif, const char *filename, int flags FI_DEFAULT(0));
    FIBITMAP *(DLL_CALLCONV *FI_LoadU)(FREE_IMAGE_FORMAT fif, const wchar_t *filename, int flags FI_DEFAULT(0));
    FIBITMAP *(DLL_CALLCONV *FI_LoadFromHandle)(FREE_IMAGE_FORMAT fif, FreeImageIO *io, fi_handle handle, int flags FI_DEFAULT(0));
    BOOL (DLL_CALLCONV *FI_Save)(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, const char *filename, int flags FI_DEFAULT(0));
    BOOL (DLL_CALLCONV *FI_SaveU)(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, const wchar_t *filename, int flags FI_DEFAULT(0));
    BOOL (DLL_CALLCONV *FI_SaveToHandle)(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, FreeImageIO *io, fi_handle handle, int flags FI_DEFAULT(0));

// Memory I/O stream routines -----------------------------------------------

    FIMEMORY *(DLL_CALLCONV *FI_OpenMemory)(BYTE *data FI_DEFAULT(0), DWORD size_in_bytes FI_DEFAULT(0));
    void (DLL_CALLCONV *FI_CloseMemory)(FIMEMORY *stream);
    FIBITMAP *(DLL_CALLCONV *FI_LoadFromMemory)(FREE_IMAGE_FORMAT fif, FIMEMORY *stream, int flags FI_DEFAULT(0));
    BOOL (DLL_CALLCONV *FI_SaveToMemory)(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, FIMEMORY *stream, int flags FI_DEFAULT(0));
    long (DLL_CALLCONV *FI_TellMemory)(FIMEMORY *stream);
    BOOL (DLL_CALLCONV *FI_SeekMemory)(FIMEMORY *stream, long offset, int origin);
    BOOL (DLL_CALLCONV *FI_AcquireMemory)(FIMEMORY *stream, BYTE **data, DWORD *size_in_bytes);
    unsigned (DLL_CALLCONV *FI_ReadMemory)(void *buffer, unsigned size, unsigned count, FIMEMORY *stream);
    unsigned (DLL_CALLCONV *FI_WriteMemory)(const void *buffer, unsigned size, unsigned count, FIMEMORY *stream);
    FIMULTIBITMAP *(DLL_CALLCONV *FI_LoadMultiBitmapFromMemory)(FREE_IMAGE_FORMAT fif, FIMEMORY *stream, int flags FI_DEFAULT(0));

// Plugin Interface ---------------------------------------------------------

    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_RegisterLocalPlugin)(FI_InitProc proc_address, const char *format FI_DEFAULT(0), const char *description FI_DEFAULT(0), const char *extension FI_DEFAULT(0), const char *regexpr FI_DEFAULT(0));
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_RegisterExternalPlugin)(const char *path, const char *format FI_DEFAULT(0), const char *description FI_DEFAULT(0), const char *extension FI_DEFAULT(0), const char *regexpr FI_DEFAULT(0));
    int (DLL_CALLCONV *FI_GetFIFCount)(void);
    int (DLL_CALLCONV *FI_SetPluginEnabled)(FREE_IMAGE_FORMAT fif, BOOL enable);
    int (DLL_CALLCONV *FI_IsPluginEnabled)(FREE_IMAGE_FORMAT fif);
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFIFFromFormat)(const char *format);
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFIFFromMime)(const char *mime);
    const char *(DLL_CALLCONV *FI_GetFormatFromFIF)(FREE_IMAGE_FORMAT fif);
    const char *(DLL_CALLCONV *FI_GetFIFExtensionList)(FREE_IMAGE_FORMAT fif);
    const char *(DLL_CALLCONV *FI_GetFIFDescription)(FREE_IMAGE_FORMAT fif);
    const char *(DLL_CALLCONV *FI_GetFIFRegExpr)(FREE_IMAGE_FORMAT fif);
    const char *(DLL_CALLCONV *FI_GetFIFMimeType)(FREE_IMAGE_FORMAT fif);
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFIFFromFilename)(const char *filename);
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFIFFromFilenameU)(const wchar_t *filename);
    BOOL (DLL_CALLCONV *FI_FIFSupportsReading)(FREE_IMAGE_FORMAT fif);
    BOOL (DLL_CALLCONV *FI_FIFSupportsWriting)(FREE_IMAGE_FORMAT fif);
    BOOL (DLL_CALLCONV *FI_FIFSupportsExportBPP)(FREE_IMAGE_FORMAT fif, int bpp);
    BOOL (DLL_CALLCONV *FI_FIFSupportsExportType)(FREE_IMAGE_FORMAT fif, FREE_IMAGE_TYPE type);
    BOOL (DLL_CALLCONV *FI_FIFSupportsICCProfiles)(FREE_IMAGE_FORMAT fif);

// Multipaging interface ----------------------------------------------------

    FIMULTIBITMAP *(DLL_CALLCONV *FI_OpenMultiBitmap)(FREE_IMAGE_FORMAT fif, const char *filename, BOOL create_new, BOOL read_only, BOOL keep_cache_in_memory FI_DEFAULT(FALSE), int flags FI_DEFAULT(0));
    BOOL (DLL_CALLCONV *FI_CloseMultiBitmap)(FIMULTIBITMAP *bitmap, int flags FI_DEFAULT(0));
    int (DLL_CALLCONV *FI_GetPageCount)(FIMULTIBITMAP *bitmap);
    void (DLL_CALLCONV *FI_AppendPage)(FIMULTIBITMAP *bitmap, FIBITMAP *data);
    void (DLL_CALLCONV *FI_InsertPage)(FIMULTIBITMAP *bitmap, int page, FIBITMAP *data);
    void (DLL_CALLCONV *FI_DeletePage)(FIMULTIBITMAP *bitmap, int page);
    FIBITMAP *(DLL_CALLCONV *FI_LockPage)(FIMULTIBITMAP *bitmap, int page);
    void (DLL_CALLCONV *FI_UnlockPage)(FIMULTIBITMAP *bitmap, FIBITMAP *data, BOOL changed);
    BOOL (DLL_CALLCONV *FI_MovePage)(FIMULTIBITMAP *bitmap, int target, int source);
    BOOL (DLL_CALLCONV *FI_GetLockedPageNumbers)(FIMULTIBITMAP *bitmap, int *pages, int *count);

// Filetype request routines ------------------------------------------------

    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFileType)(const char *filename, int size FI_DEFAULT(0));
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFileTypeU)(const wchar_t *filename, int size FI_DEFAULT(0));
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFileTypeFromHandle)(FreeImageIO *io, fi_handle handle, int size FI_DEFAULT(0));
    FREE_IMAGE_FORMAT (DLL_CALLCONV *FI_GetFileTypeFromMemory)(FIMEMORY *stream, int size FI_DEFAULT(0));

// FreeImage helper routines ------------------------- MISSING !!!! TODO

    BOOL (DLL_CALLCONV *FI_IsLittleEndian)(void);
    BOOL (DLL_CALLCONV *FI_LookupX11Color)(const char *szColor, BYTE *nRed, BYTE *nGreen, BYTE *nBlue);
    BOOL (DLL_CALLCONV *FI_LookupSVGColor)(const char *szColor, BYTE *nRed, BYTE *nGreen, BYTE *nBlue);

// Image type request routine -----------------------------------------------

    FREE_IMAGE_TYPE (DLL_CALLCONV *FI_GetImageType)(FIBITMAP *dib);

// Pixel access routines ----------------------------------------------------

    BYTE *(DLL_CALLCONV *FI_GetBits)(FIBITMAP *dib);
    BYTE *(DLL_CALLCONV *FI_GetScanLine)(FIBITMAP *dib, int scanline);

    BOOL (DLL_CALLCONV *FI_GetPixelIndex)(FIBITMAP *dib, unsigned x, unsigned y, BYTE *value);
    BOOL (DLL_CALLCONV *FI_GetPixelColor)(FIBITMAP *dib, unsigned x, unsigned y, RGBQUAD *value);
    BOOL (DLL_CALLCONV *FI_SetPixelIndex)(FIBITMAP *dib, unsigned x, unsigned y, BYTE *value);
    BOOL (DLL_CALLCONV *FI_SetPixelColor)(FIBITMAP *dib, unsigned x, unsigned y, RGBQUAD *value);

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
    BYTE *(DLL_CALLCONV *FI_GetTransparencyTable)(FIBITMAP *dib);
    void (DLL_CALLCONV *FI_SetTransparent)(FIBITMAP *dib, BOOL enabled);
    void (DLL_CALLCONV *FI_SetTransparencyTable)(FIBITMAP *dib, BYTE *table, int count);
    BOOL (DLL_CALLCONV *FI_IsTransparent)(FIBITMAP *dib);

    BOOL (DLL_CALLCONV *FI_HasBackgroundColor)(FIBITMAP *dib);
    BOOL (DLL_CALLCONV *FI_GetBackgroundColor)(FIBITMAP *dib, RGBQUAD *bkcolor);
    BOOL (DLL_CALLCONV *FI_SetBackgroundColor)(FIBITMAP *dib, RGBQUAD *bkcolor);

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

    FIBITMAP *(DLL_CALLCONV *FI_Threshold)(FIBITMAP *dib, BYTE T);
    FIBITMAP *(DLL_CALLCONV *FI_Dither)(FIBITMAP *dib, FREE_IMAGE_DITHER algorithm);
    FIBITMAP *(DLL_CALLCONV *FI_ConvertFromRawBits)(BYTE *bits, int width, int height, int pitch, unsigned bpp, unsigned red_mask, unsigned green_mask, unsigned blue_mask, BOOL topdown FI_DEFAULT(FALSE));
    void (DLL_CALLCONV *FI_ConvertToRawBits)(BYTE *bits, FIBITMAP *dib, int pitch, unsigned bpp, unsigned red_mask, unsigned green_mask, unsigned blue_mask, BOOL topdown FI_DEFAULT(FALSE));

    FIBITMAP *(DLL_CALLCONV *FI_ConvertToRGBF)(FIBITMAP *dib);
    FIBITMAP *(DLL_CALLCONV *FI_ConvertToStandardType)(FIBITMAP *src, BOOL scale_linear FI_DEFAULT(TRUE));
    FIBITMAP *(DLL_CALLCONV *FI_ConvertToType)(FIBITMAP *src, FREE_IMAGE_TYPE dst_type, BOOL scale_linear FI_DEFAULT(TRUE));

// tone mapping operators
    FIBITMAP *(DLL_CALLCONV *FI_ToneMapping)(FIBITMAP *dib, FREE_IMAGE_TMO tmo, double first_param FI_DEFAULT(0), double second_param FI_DEFAULT(0));
    FIBITMAP *(DLL_CALLCONV *FI_TmoDrago03)(FIBITMAP *src, double gamma FI_DEFAULT(2.2), double exposure FI_DEFAULT(0));
    FIBITMAP *(DLL_CALLCONV *FI_TmoReinhard05)(FIBITMAP *src, double intensity FI_DEFAULT(0), double contrast FI_DEFAULT(0));

// ZLib interface -----------------------------------------------------------

    DWORD (DLL_CALLCONV *FI_ZLibCompress)(BYTE *target, DWORD target_size, BYTE *source, DWORD source_size);
    DWORD (DLL_CALLCONV *FI_ZLibUncompress)(BYTE *target, DWORD target_size, BYTE *source, DWORD source_size);
    DWORD (DLL_CALLCONV *FI_ZLibGZip)(BYTE *target, DWORD target_size, BYTE *source, DWORD source_size);
    DWORD (DLL_CALLCONV *FI_ZLibGUnzip)(BYTE *target, DWORD target_size, BYTE *source, DWORD source_size);
    DWORD (DLL_CALLCONV *FI_ZLibCRC32)(DWORD crc, BYTE *source, DWORD source_size);


// --------------------------------------------------------------------------
// Metadata routines --------------------------------------------------------
// --------------------------------------------------------------------------

// tag creation / destruction
    FITAG *(DLL_CALLCONV *FI_CreateTag)();
    void (DLL_CALLCONV *FI_DeleteTag)(FITAG *tag);
    FITAG *(DLL_CALLCONV *FI_CloneTag)(FITAG *tag);

// tag getters and setters
    const char *(DLL_CALLCONV *FI_GetTagKey)(FITAG *tag);
    const char *(DLL_CALLCONV *FI_GetTagDescription)(FITAG *tag);
    WORD (DLL_CALLCONV *FI_GetTagID)(FITAG *tag);
    FREE_IMAGE_MDTYPE (DLL_CALLCONV *FI_GetTagType)(FITAG *tag);
    DWORD (DLL_CALLCONV *FI_GetTagCount)(FITAG *tag);
    DWORD (DLL_CALLCONV *FI_GetTagLength)(FITAG *tag);
    const void *(DLL_CALLCONV *FI_GetTagValue)(FITAG *tag);

    BOOL (DLL_CALLCONV *FI_SetTagKey)(FITAG *tag, const char *key);
    BOOL (DLL_CALLCONV *FI_SetTagDescription)(FITAG *tag, const char *description);
    BOOL (DLL_CALLCONV *FI_SetTagID)(FITAG *tag, WORD id);
    BOOL (DLL_CALLCONV *FI_SetTagType)(FITAG *tag, FREE_IMAGE_MDTYPE type);
    BOOL (DLL_CALLCONV *FI_SetTagCount)(FITAG *tag, DWORD count);
    BOOL (DLL_CALLCONV *FI_SetTagLength)(FITAG *tag, DWORD length);
    BOOL (DLL_CALLCONV *FI_SetTagValue)(FITAG *tag, const void *value);

// iterator
    FIMETADATA *(DLL_CALLCONV *FI_FindFirstMetadata)(FREE_IMAGE_MDMODEL model, FIBITMAP *dib, FITAG **tag);
    BOOL (DLL_CALLCONV *FI_FindNextMetadata)(FIMETADATA *mdhandle, FITAG **tag);
    void (DLL_CALLCONV *FI_FindCloseMetadata)(FIMETADATA *mdhandle);

// metadata setter and getter
    BOOL (DLL_CALLCONV *FI_SetMetadata)(FREE_IMAGE_MDMODEL model, FIBITMAP *dib, const char *key, FITAG *tag);
    BOOL (DLL_CALLCONV *FI_GetMetadata)(FREE_IMAGE_MDMODEL model, FIBITMAP *dib, const char *key, FITAG **tag);

// helpers
    unsigned (DLL_CALLCONV *FI_GetMetadataCount)(FREE_IMAGE_MDMODEL model, FIBITMAP *dib);

// tag to C string conversion
    const char *(DLL_CALLCONV *FI_TagToString)(FREE_IMAGE_MDMODEL model, FITAG *tag, char *Make FI_DEFAULT(NULL));


// --------------------------------------------------------------------------
// Image manipulation toolkit -----------------------------------------------
// --------------------------------------------------------------------------

// rotation and flipping
    FIBITMAP *(DLL_CALLCONV *FI_RotateClassic)(FIBITMAP *dib, double angle);
    FIBITMAP *(DLL_CALLCONV *FI_RotateEx)(FIBITMAP *dib, double angle, double x_shift, double y_shift, double x_origin, double y_origin, BOOL use_mask);
    BOOL (DLL_CALLCONV *FI_FlipHorizontal)(FIBITMAP *dib);
    BOOL (DLL_CALLCONV *FI_FlipVertical)(FIBITMAP *dib);
    BOOL (DLL_CALLCONV *FI_JPEGTransform)(const char *src_file, const char *dst_file, FREE_IMAGE_JPEG_OPERATION operation, BOOL perfect FI_DEFAULT(FALSE));

// upsampling / downsampling
    FIBITMAP *(DLL_CALLCONV *FI_Rescale)(FIBITMAP *dib, int dst_width, int dst_height, FREE_IMAGE_FILTER filter);
    FIBITMAP *(DLL_CALLCONV *FI_MakeThumbnail)(FIBITMAP *dib, int max_pixel_size, BOOL convert FI_DEFAULT(TRUE));

// color manipulation routines (point operations)
	BOOL (DLL_CALLCONV *FI_AdjustCurve)(FIBITMAP *dib, BYTE *LUT, FREE_IMAGE_COLOR_CHANNEL channel);
    BOOL (DLL_CALLCONV *FI_AdjustGamma)(FIBITMAP *dib, double gamma);
    BOOL (DLL_CALLCONV *FI_AdjustBrightness)(FIBITMAP *dib, double percentage);
    BOOL (DLL_CALLCONV *FI_AdjustContrast)(FIBITMAP *dib, double percentage);
    BOOL (DLL_CALLCONV *FI_Invert)(FIBITMAP *dib);
    BOOL (DLL_CALLCONV *FI_GetHistogram)(FIBITMAP *dib, DWORD *histo, FREE_IMAGE_COLOR_CHANNEL channel);

// channel processing routines
    FIBITMAP *(DLL_CALLCONV *FI_GetChannel)(FIBITMAP *dib, FREE_IMAGE_COLOR_CHANNEL channel);
    BOOL (DLL_CALLCONV *FI_SetChannel)(FIBITMAP *dib, FIBITMAP *dib8, FREE_IMAGE_COLOR_CHANNEL channel);
    FIBITMAP *(DLL_CALLCONV *FI_GetComplexChannel)(FIBITMAP *src, FREE_IMAGE_COLOR_CHANNEL channel);
    BOOL (DLL_CALLCONV *FI_SetComplexChannel)(FIBITMAP *dst, FIBITMAP *src, FREE_IMAGE_COLOR_CHANNEL channel);

// copy / paste / composite routines
    FIBITMAP *(DLL_CALLCONV *FI_Copy)(FIBITMAP *dib, int left, int top, int right, int bottom);
    BOOL (DLL_CALLCONV *FI_Paste)(FIBITMAP *dst, FIBITMAP *src, int left, int top, int alpha);
    FIBITMAP *(DLL_CALLCONV *FI_Composite)(FIBITMAP *fg, BOOL useFileBkg FI_DEFAULT(FALSE), RGBQUAD *appBkColor FI_DEFAULT(NULL), FIBITMAP *bg FI_DEFAULT(NULL));
    BOOL (DLL_CALLCONV *FI_JPEGCrop)(const char *src_file, const char *dst_file, int left, int top, int right, int bottom);

// own functions

    // memory I/O
    FIBITMAP *(*FI_LoadFromMem)(FREE_IMAGE_FORMAT fif, fiio_mem_handle *handle, int flags);
    FIMEMORY *(*FI_SaveToMem)(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, fiio_mem_handle *handle, int flags);

    // helpers
    FIBITMAP *(*FI_CreateDIBFromHBITMAP)(HBITMAP hBmp);
    HBITMAP  (*FI_CreateHBITMAPFromDIB)(FIBITMAP *dib);
    BOOL     (*FI_Premultiply)(HBITMAP hBmp);                               // premultiplies alpha channel for usage with AlphaBlend()
                                                                            // original HBITMAP stays valid and must be 32bit RGBA
    INT_PTR  (*FI_BmpFilterResizeBitmap)(WPARAM wParam,LPARAM lParam);      // more generic resizer for avatar images
    void     (*FI_CorrectBitmap32Alpha)(HBITMAP hBitmap, BOOL force);       // corrects broken images (when all alpha values are 0)

    FIMULTIBITMAP *(DLL_CALLCONV *FI_OpenMultiBitmapU)(FREE_IMAGE_FORMAT fif, const wchar_t *filename, BOOL create_new, BOOL read_only, BOOL keep_cache_in_memory FI_DEFAULT(FALSE), int flags FI_DEFAULT(0));

    void*   reserved[49];            // future usage
} FI_INTERFACE;


#if defined(UNICODE) || defined(_UNICODE)
#define FI_GetFIFFromFilenameT FI_GetFIFFromFilenameU
#define FI_GetFileTypeT FI_GetFileTypeU
#define FI_LoadT FI_LoadU
#define FI_OpenMultiBitmapT FI_OpenMultiBitmapU
#else
#define FI_GetFIFFromFilenameT FI_GetFIFFromFilename
#define FI_GetFileTypeT FI_GetFileType
#define FI_LoadT FI_Load
#define FI_OpenMultiBitmapT FI_OpenMultiBitmap
#endif

/*
 * image services
 *
 * only basic functionality is wrapped around Miranda services, because otherwise we would get a huge
 * load of services with complex parameter marshalling requirements.
 */

// get the interface version number
// wParam = lParam = 0
// returns FI_IF_VERSION

#define MS_IMG_GETIFVERSION "IMG/GetVersion"

// obtain the full FreeImage interface from the library. This interface provides full access to freeimage
// internal functions, thus enabling devs to fully utilize the FreeImage API. Only popular functions will
// be exported as miranda services.
// wParam = (DWORD)version Number // the caller MUST provide the desired version number
// lParam = **FI_INTERFACE
// returns S_OK on success, any other value indicates a problem.
// the interface is populated during the _DllMain() handler, so you can assume it is ready when Miranda
// calls the Load() handler of your plugin and you can return 1 in your Load() to disable your plugin when
// it depends on the freeimage interface and the freeimage plugin is missing
//
// example:
//
// FI_INTERFACE *fii = NULL;
//
// result = CallService(MS_IMG_GETINTERFACE, FI_IF_VERSION, (LPARAM)&fii);
//
// if(result != S_OK)
//     failed, in this case, fei will be NULL and your plugin will crash when using the interface,
//     so ALWAYS check it

#define MS_IMG_GETINTERFACE "IMG/GetInterface"


#define IMGL_RETURNDIB 1        // will NOT return a HBITMAP but a FIBITMAP * instead (useful, if you
                                // want to do further image manipulations before converting to a Win32 bitmap
                                // caller MUST then free the FIBITMAP * using fii->FI_Unload() or MS_IMG_UNLOAD (see below)

#define IMGL_WCHAR     2        // filename is wchar_t

#if defined(UNICODE) || defined(_UNICODE)
    #define IMGL_TCHAR  IMGL_WCHAR
#else
    #define IMGL_TCHAR  0
#endif

// load an image from disk
// wParam = full path and filename to the image
// lParam = IMGL_* flags
// returns a valid HBITMAP or 0 if image cannot be loaded
// if IMGL_RETURNDIB is set, it returns a pointer to a freeimage bitmap (FIBITMAP *)

#define MS_IMG_LOAD "IMG/Load"

/*
 * control structure for loading images from memory buffers (e.g. network buffers, memory mapped files).
 */

typedef struct _tagIMGSRVC_MEMIO {
    long iLen;					// length of the buffer
    void *pBuf;					// the buffer itself (you are responsible for allocating and free'ing it)
    FREE_IMAGE_FORMAT fif;		// -1 to detect the format or one of the FIF_* image format constants
    UINT flags;                 // flags to pass to FreeImage_LoadFromMemory()  (see freeimage docs)
} IMGSRVC_MEMIO;

// load an image from a memory buffer
// wParam = IMGSRVC_MEMIO *
// lParam = flags (see IMG/Load), valid are IMGL_RETURNDIB
// you must popupate iLen (buffer length) and pBuf (pointer to memory buffer)
// you must also specify the format in IMGSRVC_MEMIO.fif using one of the FIF_* constants defined in m_freeimage.h

#define MS_IMG_LOADFROMMEM "IMG/LoadFromMem"

// flags for IMGSRVC_INFO.dwMask

#define IMGI_FBITMAP  1  // the dib member is valid
#define IMGI_HBITMAP  2  // the hbm member is valid

/*
 * generic structure for various img functions
 * you must populate the fields as required, set the mask bits to indicate which member is valid
 */

typedef struct _tagIMGSRVC_INFO {
    DWORD cbSize;
    union {
        char *szName;
        wchar_t *wszName;
        TCHAR *tszName;
    };
    HBITMAP hbm;
    FIBITMAP *dib;
    DWORD    dwMask;
    FREE_IMAGE_FORMAT fif;
} IMGSRVC_INFO;

// save image to disk
// wParam = IMGSRVC_INFO *  (szName/wszName, hbm OR dib, cbSize, dwMask mandatory. fif optional, if FIF_UNKNOWN is given
//                           it will be determined from the filename).
// lParam = low word: IMG_* flags     (IMGL_WCHAR is the only valid - filename will be assumed to be wchar_t and wszName must be used)
//          high word: FreeImage_Save flags
// set IMGSRVC_INFO.dwMask to indicate whether the HBITMAP of FIBITMAP member is valid

#define MS_IMG_SAVE  "IMG/Save"

// unload a FIFBITMAP
// wParam = FIFBITMAP *
// lParam = 0;
// this service is useful when you have loaded a bitmap with IMGL_RETURNDIB in which case you do not get
// a HBITMAP but instead a FBITMAP * which describes the freeimage-internal representation of a bitmap.

#define MS_IMG_UNLOAD "IMG/Unload"

/*
 * resizer from loadavatars moved to image service plugin
*/

#define RESIZEBITMAP_STRETCH 0				// Distort bitmap to size in (max_width, max_height)
#define RESIZEBITMAP_KEEP_PROPORTIONS 1		// Keep bitmap proportions (probabily only one of the
											// max_width/max_height will be respected, and the other will be
											// smaller)
#define RESIZEBITMAP_CROP 2					// Keep bitmap proportions but crop it to fix exactly in (max_width, max_height)
											// Some image info outside will be lost
#define RESIZEBITMAP_MAKE_SQUARE 3			// Image will be allways square. Image will be croped and the size
											// returned will be min(max_width, max_height)

#define RESIZEBITMAP_FLAG_DONT_GROW	0x1000	// If set, the image will not grow. Else, it will grow to fit the max width/height

typedef struct {
	size_t size; // sizeof(ResizeBitmap);

	HBITMAP hBmp;

	int max_width;
	int max_height;

	int fit; // One of RESIZEBITMAP_* with an OR with RESIZEBITMAP_FLAG_DONT_GROW if needed
} ResizeBitmap;

// Returns a copy of the bitmap with the size especified or the original bitmap if nothing has to be changed
// wParam = ResizeBitmap *
// lParam = NULL
// return NULL on error, ResizeBitmap->hBmp if don't need to resize or a new HBITMAP if resized
// You are responsible for calling DestroyObject() on the original HBITMAP

#define MS_IMG_RESIZE "IMG/ResizeBitmap"


/*
 * format conversion helpers
 *
 * these helper macros allow converting HBITMAP to FIBITMAP * format and vice vera. In any case,
 * the caller is responsible for freeing or deleting the original object.
 * These macros wrap around the FI_CreateHBITMAPFromDib() and FI_CreateDIBFromHBITMAP() interface
 * functions.
 */

//#define FI_HBM2DIB(x) (FI_CreateDIBFromHBITMAP((x)))
//#define FI_DIB2HBM(x) (FI_CreateHBITMAPFromDIB((x)))

#endif // __M_IMGSRVC_H
