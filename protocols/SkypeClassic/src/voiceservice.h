#ifndef _VOICESERVICE_H_
#define _VOICESERVICE_H_

#pragma warning (push)
#pragma warning (disable: 4201) // nonstandard extension used : nameless struct/union

#pragma warning (pop)

BOOL HasVoiceService();
void VoiceServiceInit();
void VoiceServiceExit();
void VoiceServiceModulesLoaded();
void NofifyVoiceService(HANDLE hContact, char *callId, int state) ;



#endif // _VOICESERVICE_H_

