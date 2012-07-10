#ifndef __SIZE_H__
# define __SIZE_H__


struct Size 
{
	int x;
	int y;

	Size();
	Size(int aX, int aY);

	/// @return 0 on success
	int resizeTo(int x, int y);

	/// @return 0 on success
	int fitInside(int maxSize);

	/// @return 0 on success
	int scaleTo(int size);

	bool operator==(const Size &other) const;
};



#endif // __SIZE_H__
