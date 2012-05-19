/*
 *  Off-the-Record Messaging Toolkit
 *  Copyright (C) 2004-2008  Ian Goldberg, Chris Alexander, Nikita Borisov
 *                           <otr@cypherpunks.ca>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public License as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __CTRMODE_H__
#define __CTRMODE_H__

/* Encrypt or decrypt data in AES-CTR mode.  (The operations are the
 * same.)  We roll our own here just to double-check that the calls
 * libotr makes to libgcrypt are doing the right thing. */
void aes_ctr_crypt(unsigned char *out, const unsigned char *in, size_t len,
	unsigned char key[16], unsigned char ctrtop[8]);

#endif
