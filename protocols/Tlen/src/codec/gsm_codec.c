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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GSM_MAGIC 0xD

gsm_state * gsm_create() 
{
	gsm_state * state = (gsm_state *)malloc(sizeof(gsm_state));
	memset(state, 0, sizeof(gsm_state));
	state->nrp = 40;
	return state;
}

void gsm_release(gsm_state *ptr)
{
	free(ptr);
}

void gsm_encode(gsm_state *state, short *in)
{	int i, j;
	unsigned char *c = state->gsmFrame;
	short temp_in[160];

	gsm_preprocessing(state, in, temp_in);
	shortTermAnalysis(state, temp_in);
	for (i = 0; i < 4; i++) {
		state->subframe = i;
		longTermAnalysis(state, temp_in + 40 * i);
		encodeRPE(state);
        for (j = 0; j < 40; j++) {
			int ii = (int) state->dp0[120 + i * 40 + j] + state->e[i * 40 + j];
			if (ii<-32768) ii = -32768;
			else if (ii>32767) ii=32767;
	        state->dp0[120 + i * 40 + j] = ii;
		}
	}
	memcpy(state->dp0, state->dp0 + 160, 120 * sizeof(short));
	state->j ^= 1;
	*c++ =   ((GSM_MAGIC & 0xF) << 4)
	       | ((state->qLARs[0] >> 2) & 0xF);
	*c++ =   ((state->qLARs[0] & 0x3) << 6)
	       | (state->qLARs[1] & 0x3F);
	*c++ =   ((state->qLARs[2] & 0x1F) << 3)
	       | ((state->qLARs[3] >> 2) & 0x7);
	*c++ =   ((state->qLARs[3] & 0x3) << 6)
	       | ((state->qLARs[4] & 0xF) << 2)
	       | ((state->qLARs[5] >> 2) & 0x3);
	*c++ =   ((state->qLARs[5] & 0x3) << 6)
	       | ((state->qLARs[6] & 0x7) << 3)
	       | (state->qLARs[7] & 0x7);
	*c++ =   ((state->Nc[0] & 0x7F) << 1)
	       | ((state->bc[0] >> 1) & 0x1);
	*c++ =   ((state->bc[0] & 0x1) << 7)
	       | ((state->Mc[0] & 0x3) << 5)
	       | ((state->xMaxc[0] >> 1) & 0x1F);
	*c++ =   ((state->xMaxc[0] & 0x1) << 7)
	       | ((state->xMc[0] & 0x7) << 4)
	       | ((state->xMc[1] & 0x7) << 1)
	       | ((state->xMc[2] >> 2) & 0x1);
	*c++ =   ((state->xMc[2] & 0x3) << 6)
	       | ((state->xMc[3] & 0x7) << 3)
	       | (state->xMc[4] & 0x7);
	*c++ =   ((state->xMc[5] & 0x7) << 5)
	       | ((state->xMc[6] & 0x7) << 2)
	       | ((state->xMc[7] >> 1) & 0x3);
	*c++ =   ((state->xMc[7] & 0x1) << 7)
	       | ((state->xMc[8] & 0x7) << 4)
	       | ((state->xMc[9] & 0x7) << 1)
	       | ((state->xMc[10] >> 2) & 0x1);
	*c++ =   ((state->xMc[10] & 0x3) << 6)
	       | ((state->xMc[11] & 0x7) << 3)
	       | (state->xMc[12] & 0x7);
	*c++ =   ((state->Nc[1] & 0x7F) << 1)
	       | ((state->bc[1] >> 1) & 0x1);
	*c++ =   ((state->bc[1] & 0x1) << 7)
	       | ((state->Mc[1] & 0x3) << 5)
	       | ((state->xMaxc[1] >> 1) & 0x1F);
	*c++ =   ((state->xMaxc[1] & 0x1) << 7)
	       | ((state->xMc[13] & 0x7) << 4)
	       | ((state->xMc[14] & 0x7) << 1)
	       | ((state->xMc[15] >> 2) & 0x1);
	*c++ =   ((state->xMc[15] & 0x3) << 6)
	       | ((state->xMc[16] & 0x7) << 3)
	       | (state->xMc[17] & 0x7);
	*c++ =   ((state->xMc[18] & 0x7) << 5)
	       | ((state->xMc[19] & 0x7) << 2)
	       | ((state->xMc[20] >> 1) & 0x3);
	*c++ =   ((state->xMc[20] & 0x1) << 7)
	       | ((state->xMc[21] & 0x7) << 4)
	       | ((state->xMc[22] & 0x7) << 1)
	       | ((state->xMc[23] >> 2) & 0x1);
	*c++ =   ((state->xMc[23] & 0x3) << 6)
	       | ((state->xMc[24] & 0x7) << 3)
	       | (state->xMc[25] & 0x7);
	*c++ =   ((state->Nc[2] & 0x7F) << 1)
	       | ((state->bc[2] >> 1) & 0x1);
	*c++ =   ((state->bc[2] & 0x1) << 7)
	       | ((state->Mc[2] & 0x3) << 5)
	       | ((state->xMaxc[2] >> 1) & 0x1F);
	*c++ =   ((state->xMaxc[2] & 0x1) << 7)
	       | ((state->xMc[26] & 0x7) << 4)
	       | ((state->xMc[27] & 0x7) << 1)
	       | ((state->xMc[28] >> 2) & 0x1);
	*c++ =   ((state->xMc[28] & 0x3) << 6)
	       | ((state->xMc[29] & 0x7) << 3)
	       | (state->xMc[30] & 0x7);
	*c++ =   ((state->xMc[31] & 0x7) << 5)
	       | ((state->xMc[32] & 0x7) << 2)
	       | ((state->xMc[33] >> 1) & 0x3);
	*c++ =   ((state->xMc[33] & 0x1) << 7)
	       | ((state->xMc[34] & 0x7) << 4)
	       | ((state->xMc[35] & 0x7) << 1)
	       | ((state->xMc[36] >> 2) & 0x1);
	*c++ =   ((state->xMc[36] & 0x3) << 6)
	       | ((state->xMc[37] & 0x7) << 3)
	       | (state->xMc[38] & 0x7);
	*c++ =   ((state->Nc[3] & 0x7F) << 1)
	       | ((state->bc[3] >> 1) & 0x1);
	*c++ =   ((state->bc[3] & 0x1) << 7)
	       | ((state->Mc[3] & 0x3) << 5)
	       | ((state->xMaxc[3] >> 1) & 0x1F);
	*c++ =   ((state->xMaxc[3] & 0x1) << 7)
	       | ((state->xMc[39] & 0x7) << 4)
	       | ((state->xMc[40] & 0x7) << 1)
	       | ((state->xMc[41] >> 2) & 0x1);
	*c++ =   ((state->xMc[41] & 0x3) << 6)
	       | ((state->xMc[42] & 0x7) << 3)
	       | (state->xMc[43] & 0x7);
	*c++ =   ((state->xMc[44] & 0x7) << 5)
	       | ((state->xMc[45] & 0x7) << 2)
	       | ((state->xMc[46] >> 1) & 0x3);
	*c++ =   ((state->xMc[46] & 0x1) << 7)
	       | ((state->xMc[47] & 0x7) << 4)
	       | ((state->xMc[48] & 0x7) << 1)
	       | ((state->xMc[49] >> 2) & 0x1);
	*c++ =   ((state->xMc[49] & 0x3) << 6)
	       | ((state->xMc[50] & 0x7) << 3)
	       | (state->xMc[51] & 0x7);
}

void gsm_decode(gsm_state *state, short *out)
{
    int i;
	unsigned char *c = state->gsmFrame;

	if (((*c >> 4) & 0x0F) != GSM_MAGIC) return ;
	state->qLARs[0]  = (*c++ & 0xF) << 2;
	state->qLARs[0] |= (*c >> 6) & 0x3;
	state->qLARs[1]  = *c++ & 0x3F;
	state->qLARs[2]  = (*c >> 3) & 0x1F;
	state->qLARs[3]  = (*c++ & 0x7) << 2;
	state->qLARs[3] |= (*c >> 6) & 0x3;
	state->qLARs[4]  = (*c >> 2) & 0xF;
	state->qLARs[5]  = (*c++ & 0x3) << 2;
	state->qLARs[5] |= (*c >> 6) & 0x3;
	state->qLARs[6]  = (*c >> 3) & 0x7;
	state->qLARs[7]  = *c++ & 0x7;
	state->Nc[0]  = (*c >> 1) & 0x7F;
	state->bc[0]  = (*c++ & 0x1) << 1;
	state->bc[0] |= (*c >> 7) & 0x1;
	state->Mc[0]  = (*c >> 5) & 0x3;
	state->xMaxc[0]  = (*c++ & 0x1F) << 1;
	state->xMaxc[0] |= (*c >> 7) & 0x1;
	state->xMc[0]  = (*c >> 4) & 0x7;
	state->xMc[1]  = (*c >> 1) & 0x7;
	state->xMc[2]  = (*c++ & 0x1) << 2;
	state->xMc[2] |= (*c >> 6) & 0x3;
	state->xMc[3]  = (*c >> 3) & 0x7;
	state->xMc[4]  = *c++ & 0x7;
	state->xMc[5]  = (*c >> 5) & 0x7;
	state->xMc[6]  = (*c >> 2) & 0x7;
	state->xMc[7]  = (*c++ & 0x3) << 1;
	state->xMc[7] |= (*c >> 7) & 0x1;
	state->xMc[8]  = (*c >> 4) & 0x7;
	state->xMc[9]  = (*c >> 1) & 0x7;
	state->xMc[10]  = (*c++ & 0x1) << 2;
	state->xMc[10] |= (*c >> 6) & 0x3;
	state->xMc[11]  = (*c >> 3) & 0x7;
	state->xMc[12]  = *c++ & 0x7;
	state->Nc[1]  = (*c >> 1) & 0x7F;
	state->bc[1]  = (*c++ & 0x1) << 1;
	state->bc[1] |= (*c >> 7) & 0x1;
	state->Mc[1]  = (*c >> 5) & 0x3;
	state->xMaxc[1]  = (*c++ & 0x1F) << 1;
	state->xMaxc[1] |= (*c >> 7) & 0x1;
	state->xMc[13]  = (*c >> 4) & 0x7;
	state->xMc[14]  = (*c >> 1) & 0x7;
	state->xMc[15]  = (*c++ & 0x1) << 2;
	state->xMc[15] |= (*c >> 6) & 0x3;
	state->xMc[16]  = (*c >> 3) & 0x7;
	state->xMc[17]  = *c++ & 0x7;
	state->xMc[18]  = (*c >> 5) & 0x7;
	state->xMc[19]  = (*c >> 2) & 0x7;
	state->xMc[20]  = (*c++ & 0x3) << 1;
	state->xMc[20] |= (*c >> 7) & 0x1;
	state->xMc[21]  = (*c >> 4) & 0x7;
	state->xMc[22]  = (*c >> 1) & 0x7;
	state->xMc[23]  = (*c++ & 0x1) << 2;
	state->xMc[23] |= (*c >> 6) & 0x3;
	state->xMc[24]  = (*c >> 3) & 0x7;
	state->xMc[25]  = *c++ & 0x7;
	state->Nc[2]  = (*c >> 1) & 0x7F;
	state->bc[2]  = (*c++ & 0x1) << 1;
	state->bc[2] |= (*c >> 7) & 0x1;
	state->Mc[2]  = (*c >> 5) & 0x3;
	state->xMaxc[2]  = (*c++ & 0x1F) << 1;
	state->xMaxc[2] |= (*c >> 7) & 0x1;
	state->xMc[26]  = (*c >> 4) & 0x7;
	state->xMc[27]  = (*c >> 1) & 0x7;
	state->xMc[28]  = (*c++ & 0x1) << 2;
	state->xMc[28] |= (*c >> 6) & 0x3;
	state->xMc[29]  = (*c >> 3) & 0x7;
	state->xMc[30]  = *c++ & 0x7;
	state->xMc[31]  = (*c >> 5) & 0x7;
	state->xMc[32]  = (*c >> 2) & 0x7;
	state->xMc[33]  = (*c++ & 0x3) << 1;
	state->xMc[33] |= (*c >> 7) & 0x1;
	state->xMc[34]  = (*c >> 4) & 0x7;
	state->xMc[35]  = (*c >> 1) & 0x7;
	state->xMc[36]  = (*c++ & 0x1) << 2;
	state->xMc[36] |= (*c >> 6) & 0x3;
	state->xMc[37]  = (*c >> 3) & 0x7;
	state->xMc[38]  = *c++ & 0x7;
	state->Nc[3]  = (*c >> 1) & 0x7F;
	state->bc[3]  = (*c++ & 0x1) << 1;
	state->bc[3] |= (*c >> 7) & 0x1;
	state->Mc[3]  = (*c >> 5) & 0x3;
	state->xMaxc[3]  = (*c++ & 0x1F) << 1;
	state->xMaxc[3] |= (*c >> 7) & 0x1;
	state->xMc[39]  = (*c >> 4) & 0x7;
	state->xMc[40]  = (*c >> 1) & 0x7;
	state->xMc[41]  = (*c++ & 0x1) << 2;
	state->xMc[41] |= (*c >> 6) & 0x3;
	state->xMc[42]  = (*c >> 3) & 0x7;
	state->xMc[43]  = *c++ & 0x7;
	state->xMc[44]  = (*c >> 5) & 0x7;
	state->xMc[45]  = (*c >> 2) & 0x7;
	state->xMc[46]  = (*c++ & 0x3) << 1;
	state->xMc[46] |= (*c >> 7) & 0x1;
	state->xMc[47]  = (*c >> 4) & 0x7;
	state->xMc[48]  = (*c >> 1) & 0x7;
	state->xMc[49]  = (*c++ & 0x1) << 2;
	state->xMc[49] |= (*c >> 6) & 0x3;
	state->xMc[50]  = (*c >> 3) & 0x7;
	state->xMc[51]  = *c & 0x7;

	for (i = 0; i < 4; i++) {
		state->subframe = i;
		decodeRPE(state);
		longTermSynthesis(state);
	}
	memcpy(state->dp0, state->dp0 + 160, 120 * sizeof(short));
	shortTermSynthesis(state, &state->dp0[120], out);
	gsm_postprocessing(state, out, out);
	state->j ^= 1;
}
