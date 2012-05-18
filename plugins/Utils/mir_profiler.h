/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci

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


#ifndef __PROFILER_H__
# define __PROFILER_H__

#include <windows.h>
#include <string>
#include <map>



class MProfiler
{
public:

	static void Reset();
	static void Start(const char *name);
	static void Step(const char *name);
	static void End();
	static void Dump(const char *module);



	static struct Block
	{
		std::string name;
		Block *parent;
		std::map<std::string, Block *> children;
		bool started;
		LARGE_INTEGER start;
		LARGE_INTEGER last_step;
		LARGE_INTEGER total;

		Block();
		~Block();

		void Reset();
		void Start();
		void Step(const char *name);
		void Stop();
		double GetTimeMS() const;

		Block * GetChild(const char *name);
	};

	
private:

	static Block root;
	static Block *current;

	static void Dump(const char *module, std::string prefix, Block *block, double parent, double total);

};




#endif // __PROFILER_H__
