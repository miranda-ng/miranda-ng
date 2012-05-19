#ifndef _SCAN_INC
#define _SCAN_INC

#include "xmldata.h"
#include "services.h"
#include "utils.h"
#include "allocations.h"
#include "options.h"

void ScanPlugins(FilenameMap *fn_map, UpdateList *update_list);
void ScanLangpacks(FilenameMap *fn_map, UpdateList *update_list);

// returns true if any dll is not 'disabled' in db
bool RearrangeDlls(char *shortName, StrList &filenames);

bool RearrangeLangpacks(char *shortName, StrList &filenames);

#endif
