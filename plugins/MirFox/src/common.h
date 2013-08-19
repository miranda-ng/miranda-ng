#include "MirFoxCommons_pch.h"


#pragma comment(lib, "comctl32.lib")
#include <commctrl.h>

//Miranda headers
#include <win2k.h>
#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_clistint.h>
#include <m_protosvc.h>
#include <m_popup.h>
#include <m_message.h>
//for future use  #include <m_clist.h>


#define PLUGIN_DB_ID				"MirFox"
#define PLUGIN_OPTIONS_NAME			"MirFox"




//for MirandaUtils.h and MirfoxData.h and MirfoxMiranda.h
#include "MirFoxCommons_logger.h"

//for MirfoxMiranda
#include "MirFoxCommons_sharedMemory.h"
#include "MirFoxCommons_messageQueueBySM.h"

//for MirandaUtils.h
#include <iostream>								//for	ostringstream
#include <sstream>								//for	ostringstream
#include <locale>								//for	use_facet
#include <map>									//for	map

#include <winsock.h>		//needed for m_netlib.h
#include <m_netlib.h>

//for MirfoxData.h
#include <boost/algorithm/string.hpp>			//for	boost::iequals
#include <boost/algorithm/string/replace.hpp>	//for	boost::replace_all

