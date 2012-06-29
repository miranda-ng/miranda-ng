#ifndef _XSN_UTILS_H
#define _XSN_UTILS_H

#include "xsn_types.h"
#include "SoundNotifyData.h"

void initModuleConvertTable(ModuleConvertTable & table);

bool isReceiveMessage(LPARAM event);

xsn_string getContactSound(HANDLE contact);
ModuleString getContactModule(HANDLE contact);

xsn_string getIcqContactId(HANDLE contact, const ModuleString & module);
xsn_string getJabberContactId(HANDLE contact, const ModuleString & module);


#endif
