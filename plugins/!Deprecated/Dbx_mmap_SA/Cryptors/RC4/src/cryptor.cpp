#include <windows.h>

#define buf_size 1024

typedef struct{
	void* (__stdcall *GenerateKey)(char* key);
	void (__stdcall *FreeKey)(void* key);
	void (__stdcall *EncryptMem)(BYTE* data, int size, void* key);
	void (__stdcall *DecryptMem)(BYTE* data, int size, void* key);

    char* Name;
    char* Info;
    char* Author;
    char* Site;
    char* Email;

	DWORD Version;

	WORD uid;
} Cryptor;

typedef struct rc4_key
{
   BYTE state[256];
   BYTE x;
   BYTE y;
} rc4_key;

#define swap_byte(x,y) t = *(x); *(x) = *(y); *(y) = t

void prepare_key(BYTE *key_data_ptr, size_t key_data_len, rc4_key *key)
{
  int i = 0;
  BYTE t = 0;
  BYTE index1 = 0;
  BYTE index2 = 0;
  BYTE* state = 0;
  unsigned long counter = 0;

  state = &key->state[0];
  for(counter = 0; counter < 256; counter++)
  state[counter] = (BYTE)counter;
  key->x = 0;
  key->y = 0;
  index1 = 0;
  index2 = 0;
  for(counter = 0; counter < 256; counter++)
  {
    index2 = (key_data_ptr[index1] + state[counter] + index2) % 256;
    swap_byte(&state[counter], &state[index2]);
    index1 = (index1 + 1) % key_data_len;
  }
}

void rc4(BYTE *buffer_ptr, int buffer_len, rc4_key *key)
{
  BYTE t = 0;
  BYTE x = 0;
  BYTE y = 0;
  BYTE* state = 0;
  BYTE xorIndex = 0;
  DWORD counter = 0;
  BYTE old_state[256];

  x = key->x;
  y = key->y;
  state = &key->state[0];
  memcpy(old_state, key->state, 256);
  for(counter = 0; counter < (DWORD)buffer_len; counter++)
  {
    x = (x + 1) % 256;
    y = (state[x] + y) % 256;
    swap_byte(&state[x], &state[y]);
    xorIndex = (state[x] + state[y]) % 256;
    buffer_ptr[counter] ^= state[xorIndex];
  }
  memcpy(key->state, old_state, 256);
  //key->x = x;
  //key->y = y;
}


void zero_fill(BYTE * pBuf, size_t bufSize)
{
	size_t i;
	for(i = 0; i < bufSize; i++)
		pBuf[i] = 0;
}

void* __stdcall GenerateKey(char* pwd)
{
	rc4_key* key;
	key = (rc4_key*)malloc(sizeof(rc4_key));
	zero_fill((BYTE*)key, sizeof(key));
	prepare_key((BYTE*)pwd, strlen(pwd), key);
	return key;
}

void __stdcall FreeKey(void* key)
{
	free(key);
}

void __stdcall EncryptMem(BYTE* data, int size, void* key)
{
	rc4(data, size, (rc4_key*)key);
}

void __stdcall DecryptMem(BYTE* data, int size, void* key)
{
	rc4(data, size, (rc4_key*)key);
}

Cryptor cryptor =
	{
		GenerateKey,
		FreeKey,
		EncryptMem,
		DecryptMem,
		"RC4",
		"Old, very easy to crack stream cipher.",
		"Unknown",
		"http://ru.wikipedia.org/wiki/RC4",
		"nomail",
		0x00000100,
		0xDEAD
	};


extern "C" __declspec(dllexport) Cryptor* GetCryptor()
{
	return &cryptor;
}
