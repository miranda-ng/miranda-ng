/*  
 * Skype Login
 * 
 * Based on:
 *   FakeSkype : Skype reverse engineering proof-of-concept client
 *               Ouanilo MEDEGAN (c) 2006   http://www.oklabs.net
 *   pyskype   : Skype login Python script by uunicorn
 *
 * Written by:   leecher@dose.0wnz.at (c) 2015 
 *
 * Module:       Object management functions
 *
 */
#include "common.h"
#include "objects.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>
#endif

void	WriteValue(uchar **BufferAddr, uint Value)
{
	uint		a;

	for (a = Value; a > 0x7F; a >>= 7, (*BufferAddr)++) **BufferAddr = (uchar)a | 0x80;
	**BufferAddr = (uchar) a;
	(*BufferAddr)++;
}

void	ReadValue(uchar **BufferAddr, uint *Value)
{
	uint		a=0;
	uchar		c;

	for (*Value=0, c=**BufferAddr; a==0 || (c & 0x80); a+=7, (*BufferAddr)++) *Value|=((c=**BufferAddr) & 0x7F) << a;
}


void	WriteObject(uchar **Buffer, ObjectDesc Object)
{
	int	IdxDown, IdxUp;

	Object.ObjListInfos.Id = -1;
	Object.ObjListInfos.Rank = 0;

	WriteValue(Buffer, Object.Family);
	WriteValue(Buffer, Object.Id);
	switch(Object.Family)
	{
	case OBJ_FAMILY_NBR:
		WriteValue(Buffer, Object.Value.Nbr);
		break;
	case OBJ_FAMILY_TABLE:
		IdxUp = 0;
		IdxDown = sizeof(Object.Value.Table) - 1;
		while (IdxDown >= 0)
			(*Buffer)[IdxDown--] = Object.Value.Table[IdxUp++];
		*Buffer += sizeof(Object.Value.Table);
		break;
	case OBJ_FAMILY_NETADDR:
		*(unsigned int *)(*Buffer) = inet_addr(Object.Value.Addr.ip);
		*Buffer += 4;
		*(unsigned short *)(*Buffer) = htons((u_short)Object.Value.Addr.port);
		*Buffer += 2;
		break;
	case OBJ_FAMILY_BLOB:
		WriteValue(Buffer, Object.Value.Memory.MsZ);
		memcpy(*Buffer, Object.Value.Memory.Memory, Object.Value.Memory.MsZ);
		*Buffer += Object.Value.Memory.MsZ;
		break;
	case OBJ_FAMILY_STRING:
		memcpy(*Buffer, Object.Value.Memory.Memory, Object.Value.Memory.MsZ);
		*Buffer += Object.Value.Memory.MsZ;
		*(*Buffer) = 0x00;
		*Buffer += 1;
		break;
	case OBJ_FAMILY_INTLIST:
	{
		uint	*IntList;

		IdxUp = 0;
		IntList = (uint *)Object.Value.Memory.Memory;
		WriteValue(Buffer, Object.Value.Memory.MsZ);
		while (IdxUp < Object.Value.Memory.MsZ)
		{
			WriteValue(Buffer, IntList[IdxUp]);
			IdxUp++;
		}
		break;
	}
	default:
		DBGPRINT("WriteObject : Unmanaged Object Family\n");
		break;
	}
}

int		DecodeRawObjects(uchar **Buffer, uint Size, SResponse *Response, ObjectDesc **Objs, int Suffix)
{
	int				NbObjs, lIdx, IdxUp, IdxDown;
	uint			Family, Id;
	uchar			*Str, *Mark;
	struct in_addr	IP;
	ObjectDesc		*Object;
	static int		Idx = 0;
	static int		Level = 0;
	static int		CurObjListId = -1;
	static uint		CurObjListRank = 0;
	uint			LocalObjListRank;

	LocalObjListRank = CurObjListRank;
	NbObjs = Family = Id = lIdx = 0;
	Mark = *Buffer;

	ReadValue(Buffer, &NbObjs);
	Level += 1;

	if (Level == 1)
		Idx += Suffix;

	while (lIdx < NbObjs)
	{
		Family = **Buffer;
		*Buffer += 1;
		ReadValue(Buffer, &Id);
		*Objs = (ObjectDesc *)realloc(*Objs, sizeof(ObjectDesc) * (Idx + 1));
		Object = &((*Objs)[Idx]);
		Object->Family = Family;
		Object->Id = Id;
		Object->ObjListInfos.Id = CurObjListId;
		Object->ObjListInfos.Rank = LocalObjListRank;

		switch (Family)
		{
		case OBJ_FAMILY_NBR:
			ReadValue(Buffer, &(Object->Value.Nbr));
			break;
		case OBJ_FAMILY_TABLE:
			IdxUp = 0;
			IdxDown = sizeof(Object->Value.Table) - 1;
			while (IdxDown >= 0)
				Object->Value.Table[IdxUp++] = (*Buffer)[IdxDown--];
			*Buffer += sizeof(Object->Value.Table);
			break;
		case OBJ_FAMILY_NETADDR:
#ifdef _WIN32
			IP.S_un.S_addr = *(unsigned int *)*Buffer;
#else
			IP.s_addr = *(unsigned int *)*Buffer;
#endif
			memset(Object->Value.Addr.ip, 0, MAX_IP_LEN + 1);
			strncpy(Object->Value.Addr.ip, inet_ntoa(IP), MAX_IP_LEN + 1);
			*Buffer += 4;
			Object->Value.Addr.port = htons(*(unsigned short *)(*Buffer));
			*Buffer += 2;
			break;
		case OBJ_FAMILY_BLOB:
			ReadValue(Buffer, &(Object->Value.Memory.MsZ));
			Object->Value.Memory.Memory = (uchar *)malloc(Object->Value.Memory.MsZ);
			memcpy(Object->Value.Memory.Memory, *Buffer, Object->Value.Memory.MsZ);
			*Buffer += Object->Value.Memory.MsZ;
			break;
		case OBJ_FAMILY_STRING:
			Str = *Buffer;
			Object->Value.Memory.MsZ = 1;
			while (*Str++ != 0)
				Object->Value.Memory.MsZ += 1;
			Object->Value.Memory.Memory = (uchar *)malloc(Object->Value.Memory.MsZ);
			memcpy(Object->Value.Memory.Memory, *Buffer, Object->Value.Memory.MsZ);
			*Buffer += Object->Value.Memory.MsZ;
			break;
		case OBJ_FAMILY_INTLIST:
			ReadValue(Buffer, &(Object->Value.Memory.MsZ));
			Object->Value.Memory.Memory = (uchar *)malloc(Object->Value.Memory.MsZ * sizeof(uint));
			for (IdxUp = 0; IdxUp < Object->Value.Memory.MsZ; IdxUp++)
				ReadValue(Buffer, (uint*)(Object->Value.Memory.Memory + (IdxUp * sizeof(uint))));
			break;
		case OBJ_FAMILY_OBJLIST:
		{
			uint	OldNbObj;
			int		OldCurObjListId;

			OldCurObjListId = CurObjListId;
			CurObjListId = (int)Id;
			CurObjListRank += 1;
			OldNbObj = Response->NbObj;
			ManageObjects(Buffer, Size - (*Buffer - Mark), Response);
			CurObjListId = OldCurObjListId;
			//Suffix += Response->NbObj - OldNbObj;
			NbObjs -= 1;
			//Idx += (Response->NbObj - OldNbObj);
			goto ContinueDecode;
			break;
		}
		default :
			break;
		}
		Idx += 1;
		lIdx += 1;
ContinueDecode:
		continue ;
	}

	Level -= 1;
	if (Level == 0)
	{
		Idx = 0;
		CurObjListId = -1;
		CurObjListRank = 0;
	}
	return (NbObjs);
}

int		ManageObjects(uchar **Buffer, uint Size, SResponse *Response)
{
	uchar	Mode;

	Mode = **Buffer;
	*Buffer += 1;
	switch (Mode)
	{
	case RAW_PARAMS:
		Response->NbObj += DecodeRawObjects(Buffer, Size - 1, Response, &(Response->Objs), Response->NbObj);
		return 0;
	case EXT_PARAMS:
		// Not supported as not needed by plain login
		//Response->NbObj += DecodeExtObjects(Buffer, Size - 1, Response, &(Response->Objs), Response->NbObj);
		break;
	default:
		break;
	}
	return -1;
}

void FreeResponse(SResponse *Response)
{
	uint Idx;

	for (Idx=0; Idx<Response->NbObj; Idx++)
	{
		switch (Response->Objs[Idx].Family)
		{
		case OBJ_FAMILY_BLOB:
		case OBJ_FAMILY_STRING:
		case OBJ_FAMILY_INTLIST:
			if (Response->Objs[Idx].Value.Memory.Memory) free (Response->Objs[Idx].Value.Memory.Memory);
			break;
		}
	}
	free (Response->Objs);
	memset(Response, 0, sizeof(SResponse));
}
