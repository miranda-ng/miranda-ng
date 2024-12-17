#ifndef _STEAM_UTILS_H_
#define _STEAM_UTILS_H_

int SteamToMirandaStatus(PersonaState state);
PersonaState MirandaToSteamState(int status);

void ShowNotification(const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);
void ShowNotification(const wchar_t *caption, const wchar_t *message, int flags = 0, MCONTACT hContact = NULL);

MBinBuffer RsaEncrypt(const char *pszModulus, const char *exponent, const char *data);
MBinBuffer createMachineID(const char *accName);

#define now() time(0)

int64_t getRandomInt();
CMStringA protobuf_c_text_to_string(const ProtobufCMessage &msg);

#endif //_STEAM_UTILS_H_
