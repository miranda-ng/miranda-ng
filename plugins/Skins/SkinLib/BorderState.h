#pragma once

class BorderState
{
public:
	BorderState(int left, int right, int top, int bottom);
	~BorderState();

	int getLeft() const;
	void setLeft(int left);

	int getRight() const;
	void setRight(int right);

	int getTop() const;
	void setTop(int top);

	int getBottom() const;
	void setBottom(int bottom);

	void setAll(int border);

private:
	int left;
	int right;
	int top;
	int bottom;
};
