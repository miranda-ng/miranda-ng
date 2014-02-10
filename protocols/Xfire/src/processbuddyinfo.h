//funktion soll avatar runterladen und dem buddy zuordnen
#include "baseProtocol.h"
#include "resource.h"
#include "getbuddyinfo.h"
#include "buddyinfo.h"

void ProcessBuddyInfo(xfirelib::BuddyInfoPacket *buddyinfo, HCONTACT hcontact,char* username);