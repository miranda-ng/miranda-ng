// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2008 Joe Kucera
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------------

#ifndef __TLV_H
#define __TLV_H

/*---------* Structures *--------------*/

struct oscar_tlv
{
  WORD wType;
  WORD wLen;
  BYTE *pData;
};


struct oscar_tlv_chain
{
  oscar_tlv tlv;
  oscar_tlv_chain *next;

  WORD getChainLength();

  oscar_tlv* getTLV(WORD wType, WORD wIndex);
  oscar_tlv* putTLV(WORD wType, WORD wLen, BYTE *pData, BOOL bReplace);
  oscar_tlv_chain* removeTLV(oscar_tlv *tlv);
  WORD getLength(WORD wType, WORD wIndex);

  DWORD getDWord(WORD wType, WORD wIndex);
  WORD getWord(WORD wType, WORD wIndex);
  BYTE getByte(WORD wType, WORD wIndex);
  int getNumber(WORD wType, WORD wIndex);
  double getDouble(WORD wType, WORD wIndex);
  char* getString(WORD wType, WORD wIndex);
};


struct oscar_tlv_record_list
{
  oscar_tlv_chain *item;
  oscar_tlv_record_list *next;

  oscar_tlv_chain* getRecordByTLV(WORD wType, int nValue);
};

/*---------* Functions *---------------*/

oscar_tlv_chain* readIntoTLVChain(BYTE **buf, WORD wLen, int maxTlvs);
void disposeChain(oscar_tlv_chain** chain);

oscar_tlv_record_list* readIntoTLVRecordList(BYTE **buf, WORD wLen, int nCount);
void disposeRecordList(oscar_tlv_record_list** list);


#endif /* __TLV_H */
