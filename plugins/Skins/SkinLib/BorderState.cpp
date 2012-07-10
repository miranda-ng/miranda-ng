#include "globals.h"
#include <windows.h>
#include "BorderState.h"

BorderState::BorderState(int aLeft, int aRight, int aTop, int aBottom) 
		: left(aLeft), right(aRight), top(aTop), bottom(aBottom)
{
}

BorderState::~BorderState()
{
}

int BorderState::getLeft() const
{
	return left;
}

void BorderState::setLeft(int left)
{
	this->left = max(0, left);
}

int BorderState::getRight() const
{
	return right;
}

void BorderState::setRight(int right)
{
	this->right = max(0, right);
}

int BorderState::getTop() const
{
	return top;
}

void BorderState::setTop(int top)
{
	this->top = max(0, top);
}

int BorderState::getBottom() const
{
	return bottom;
}

void BorderState::setBottom(int bottom)
{
	this->bottom = max(0, bottom);
}

void BorderState::setAll(int border)
{
	border = max(0, border);
	left = border;
	right = border; 
	top = border;
	bottom = border;
}
