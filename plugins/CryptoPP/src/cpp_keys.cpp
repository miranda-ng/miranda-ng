#include "commonheaders.h"

const unsigned char IV[] = "PSKhell@MIRANDA!";

// generate KeyA pair, return public key as ASCII
LPSTR __cdecl cpp_init_keya(HANDLE context, int features)
{
	pCNTX ptr = get_context_on_id(context); if (!ptr) return NULL;
	pSIMDATA p = (pSIMDATA)cpp_alloc_pdata(ptr);

	int send_features = FEATURES;
	if (p->KeyP) send_features |= FEATURES_PSK;

	SAFE_DELETE(p->dh);
	if (features & FEATURES_NEWPG) {
		Integer p0("0x865734026113B4DF0082CED84C197718516088FBDA406CFDFD7F033694E11E46F01C8F01E0E5AE6B09F6284691C7DD30A5BA8A74BA4B780198624B84BC8DAF6E0DFF874C0440ABB5C043C82E9E9C9E6F1A470B6A2A6BCEAC9460C43B1BB1331DF0FFD898DB74D22E8A71DB2659F1B0F52F337718D233DED611DA25AEAA90F3BE0C42FA9E541D0487DF58A77E2F44D295AD0C54C369CE260C969CA12F690EAAFAEEF8676C631AF29A3DE3389D3000B94EFA775E31FCA879AEB00A4D05EEF50D4C084A049EB12EF4CDFBD48E36B29CEAF8978D535D6C70BB274D1FEA02ABD521D2EF482A76326C17AF597FCB9B8BF37D9110E22AB0746D6A9779DF5133822E3F15");
		Integer q0("0xF1515160E1BFC7636338C13AD5BA775318E287147A1F96B73CF0FB4D97EFFB9D1FCDCF31AB9D92C4F49C9F8D50F06E697D2313E2EBAC7781312A51F458D66FFC687960CAA86BDF150A36ED53D79FBDB4F501FD25E37C181B45F9555D7F1C6124CAB29A822AD1E7BF5DA93C2FDB12A61919B5E5359793CBB16E71516919040A7F");
		Integer g0("0x434158F2FF2CF667E3CC1A707770DDE7B56048F6C5005473C6DC4A5E3FC490667F2908C44F9ADC2071EB8A8A1EC6AD6901CDAAAFE806626E3F4C5E69D4FCBF9DF1B67D574DC61C687C4BEF3ACF77A7256752F4D3A7EAEEE5874ED4912AB10C5B122A24E698B14CAC3E0FD006E0DB02714E028AECCA25BAB9DDCA2CF6405E862B403C61BC381D64B46BD96D3FEFBE22B7497FCBDA2B49C840C1B2853502A5216B036F83D2EAD8F835B603FC5BA1EFB816C7634915B1D43963FDD1B1ED626F48CCF8F152392E4693D38C566016708FA9E9598AECF95A8B067212669247552418E538A706564F855D3D5D50717D356259A20D8FD0C6B2A9FCC045D3F7AED1E857C5");
		p->dh = new DH(p0, q0, g0);
	}
	else {
		Integer p0("0xD494AAFBCD2EAC6A36DB8E7DD4A2A64512A5BBB15B9BFB581C7C1CAFB647D4612973C3770C2166D75EEA695F67EA8261557591DB78BCF5A886AA5294F3AEE4D25B57C8EE8C7FE8DBF70C132CD7FFCB6F89426F807F552C5DAE2FB1F329E340094E4B30D8EF6265AB4D350E9837B151C86AC524DE4E1FC04746C668BE318275E420D51AEDDFBDF887D435CDEEF6AC81293DB45287132F8236A43AD8F4D6642D7CA6732DA06A1DE008259008C9D74403B68ADAC788CF8AB5BEFFC310DCCCD32901D1F290E5B7A993D2CF6A652AF81B6DA0FD2E70678D1AE086150E41444522F20621195AD2A1F0975652B4AF7DE5261A9FD46B9EA8B443641F3BBA695B9B020103");
		Integer g0("0x12A567BC9ABCDEF1234567823BCDEF1E");
		p->dh = new DH(p0, g0);
	}

	BYTE priv1[KEYSIZE];	// private key of 2048 bit
	BYTE publ1[KEYSIZE + 2];	// public key of 2048 bit + faetures field

	memset(priv1, 0, sizeof(priv1));
	memset(publ1, 0, sizeof(publ1));

	AutoSeededRandomPool autorng;
	p->dh->GenerateKeyPair(autorng, priv1, publ1);

	SAFE_FREE(p->PubA);
	p->PubA = (PBYTE)malloc(KEYSIZE);
	memcpy(p->PubA, publ1, KEYSIZE);

	SAFE_FREE(p->KeyA);
	p->KeyA = (PBYTE)malloc(KEYSIZE);
	memcpy(p->KeyA, priv1, KEYSIZE);

	if (p->KeyP) {
		// encrypt PUBLIC use PSK
		string ciphered;
		CFB_Mode<AES>::Encryption enc(p->KeyP, Tiger::DIGESTSIZE, IV);
		StreamTransformationFilter cbcEncryptor(enc, new StringSink(ciphered));

		cbcEncryptor.Put(publ1, KEYSIZE);
		cbcEncryptor.MessageEnd();
		memcpy(publ1, ciphered.data(), ciphered.length());
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("cpp_init_keya: %d %d", KEYSIZE, ciphered.length());
#endif
	}
	memcpy((PVOID)&publ1[KEYSIZE], (PVOID)&send_features, 2);

	if (ptr->mode & MODE_BASE64 || features & FEATURES_NEWPG)
		replaceStr(ptr->tmp, mir_base64_encode(publ1, KEYSIZE + 2));
	else
		replaceStr(ptr->tmp, base16encode((LPSTR)&publ1, KEYSIZE + 2));

	return ptr->tmp;
}

// store KeyB
int __cdecl cpp_init_keyb(HANDLE context, LPCSTR key)
{
	pCNTX ptr = get_context_on_id(context); if (!ptr) return 0;
	pSIMDATA p = (pSIMDATA)cpp_alloc_pdata(ptr);

	unsigned clen = (unsigned)rtrim(key);
	ptr->features = 0;

	LPSTR pub_binary;
	if ((clen == KEYSIZE * 2) || (clen == (KEYSIZE + 2) * 2))
		pub_binary = base16decode(key, &clen);
	else
		pub_binary = (LPSTR)mir_base64_decode(key, &clen);

	if (!pub_binary || (clen != KEYSIZE && clen != KEYSIZE + 2)) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("cpp_init_keyb: error bad_keyb");
#endif
		ptr->error = ERROR_BAD_KEYB;
		mir_free(pub_binary);
		return 0;
	}

	if (clen == KEYSIZE + 2)
		memcpy((PVOID)&ptr->features, (PVOID)(pub_binary + KEYSIZE), 2);

	if (p->KeyP) {
		if (!(ptr->features & FEATURES_PSK)) { // if NO PSK on other side
#if defined(_DEBUG) || defined(NETLIB_LOG)
			Sent_NetLog("cpp_init_keyb: error no_psk");
#endif
			ptr->error = ERROR_NO_PSK;
			return 0;
		}
		ptr->error = ERROR_BAD_PSK;
		try {
			// decrypt PUBLIC use PSK
			string unciphered;
			CFB_Mode<AES>::Decryption dec(p->KeyP, Tiger::DIGESTSIZE, IV);
			StreamTransformationFilter cbcDecryptor(dec, new StringSink(unciphered));

			cbcDecryptor.Put((PBYTE)pub_binary, KEYSIZE);
			cbcDecryptor.MessageEnd();
			memcpy(pub_binary, unciphered.data(), unciphered.length());
		}
		catch (...) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
			Sent_NetLog("cpp_init_keyb: error bad_psk");
#endif
			return 0;
		}
	}

	mir_free(p->KeyB);
	p->KeyB = (PBYTE)pub_binary;

	if (p->PubA && memcmp(p->PubA, p->KeyB, KEYSIZE) == 0) {
#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("cpp_init_keyb: error bad_keyb keya==keyb");
#endif
		mir_free(p->KeyB);
		ptr->error = ERROR_BAD_KEYB;
		return 0;
	}

	ptr->error = ERROR_NONE;
	return 1;
}

// calculate secret key, return true or false
int __cdecl cpp_calc_keyx(HANDLE context)
{
	pCNTX ptr = get_context_on_id(context); if (!ptr) return 0;
	pSIMDATA p = (pSIMDATA)cpp_alloc_pdata(ptr);

	if (!p->KeyA) { ptr->error = ERROR_NO_KEYA; return 0; }
	if (!p->KeyB) { ptr->error = ERROR_NO_KEYB; return 0; }
	ptr->error = ERROR_NONE;

	BYTE agreeVal[KEYSIZE];
	memset(agreeVal, 0, sizeof(agreeVal));

	BYTE agr = p->dh->Agree(agreeVal, p->KeyA, p->KeyB, true); // calculate key
	if (agr) {
		// not needed anymore
		SAFE_FREE(p->PubA);
		SAFE_FREE(p->KeyA);
		mir_free(p->KeyB); p->KeyB = 0;

		BYTE buffer[Tiger::DIGESTSIZE]; // buffer for hash
		memset(buffer, 0, sizeof(buffer));

		// do this only if key exchanged is ok
		// we use a 192bit key (24*8)
		Tiger().CalculateDigest(buffer, agreeVal, KEYSIZE); // calculate hash

		// store key
		SAFE_FREE(p->KeyX);
		p->KeyX = (PBYTE)malloc(Tiger::DIGESTSIZE);
		memcpy(p->KeyX, buffer, Tiger::DIGESTSIZE);
	}
	return (int)agr;
}

// create pre-shared key from password
int __cdecl cpp_init_keyp(HANDLE context, LPCSTR password)
{
	pCNTX ptr = get_context_on_id(context); if (!ptr) return 0;
	pSIMDATA p = (pSIMDATA)cpp_alloc_pdata(ptr);

	BYTE buffer[Tiger::DIGESTSIZE]; // buffer for hash
	memset(buffer, 0, sizeof(buffer));

	// calculate hash
	Tiger().CalculateDigest(buffer, (PBYTE)password, strlen(password));

	// store pre-shared key
	SAFE_FREE(p->KeyP);
	p->KeyP = (PBYTE)malloc(Tiger::DIGESTSIZE);
	memcpy(p->KeyP, buffer, Tiger::DIGESTSIZE);

	return 1;
}
