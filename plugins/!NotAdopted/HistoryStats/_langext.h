#if !defined(HISTORYSTATS_GUARD__LANGEXT_H)
#define HISTORYSTATS_GUARD__LANGEXT_H

/*
 * language "enhancements" - people will hate me for this
 */

#define bool_(c_bool) \
	((c_bool) ? true : false)

#define BOOL_(cpp_bool) \
	((cpp_bool) ? TRUE : FALSE)

#define array_len(array_var) \
	(sizeof(array_var) / sizeof((array_var)[0]))

#define range_each_(index_var, lo_bound, up_bound) \
	for (int index_var = (lo_bound); index_var < (up_bound); ++index_var)

#define upto_each_(index_var, up_bound) \
	range_each_(index_var, 0, (up_bound))

#define array_each_(index_var, array_var) \
	range_each_(index_var, 0, array_len(array_var))

#define vector_each_(index_var, vector_var) \
	for (int index_var = 0; index_var != (vector_var).size(); ++index_var)

#define iter_each_(cont_type, iter_var, cont_var) \
	for (cont_type::iterator iter_var = (cont_var).begin(); iter_var != (cont_var).end(); ++iter_var)

#define citer_each_(cont_type, iter_var, cont_var) \
	for (cont_type::const_iterator iter_var = (cont_var).begin(); iter_var != (cont_var).end(); ++iter_var)

#endif // HISTORYSTATS_GUARD__LANGEXT_H
