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


// Disable "...truncated to '255' characters in the debug information" warnings
#pragma warning(disable: 4786)


#include "mir_profiler.h"
#include "mir_log.h"

#include <stdio.h>


MProfiler::Block MProfiler::root;
MProfiler::Block * MProfiler::current = &MProfiler::root;


MProfiler::Block::Block()
{
	parent = NULL;
	memset(&total, 0, sizeof(total));
	started = false;
}


MProfiler::Block::~Block()
{
	Reset();
}


void MProfiler::Block::Reset()
{
	for(std::map<std::string, MProfiler::Block *>::iterator it = children.begin(); 
			it != children.end(); ++it)
		delete it->second;
	children.clear();

	memset(&total, 0, sizeof(total));
	started = false;
}


void MProfiler::Block::Start()
{
	if (started)
		return;

	QueryPerformanceCounter(&start);
	last_step = start;
	started = true;
}


void MProfiler::Block::Step(const char *name)
{
	if (!started)
		return;
	
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	GetChild(name)->total.QuadPart += end.QuadPart - last_step.QuadPart;

	last_step = end;
}


void MProfiler::Block::Stop()
{
	if (!started)
		return;
	
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	total.QuadPart += end.QuadPart - start.QuadPart;

	started = false;
}


double MProfiler::Block::GetTimeMS() const
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	return total.QuadPart * 1000. / frequency.QuadPart;
}


MProfiler::Block * MProfiler::Block::GetChild(const char *name)
{
	MProfiler::Block *ret = children[name];
	if (ret == NULL)
	{
		ret = new MProfiler::Block();
		ret->name = name;
		ret->parent = this;
		children[name] = ret;
	}
	return ret;
}


void MProfiler::Reset()
{
	root.Reset();
}

void MProfiler::Start(const char *name)
{
	current = current->GetChild(name);
	current->Start();
}

void MProfiler::Step(const char *name)
{
	current->Step(name);
}

void MProfiler::End()
{
	current->Stop();

	if (current->parent != NULL)
	{
		current = current->parent;
		QueryPerformanceCounter(&current->last_step);
	}
}

void MProfiler::Dump(const char *module)
{
	Dump(module, "", &root, -1, -1);
}


void MProfiler::Dump(const char *module, std::string prefix, Block *block, double parent, double total)
{
	for(std::map<std::string, MProfiler::Block *>::iterator it = block->children.begin(); 
			it != block->children.end(); ++it)
	{
		Block *child = it->second;
		double elapsed = child->GetTimeMS();

		if (total > 0)
		{
			mlog(module, "Profiler", "%s%-20s\t%5.1lf\t[%3.0lf%%]\t[%3.0lf%%]", prefix.c_str(), it->first.c_str(), 
				elapsed, elapsed / parent * 100, elapsed / total * 100);
		}
		else
		{
			mlog(module, "Profiler", "%s%-20s\t%5.1lf", prefix.c_str(), it->first.c_str(), 
				elapsed);
		}

		Dump(module, prefix + "   ", child, elapsed, total > 0 ? total : elapsed);
	}
}

