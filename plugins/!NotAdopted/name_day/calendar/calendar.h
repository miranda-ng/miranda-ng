/**
 * FIXME:
 *
 */
#ifndef calendar_h
#define calendar_h

#include <vector>
#include <string>
using namespace std;

class calendar_t
{
	public:
							calendar_t				(const char *const *const static_names, const size_t names_count, const unsigned icon_id);
							~calendar_t				();
			
			const	string	&get_name				(const unsigned day_in_year) const;
			const	string	&get_name				(const unsigned day, const unsigned month) const;

					unsigned	get_day_in_year		(const unsigned month, const unsigned day) const;
					unsigned	get_name_count		(void) const;



				string				country;
				unsigned			icon_id;

    private:
						calendar_t			() {};

				vector<string>		names;

};


#endif // calendar_h