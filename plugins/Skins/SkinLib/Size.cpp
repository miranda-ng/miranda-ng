#include "globals.h"
#include "Size.h"

Size::Size() : x(0), y(0)
{
}

Size::Size(int aX, int aY) : x(aX), y(aY)
{

}

int Size::resizeTo(int newX, int newY)
{
	if (newX < 0 && newY < 0)
		return -1;
	
	if (newY < 0) 
	{
		if (x < 0 || y < 0)
			return -2;

		y = (int) (y * (newX / (float) x));
		x = newX;
	}
	else if (newX < 0)
	{
		if (x < 0 || y < 0)
			return -2;

		x = (int) (x * (newY / (float) y));
		y = newY;
	}
	else
	{
		x = newX;
		y = newY;
	}
	return 0;
}

int Size::fitInside(int maxSize)
{
	if (x < 0 || y < 0)
		return -2;
	if (x <= maxSize && y <= maxSize)
		return 0;

	if (x >= y)
	{
		y = (int) (y * (maxSize / (float) x));
		x = maxSize;
	}
	else
	{
		x = (int) (x * (maxSize / (float) y));
		y = maxSize;
	}
	return 0;
}

int Size::scaleTo(int size)
{
	if (x < 0 || y < 0)
		return -2;

	if (x >= y)
	{
		y = (int) (y * (size / (float) x));
		x = size;
	}
	else
	{
		x = (int) (x * (size / (float) y));
		y = size;
	}

	return 0;
}

bool Size::operator==(const Size &other) const
{
	return x == other.x && y == other.y;
}