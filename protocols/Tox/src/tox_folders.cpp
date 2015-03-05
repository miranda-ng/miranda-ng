#include "common.h"

HANDLE CToxProto::hProfileFolderPath = NULL;

void CToxProto::InitFolders()
{
	hProfileFolderPath = FoldersRegisterCustomPathT("Tox", Translate("Profiles folder"), MIRANDA_USERDATAT);
}
