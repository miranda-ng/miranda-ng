/**
 * FIXME:
 *
 *
 */

#include "calendar.h"



/**
 * @brief constructor
 * @param static names		Static array of calendar names.
 *
 * Insert the names into the dynamic array.
 */
calendar_t::calendar_t(const char *const *const static_names, const size_t names_count, const unsigned icon_id) //:
		//country
		//icon_id
		//names
{
	names.resize(names_count);

	// The first element of the static array is the country name.

	country = static_names[0];

	for (size_t i = 0; i < names_count - 1; ++i) {
		names[i] = static_names[i + 1];
	}

	this->icon_id = icon_id;
}


/**
 * @brief destructor
 *
 */
calendar_t::~calendar_t(void)
{
	names.clear();
}

/**
 * @brief get_name
 * @param day_in_year
 *
 */
const string &calendar_t::get_name(const unsigned day_in_year) const
{
	return names[day_in_year];
}

/**
 * @brief get the day in the year.
 * @return index
 * @param month
 * @param day
 *
 */
unsigned calendar_t::get_day_in_year(const unsigned month, const unsigned day) const
{
	// NOTE: it's too late for me to come with some clever
	// alg. for this... :)


	unsigned index = day + 29 * (month - 1) - 1;

	if (month == 1) {
		return index;
	}

	// January
	index += 2;
	
	// Jan & Feb end.
	if (month == 2 || month == 3) {
		return index;
	}	


	index += 2;

	if (month == 4) {
		return index;
	}

	index += 1;

	if (month == 5) {
		return index;
	}

	index += 2;

	if (month == 6) {
		return index;
	}

	index += 1;

	if (month == 7) {
		return index;
	}

	index += 2;

	if (month == 8) {
		return index;
	}

	index += 2;

	if (month == 9) {
		return index;
	}

	index += 1;

	if (month == 10) {
		return index;
	}

	index += 2;

	if (month == 11) {
		return index;
	}

	index += 1;

	return index;
}

/**
 * @brief get the name
 * @param day		Day in month
 * @param month		Month in year
 *
 */
const string &calendar_t::get_name(const unsigned day, const unsigned month) const
{
	const unsigned day_in_year = get_day_in_year(day, month);

	return get_name(day_in_year);
}

/**
 * @brief get name count
 * @return name_count
 *
 */
unsigned calendar_t::get_name_count(void) const
{
	return names.size();
}