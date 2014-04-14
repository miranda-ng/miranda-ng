/**
 * FIXME:
 *
 *
 */

#include "name_day_core.h"
#include "utils/string_tokenizer.h"

#include "static_database.h"


// Miranda stuff.
#include "resource.h"
#include "../../headers_c/newpluginapi.h"
#include "../../headers_c/m_database.h"
#include "../../headers_c/m_clist.h"
#include "../../headers_c/m_popup.h"
#include "../../headers_c/m_protocols.h"
#include "../../headers_c/m_skin.h"
#include "../../headers_c/m_protomod.h"
#include "../../headers_c/m_ignore.h"

#include <sstream>


extern		HINSTANCE hInst;

static		name_day_core_t	*name_day_object = NULL;

int command0(WPARAM wParam, LPARAM lParam)
{
	return name_day_object->perform_command(0);
}

int command1(WPARAM wParam, LPARAM lParam)
{
	return name_day_object->perform_command(1);
}

int command2(WPARAM wParam, LPARAM lParam)
{
	return name_day_object->perform_command(2);
}

/**
 * @brief constructor
 *
 */
name_day_core_t::name_day_core_t()
{

	name_day_object = this;

	// Create the calendars from the provided static arrays.

	// FIXME: this is not very optimal! It will create the const object and copy it
	// into the array, right?
	//
	
	// FIXME: insert only the calendars that are wanted by the user (some option
	// screen is needed.

	calendars.push_back(calendar_t(czech_names, 366, IDI_ICON1));
	calendars.push_back(calendar_t(slovak_names, 366, IDI_ICON2));
	calendars.push_back(calendar_t(french_names, 366, IDI_ICON4));
}

/**
 * @brief destructor
 *
 */
name_day_core_t::~name_day_core_t(void)
{
	calendars.clear();
}

/**
 * @brief perform name day test.
 *
 * Traverse the contact list and test the user first names against today name
 * days.
 */
void name_day_core_t::perform_name_day_test(void)
{
	// Get the actual time.
	// FIXME: this function returns the UTC!
	SYSTEMTIME sys_time;	
	GetSystemTime(&sys_time);



	// Get the first contact from the contact list.

	HANDLE contact_handle = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	
	// Traverse the contacts.
	// Get the first name for each contact.

	while (contact_handle != NULL) {		
		
		// Get the contact proto.
		
		char *proto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)contact_handle,0);

		if (! proto) {
			contact_handle = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)contact_handle, 0); 
			continue;
		}

		// Determine the contact name.

		string contact_name = (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)contact_handle, 0);


		// And the first name.

		DBVARIANT dbv;
		dbv.pszVal = NULL;
		DBGetContactSetting(contact_handle, proto, "FirstName", &dbv);		

		if (dbv.pszVal) {

			string first_name = dbv.pszVal;

			for (unsigned i = 0; i < calendars.size(); ++i) {
				
				const string name_day = calendars[i].get_name(sys_time.wMonth, sys_time.wDay);
				
				// This user has name day.
				// Create the miranda event.
				if (has_name_day(name_day, first_name)) {
					create_name_day_event(contact_handle, contact_name, first_name, calendars[i].country);
				}
			}			
		}		

		contact_handle = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)contact_handle, 0); 
	}
}

/**
 * @brief create name day event
 * @param handle
 * @param contact_name
 * @param first_name
 * @param country
 */
void name_day_core_t::create_name_day_event(HANDLE &handle, const string &contact_name, const string &first_name, const string &country)
{
	CLISTEVENT cle;
	
	ZeroMemory(&cle, sizeof(cle));

	cle.cbSize = sizeof(cle);
	cle.flags = CLEF_URGENT;
	cle.hContact = handle;
	//cle.hDbEvent = (HANDLE)(uniqueEventId++);
	cle.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON3));
	//cle.pszService = "";
	
	const string tooltip = contact_name + "(" + first_name + ") has a Name Day today [" + country + "]! Congratulation!";

	cle.pszTooltip = const_cast<char *>(tooltip.c_str());
	CallService(MS_CLIST_ADDEVENT, 0,( LPARAM)&cle);
}

/**
 * @brief Create miranda menu
 *
 */
void name_day_core_t::create_menu(void)
{
	// Create the main menu.
	CLISTMENUITEM mi;	
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);

	// The main manu caption.
	mi.pszPopupName = "Name Day";

	// The submenu caption.
	mi.pszName = "Happy Name Day to";	

	// The separator.
	mi.hotKey=MAKELPARAM(0,VK_F1);

	// Approx. position on the menu. lower numbers go nearer the top
	// Try to set it as top as possible.
	mi.popupPosition = -0x7FFFFFFF;
	mi.position = -0x7FFFFFFF;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON3));

	CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);	


	// FIXME: solve this some way
	CreateServiceFunction("NameDay/0", command0);
	CreateServiceFunction("NameDay/1", command1);
	CreateServiceFunction("NameDay/2", command2);

	// Add the submenus for each country database.

	for (size_t i = 0; i < calendars.size(); ++i) {

		std::stringstream sstr;
		sstr << "NameDay/" << i;

		create_sub_menu(calendars[i], sstr.str());
	}

	/*
	HANDLE hContact;

	hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
	CallService(MS_PROTO_ADDTOCONTACT, ( WPARAM )hContact, ( LPARAM )"PING" );
	CallService(MS_IGNORE_IGNORE, (WPARAM)hContact, (WPARAM)IGNOREEVENT_USERONLINE);
	
	DBWriteContactSettingString(hContact, "PING", "Nick", "yaaa");
	*/
}
/**
 * @create sub menu
 * @param calendar
 *
 */
void name_day_core_t::create_sub_menu(const calendar_t &calendar, const string &function_name)
{

	// Get the actual date and time.
	SYSTEMTIME		sys_time;	
	GetSystemTime(&sys_time);

	// Resolve the actual name for this database	
	string name_day = calendar.get_name(sys_time.wMonth, sys_time.wDay);

	CLISTMENUITEM	mi;	
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.pszPopupName = "Name Day";
	mi.pszName = const_cast<char *>(name_day.c_str());
	mi.position = 1;	
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(calendar.icon_id));
	
	mi.pszService = const_cast<char *>(function_name.c_str());

	CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);	
}


/**
 * @brief has name day
 * @param name_day
 * @param first_name
 *
 */
bool name_day_core_t::has_name_day(const string &name_day, const string &first_name)
{
	// Parse the name day string for today.
	// Extract the names.

	vector<string> today_names = string_tokenizer(name_day, " ");

	// Try to match the first name with the extracted names.
	for (size_t i = 0; i < today_names.size(); ++i) {
		
		// Huray we have found the contact who could celebrate.
		if (today_names[i] == first_name) {
			return true;
		}
	}

	return false;
}

int name_day_core_t::perform_command(const unsigned calendar_idx)
{
	const unsigned	future_names_count = 7;
	
	SYSTEMTIME	sys_time;
	GetSystemTime(&sys_time);

	const unsigned day_in_year = calendars[calendar_idx].get_day_in_year(sys_time.wMonth, sys_time.wDay);

	std::stringstream sstr;

	for (unsigned i = 0; i < future_names_count; ++i) {
		if (i + day_in_year >= calendars[calendar_idx].get_name_count()) {
			break;
		}
		sstr << "+ " << i << " " << calendars[calendar_idx].get_name(day_in_year + i) << "\n";
	}

	MessageBox(NULL, sstr.str().c_str(), "Name Day", MB_OK);

	return 0;	
}

