#ifndef _XSN_UTILS_H
#define _XSN_UTILS_H

void initModuleConvertTable(ModuleConvertTable &table);

bool isReceiveMessage(LPARAM event);

std::tstring getContactSound(HANDLE contact);
ModuleString getContactModule(HANDLE contact);

std::tstring getIcqContactId(HANDLE contact, const ModuleString &module);
std::tstring getJabberContactId(HANDLE contact, const ModuleString &module);


#endif
