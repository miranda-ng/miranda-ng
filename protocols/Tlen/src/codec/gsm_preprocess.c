/*

Tlen Protocol Plugin for Miranda NG
Copyright (C) 2004-2007  Piotr Piastucki

This program is based on GSM 06.10 source code developed by 
Jutta Degener and Carsten Bormann,
Copyright 1992, 1993, 1994 by Jutta Degener and Carsten Bormann,
Technische Universitaet Berlin 
  
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

*/
#include   "gsm.h"
#include   <stdio.h>

/*   4.2.0 .. 4.2.3   PREPROCESSING SECTION
 *
 *     After A-law to linear conversion (or directly from the
 *      Ato D converter) the following scaling is assumed for
 *    input to the RPE-LTP algorithm:
 *
 *
 *   Where S is the sign bit, v a valid bit, and * a "don't care" bit.
 *    The original signal is called sop[..]
 *
 */


void gsm_preprocessing( gsm_state *state, short *in, short *out)
{
	int		z1 = state->z1;
	int		z2 = state->z2;
	int		mp = state->mp;
	int		so, s1;
	int		msp, lsp;

	int		k = 160;

	while (k--) {
   /*  Downscaling of the input signal
 *      in:  0.1.....................12
 *        S.v.v.v.v.v.v.v.v.v.v.v.v.*.*.*
 *      out:   0.1................... 12
 *        S.S.v.v.v.v.v.v.v.v.v.v.v.v.0.0
 */
		so = (*in >> 3) << 2;
		in++;
   /*  4.2.2   Offset compensation
    *
    *  This part implements a high-pass filter and requires extended
    *  arithmetic precision for the recursive part of this filter.
    *  The input of this procedure is the array so[0...159] and the
    *  output the array sof[ 0...159 ].
    */
      /*   Compute the non-recursive part
       */
		s1 = (so - z1) << 15;
		z1 = so;
		msp = z2 >> 15;
		lsp = z2 - (msp	<< 15);
		s1 += ((lsp * 32735) + 16384) >> 15;
		z2 = msp * 32735 + s1;
   /*   4.2.3  Preemphasis
    */
		msp = (-28180 * mp + 16384) >> 15;
		mp = (z2 + 16384) >> 15;
		*out++ = mp + msp;
	}
	state->z1   = z1;
	state->z2 	= z2;
	state->mp   = mp;
/*
	printf("preprocessed: \n");
	for (k=0;k<160;k++) {
		printf("%7d ", out[k]);//((float)i_lpc[i])/32768.0f);
	}
	printf("\n");
	*/
}

void gsm_postprocessing( gsm_state *state, short *in, short *out)
{
	int k;
	int msr = state->msr;
	for (k = 160; k--; in++, out++) {
		msr = *in + ((msr * 28180 + 16384) >>15); 	   /* Deemphasis 	     */
		*out  = (msr + msr ) & 0xFFF8;  		   /* Truncation & Upscaling */
	}
	state->msr = msr;
}
