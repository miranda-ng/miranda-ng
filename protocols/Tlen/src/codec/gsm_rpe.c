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
#include "gsm.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static const int gsm_H[11] = {-134, -374, 0, 2054, 5741, 8192, 5741, 2054, 0, -374, -134 };
static const int gsm_NRFAC[8] = { 29128, 26215, 23832, 21846, 20165, 18725, 17476, 16384 };
static const int gsm_FAC[8]   = { 18431, 20479, 22527, 24575, 26623, 28671, 30719, 32767 };
/*
 *  The coefficients of the weighting filter are stored in the gsm_H table.
 *  The following scaling is used:
 *
 *	gsm_H[0..10] = integer( real_H[ 0..10] * 8192 );
 */
static void weightingFilter (int *e, int *x)
{
	int	L_result;
	int	k, i;
	for (k = 0; k < 40; k++) {
		L_result = 4096;
		for (i = 0; i < 11; i++) {
			int ix = i + k;
			if (ix>4 && ix<45) L_result += e[ix - 5] * gsm_H[i];
		}
		L_result >>= 13;
		x[k] =  (L_result < -32768 ? -32768 : (L_result > 32767 ? 32767 : L_result));
	}
}

/*
 *  The signal x[0..39] is used to select the RPE grid which is
 *  represented by Mc.
 */
static void gridSelection( int *x, int *xM, int *Mc_out)
{
	int		m, i;
	int 	L_result, L_temp;
	int 	EM, Mc;

	EM = 0;
	Mc = 0;

	for (m = 0; m < 4; m++) {
		L_result = 0;
		for (i = 0; i < 13; i++) {
			int temp = x[m + 3*i] >> 2;
			L_temp = temp * temp << 1;
			L_result += L_temp;
		}
		if (L_result > EM) {
			Mc = m;
			EM = L_result;
		}
	}
	/*  Down-sampling by a factor 3 to get the selected xM[0..12]
	 *  RPE sequence.
	 */
	for (i = 0; i < 13; i ++) xM[i] = x[Mc + 3*i];
	*Mc_out = Mc;
}

/*
 *  This procedure computes the reconstructed long term residual signal
 *  ep[0..39] for the LTP analysis filter.  The inputs are the Mc
 *  which is the grid position selection and the xMp[0..12] decoded
 *  RPE samples which are upsampled by a factor of 3 by inserting zero
 *  values.
 */
static void gridPositioning (int Mc, int *xMp, int *ep)
{
	int i, k;
	for (k = 0; k < 40; k++) ep[k] = 0;
	for (i = 0; i < 13; i++) {
		ep[ Mc + (3*i) ] = xMp[i];
	}
}

static void APCMXmaxcToExpMant (int xmaxc, int *exp_out, int *mant_out )
{
	int 	exp, mant;
	/* Compute exponent and mantissa of the decoded version of xmaxc
	 */
	exp = 0;
	if (xmaxc > 15) exp = (xmaxc >> 3) - 1;
	mant = xmaxc - (exp << 3);

	if (mant == 0) {
		exp  = -4;
		mant = 7;
	}
	else {
		while (mant < 8) {
			mant = mant << 1 | 1;
			exp--;
		}
		mant -= 8;
	}

	*exp_out  = exp;
	*mant_out = mant;
}

static void APCMQuantization (int *xM, int *xMc, int *mant_out, int *exp_out, int *xmaxc_out)
{
	int		i, itest;
	int		xmax, xmaxc, temp, temp1, temp2;
	int		exp, mant;


	/*  Find the maximum absolute value xmax of xM[0..12].
	 */

	xmax = 0;
	for (i = 0; i < 13; i++) {
		temp = abs(xM[i]);
		if (temp > xmax) xmax = temp;
	}
	if (xmax > 32767) xmax = 32767;
	/*  Qantizing and coding of xmax to get xmaxc.
	 */

	exp   = 0;
	temp  = xmax >> 9;
	itest = 0;

	for (i = 0; i < 6; i++) {
		if (temp != 0) exp++;
		temp = temp >> 1;
	}

	temp = exp + 5;

	xmaxc = (xmax >> temp) + (exp << 3);

	/*   Quantizing and coding of the xM[0..12] RPE sequence
	 *   to get the xMc[0..12]
	 */
	APCMXmaxcToExpMant( xmaxc, &exp, &mant );

	/*  This computation uses the fact that the decoded version of xmaxc
	 *  can be calculated by using the exponent and the mantissa part of
	 *  xmaxc (logarithmic table).
	 *  So, this method avoids any division and uses only a scaling
	 *  of the RPE samples by a function of the exponent.  A direct
	 *  multiplication by the inverse of the mantissa (NRFAC[0..7]
	 *  found in table 4.5) gives the 3 bit coded version xMc[0..12]
	 *  of the RPE samples.
	 */

	/* Direct computation of xMc[0..12] using table 4.5
	 */
	temp = (mant|8)<<(5+exp);
	temp1 = 6 - exp;
	temp2 = gsm_NRFAC[ mant ];
	for (i = 0; i < 13; i++) {
		xMc[i] = ((xM[i] << temp1) * temp2 >> 27) + 4;
	}

	/*  NOTE: This equation is used to make all the xMc[i] positive.
	 */

	*mant_out  = mant;
	*exp_out   = exp;
	*xmaxc_out = xmaxc;
}

static void APCMDequantization (int *xMc, int mant, int exp, int *xMp)
{
   	int		i, temp1, temp2, temp3;
	temp1 = gsm_FAC[ mant ];
	temp2 = 6 - exp;
	if (temp2 <= 0) {
		temp3 = 1 >> (1 - temp2);
	} else {
		temp3 = 1 << (temp2 - 1);
	}
	for (i = 0; i < 13; i++) {
		xMp[i] = ((((((xMc[i]<<1)-7)<<12)*temp1+16384)>>15)+temp3)>>temp2;
	}
}

void encodeRPE(gsm_state *state)
{   int		x[40];
	int		xM[13], xMp[13];
	int		mant, exp;
	int     *Mc = state->Mc + state->subframe;
	int     *xMaxc = state->xMaxc + state->subframe;
	int 	*xMc = state->xMc + state->subframe * 13;
	int     *e = state->e + state->subframe * 40;
	//int		i;
	/*
	printf("RPE in: \n");
	for (i=0;i<40;i++) {
		printf("%7d ", e[i]);
	}
	printf("\n");
	*/
	weightingFilter(e, x);
	/*
	printf("RPE weighting filter: \n");
	for (i=0;i<40;i++) {
		printf("%7d ", x[i]);
	}
	printf("\n");
	*/
	gridSelection(x, xM, Mc);
	APCMQuantization(xM, xMc, &mant, &exp, xMaxc);
	/* printf("RPE Mc(grid #)=%d xmaxc=%d mant=%d exp=%d \n", *Mc, *xMaxc, mant, exp); */
	APCMDequantization(xMc, mant, exp, xMp);
	gridPositioning(*Mc, xMp, e);
}

void decodeRPE(gsm_state *state)
{
	int		exp, mant;
	int		xMp[ 13 ];
	int 	*xMc = state->xMc + state->subframe * 13;
	int     *e = state->e + state->subframe * 40;

	APCMXmaxcToExpMant(state->xMaxc[state->subframe], &exp, &mant);
	APCMDequantization(xMc, mant, exp, xMp);
	gridPositioning(state->Mc[state->subframe], xMp, e);
}
