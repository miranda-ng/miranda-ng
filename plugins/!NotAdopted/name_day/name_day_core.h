/**
 * FIXME:
 *
 */
#ifndef name_day_core_h
#define name_day_core_h

#pragma warning(disable:4996)

#define MIID_NAMEDAY { 0x5dc83ad2, 0x868c, 0x4c50, { 0xad, 0x33, 0x40, 0x51, 0x22, 0xe8, 0xcb, 0x38 } }


#include "calendar/calendar.h"

// WInd00z stuff
#include <windows.h>


/**
 * FIXME:
 *
 */
class name_day_core_t
{
	public:
							name_day_core_t				();
							~name_day_core_t			();
		
			void			perform_name_day_test		(void);
			void			create_menu					(void);

			int				perform_command				(const unsigned calendar_idx);

    private:				

			void			create_name_day_event		(HANDLE &handle, const string &contact_name, const string &first_name, const string &country);			
			bool			has_name_day				(const string &name_day, const string &first_name);
			void			create_sub_menu				(const calendar_t &calendar, const string &function_name);			

				vector<calendar_t>		calendars;

};


#endif // calendar_h