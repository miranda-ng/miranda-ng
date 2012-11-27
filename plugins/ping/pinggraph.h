#ifndef _PINGGRAPH_H
#define _PINGGRAPH_H

#include "pinglist.h"
#include "pingthread.h"

#define MIN_GRAPH_HEIGHT		10		// minimum braph height, ms
#define MIN_BARS				20		// space for at least this many bars
#define MARK_PERIOD				3600	// vertical lines every this many secs (3600 == 1 hour)
#define MARK_TIME				100		// horizontal lines every this many ms

INT_PTR ShowGraph(WPARAM wParam, LPARAM lParam);

// save window positions, close windows
void graphs_cleanup();

// restore windows that were open when cleanup was called last?
void graphs_init();

extern HistoryMap history_map;

#endif
