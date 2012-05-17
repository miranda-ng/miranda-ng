/*
Copyright (C) 2009 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#ifndef __SCOPE_H__
# define __SCOPE_H__


#define DEFINE_SCOPED_TYPE(_NAME_, _DELETE_)			\
	template<class T>									\
	class _NAME_										\
	{													\
		_NAME_(_NAME_ &);								\
		_NAME_ & operator=(_NAME_ &);					\
														\
	public:												\
		_NAME_(T *t = NULL) : p(t) {}					\
														\
		~_NAME_() 										\
		{ 												\
			release(); 									\
		}												\
														\
		T * operator=(T *t) 							\
		{ 												\
			release(); 									\
			p = t; 										\
			return t;									\
		}												\
														\
		operator T*() const 							\
		{ 												\
			return p; 									\
		}												\
														\
		bool operator==(T *t) const 					\
		{ 												\
			return p == t;								\
		}												\
														\
		bool operator!=(T *t) const 					\
		{ 												\
			return p != t;								\
		}												\
														\
		T *get() const 									\
		{												\
			return p;									\
		}												\
														\
		void release()									\
		{												\
			if (p != NULL)								\
				dealoc(p);								\
			p = NULL;									\
		}												\
														\
		T* detach() 									\
		{ 												\
			T *ret = p;									\
			p = NULL;									\
			return ret;									\
		}												\
														\
	protected:											\
		T *p;											\
														\
		void dealoc(T *ptr)								\
		{												\
			_DELETE_;									\
		}												\
	};													\
														\
	template<typename T> 								\
	inline bool operator==(T* p, const _NAME_<T> &b) {	\
	  return p == b.get();								\
	}													\
														\
	template<typename T> 								\
	inline bool operator!=(T* p, const _NAME_<T> &b) {	\
	  return p != b.get();								\
	}

DEFINE_SCOPED_TYPE(scoped_ptr, delete ptr)
DEFINE_SCOPED_TYPE(scoped_array, delete[] ptr)
DEFINE_SCOPED_TYPE(scoped_free, free(ptr))
DEFINE_SCOPED_TYPE(scoped_mir_free, mir_free(ptr))


#endif // __SCOPE_H__
