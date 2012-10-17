/*

Tlen Protocol Plugin for Miranda NG
Copyright (C) 2004  Piotr Piastucki

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
#include <windows.h>

#define P_MAX   8   /* order p of LPC analysis, typically 8..14 */

typedef struct {
	int			LARpp[2][8]; /* LAR coefficients for previous and current frame */
	int			j;			 /* current frame index in LARpp[j][]   */
	int	  		qLARs[8];    /* quantized LARs for current frame */
	int 		u[8];		 /* used by short term analysis filter */
	int			v[9];		 /* used by short term synthesis filter */
	int         subframe;    /* subframe index (0..3)*/
	short		dp0[ 280 ];  /* previous short-term filter output/prediction, used by long term analysis*/
	int			e[160];		 /* errors (residuals) */
	int			nrp; 		 /* long term synthesis */
	int			Nc[4], bc[4]; /* long-term lag and gain for current frame */
	int			Mc[4];	 	 /* RPE grid index (0..3) */
	int			xMaxc[4];	 /* RPE maximum error quantized and encoded (mantissa + exp) */
	int			xMc[52];	 /* quantized residuals (errors) 4 * 13 */

	int			z1;			 /* preprocessing.c, Offset_com. */
	int			z2;			/*                  Offset_com. */
	int			mp;			/*                  Preemphasis	*/
	int			msr;		/* decoder.c,	Postprocessing	*/

	unsigned char gsmFrame[33];
}gsm_state;

extern gsm_state * gsm_create() ;
extern void gsm_release(gsm_state *ptr);
extern void gsm_encode(gsm_state *state, short *in);
extern void gsm_decode(gsm_state *state, short *out);

extern void gsm_preprocessing( gsm_state *state, short *in, short *out);
extern void shortTermAnalysis(gsm_state *state, short *in);
extern void longTermAnalysis(gsm_state *state, short *in);
extern void encodeRPE(gsm_state *state);
extern void decodeRPE(gsm_state *state);
extern void longTermSynthesis(gsm_state *state);
extern void shortTermSynthesis(gsm_state *state, short *in, short *out);
extern void gsm_postprocessing( gsm_state *state, short *in, short *out);
