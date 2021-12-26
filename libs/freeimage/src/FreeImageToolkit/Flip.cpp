// ==========================================================
// Flipping routines
//
// Design and implementation by
// - Floris van den Berg (flvdberg@wxs.nl)
// - Hervé Drolon (drolon@infonie.fr)
// - Jim Keir (jimkeir@users.sourceforge.net)
//
// This file is part of FreeImage 3
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at your own risk!
// ==========================================================

#include "../stdafx.h"

/**
Flip the image horizontally along the vertical axis.
@param src Input image to be processed.
@return Returns TRUE if successful, FALSE otherwise.
*/
BOOL DLL_CALLCONV 
FreeImage_FlipHorizontal(FIBITMAP *src) {
	if (!FreeImage_HasPixels(src)) return FALSE;

	unsigned line   = FreeImage_GetLine(src);
	unsigned width	= FreeImage_GetWidth(src);
	unsigned height = FreeImage_GetHeight(src);

	unsigned bytespp = FreeImage_GetLine(src) / FreeImage_GetWidth(src);

	// copy between aligned memories
	uint8_t *new_bits = (uint8_t*)FreeImage_Aligned_Malloc(line * sizeof(uint8_t), FIBITMAP_ALIGNMENT);
	if (!new_bits) return FALSE;

	// mirror the buffer

	for (unsigned y = 0; y < height; y++) {
		uint8_t *bits = FreeImage_GetScanLine(src, y);
		memcpy(new_bits, bits, line);

		switch (FreeImage_GetBPP(src)) {
			case 1 :
			{				
				for(unsigned x = 0; x < width; x++) {
					// get pixel at (x, y)
					BOOL value = (new_bits[x >> 3] & (0x80 >> (x & 0x07))) != 0;
					// set pixel at (new_x, y)
					unsigned new_x = width - 1 - x;
					value ? bits[new_x >> 3] |= (0x80 >> (new_x & 0x7)) : bits[new_x >> 3] &= (0xff7f >> (new_x & 0x7));
				}
			}
			break;

			case 4 :
			{
				for(unsigned c = 0; c < line; c++) {
					bits[c] = new_bits[line - c - 1];

					uint8_t nibble = (bits[c] & 0xF0) >> 4;

					bits[c] = bits[c] << 4;
					bits[c] |= nibble;
				}
			}
			break;

			case 8:
			{				
				uint8_t *dst_data = (uint8_t*) bits; 				
				uint8_t *src_data = (uint8_t*) (new_bits + line - bytespp); 				
				for(unsigned c = 0; c < width; c++) { 			
					*dst_data++ = *src_data--;  
				} 
			}
			break;

			case 16:
			{				
				uint16_t *dst_data = (uint16_t*) bits; 				
				uint16_t *src_data = (uint16_t*) (new_bits + line - bytespp); 				
				for(unsigned c = 0; c < width; c++) { 			
					*dst_data++ = *src_data--;  
				} 
			}
			break;

			case 24 :
			case 32 :
			case 48:
			case 64:
			case 96:
			case 128:
			{				
				uint8_t *dst_data = (uint8_t*) bits; 				
				uint8_t *src_data = (uint8_t*) (new_bits + line - bytespp); 				
				for(unsigned c = 0; c < width; c++) { 		
					for(unsigned k = 0; k < bytespp; k++) {
						*dst_data++ = src_data[k];  
					}
					src_data -= bytespp;
				} 
			}
			break;

		}
	}

	FreeImage_Aligned_Free(new_bits);

	return TRUE;
}


/**
Flip the image vertically along the horizontal axis.
@param src Input image to be processed.
@return Returns TRUE if successful, FALSE otherwise.
*/

BOOL DLL_CALLCONV 
FreeImage_FlipVertical(FIBITMAP *src) {
	uint8_t *From, *Mid;

	if (!FreeImage_HasPixels(src)) return FALSE;

	// swap the buffer

	unsigned pitch  = FreeImage_GetPitch(src);
	unsigned height = FreeImage_GetHeight(src);

	// copy between aligned memories
	Mid = (uint8_t*)FreeImage_Aligned_Malloc(pitch * sizeof(uint8_t), FIBITMAP_ALIGNMENT);
	if (!Mid) return FALSE;

	From = FreeImage_GetBits(src);
	
	unsigned line_s = 0;
	unsigned line_t = (height-1) * pitch;

	for(unsigned y = 0; y < height/2; y++) {

		memcpy(Mid, From + line_s, pitch);
		memcpy(From + line_s, From + line_t, pitch);
		memcpy(From + line_t, Mid, pitch);

		line_s += pitch;
		line_t -= pitch;

	}

	FreeImage_Aligned_Free(Mid);

	return TRUE;
}

