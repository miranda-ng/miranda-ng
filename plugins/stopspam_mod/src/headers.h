#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include <malloc.h>
#include <time.h>
#include <string>
#include <sstream>
#include<fstream>


#include <newpluginapi.h>
#include <m_database.h>
#include <m_protosvc.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_clistint.h>
#include <m_skin.h>
#include <m_button.h>
#include <m_contacts.h>

#include <m_variables.h>
#include <m_folders.h>


#include "globals.h"
#include "stopspam.h"
#include "options.h"
#include "eventhooker.h"
#include "version.h"
#include "resource.h"
#include "utilities.h"

//boost
#include <boost/thread.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/nondet_random.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_int.hpp>

//utf8cpp
#include <utf8.h>


//#include <m_dos.h>
