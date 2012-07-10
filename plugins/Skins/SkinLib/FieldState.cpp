#include "globals.h"
#include "FieldState.h"
#include "DialogState.h"
#include "BorderState.h"

#define START		1<<0
#define LEN			1<<1
#define END			1<<2
#define USING_MASK	0xFF
#define LAST_SHIFT	8
#define SET(_FIELD_, _ITEM_)		_FIELD_ = (((_FIELD_ | _ITEM_) & USING_MASK) | (_ITEM_ << LAST_SHIFT))
#define LAST_SET(_FIELD_)			(_FIELD_ >> LAST_SHIFT)


FieldState::FieldState(DialogState *aDialog, Field *aField) 
		: field(aField), dialog(aDialog), size(-1, -1), pos(0, 0), 
		  usingX(0), usingY(0), visible(aField->isEnabled()), borders(0,0,0,0),
		  tooltipSet(false), halign(HORIZONTAL_ALIGN_LEFT), valign(VERTICAL_ALIGN_TOP)
{
}

FieldState::~FieldState()
{
}

Field * FieldState::getField() const
{
	return field;
}

DialogState * FieldState::getDialog() const
{
	return dialog;
}

int FieldState::getX() const
{
	return pos.x;
}

void FieldState::setX(int x)
{
	if (usingX & END)
	{
		int diff = x - getX();
		size.x = max(0, getWidth() - getHorizontalBorders() - diff);
	}

	pos.x = x;

	SET(usingX, START);
}

int FieldState::getY() const
{
	return pos.y;
}

void FieldState::setY(int y)
{
	if (usingY & END)
	{
		int diff = y - getY();
		size.y = max(0, getHeight() - getVerticalBorders() - diff);
	}

	pos.y = y;

	SET(usingY, START);
}

int FieldState::getWidth() const
{
	if (size.x >= 0)
		return size.x + getHorizontalBorders();

	return getPreferedSize().x + getHorizontalBorders();
}

void FieldState::setWidth(int width)
{
	width = max(0, width - getHorizontalBorders()) + getHorizontalBorders();

	if (LAST_SET(usingX) == END)
	{
		int diff = width - getWidth();
		pos.x = getX() - diff;
	}

	size.x = width - getHorizontalBorders();

	usingX |= LEN;
}

int FieldState::getHeight() const
{
	if (size.y >= 0)
		return size.y + getVerticalBorders();

	return getPreferedSize().y + getVerticalBorders();
}

void FieldState::setHeight(int height)
{
	height = max(0, height - getVerticalBorders()) + getVerticalBorders();

	if (LAST_SET(usingY) == END)
	{
		int diff = height - getHeight();
		pos.y = getY() - diff;
	}

	size.y = height - getVerticalBorders();

	usingY |= LEN;
}

bool FieldState::isVisible() const
{
	if (!visible)
		return false;

	RECT rc = getRect();
	if (rc.right <= rc.left || rc.bottom <= rc.top)
		return false;

	return true;
}

void FieldState::setVisible(bool visible)
{
	this->visible = visible;
}

bool FieldState::isEnabled() const
{
	return field->isEnabled();
}

int FieldState::getLeft() const
{
	return getX();
}

void FieldState::setLeft(int left)
{
	setX(left);
}

int FieldState::getTop() const
{
	return getY();
}

void FieldState::setTop(int top)
{
	setY(top);
}

int FieldState::getRight() const
{
	return getX() + getWidth();
}

void FieldState::setRight(int right)
{
	if (usingX & START)
	{
		size.x = max(0, right - getX());
	}
	else
	{
		pos.x = right - getWidth();
	}

	SET(usingX, END);
}

int FieldState::getBottom() const
{
	return getY() + getHeight();
}

void FieldState::setBottom(int botom)
{
	if (usingY & START)
	{
		size.y = max(0, botom - getY());
	}
	else
	{
		pos.y = botom - getHeight();
	}

	SET(usingY, END);
}

const TCHAR * FieldState::getToolTip() const
{
	if (tooltipSet)
		return tooltip.c_str();
	else
		return field->getToolTip();
}

void FieldState::setToolTip(const TCHAR *tooltip)
{
	this->tooltip = tooltip;
	tooltipSet = true;
}

BorderState * FieldState::getBorders()
{
	return &borders;
}

const BorderState * FieldState::getBorders() const
{
	return &borders;
}

int FieldState::getHorizontalBorders() const
{
	return borders.getLeft() + borders.getRight();
}

int FieldState::getVerticalBorders() const
{
	return borders.getTop() + borders.getBottom();
}

static inline int beetween(int val, int minVal, int maxVal)
{
	return max(minVal, min(maxVal, val));
}

static inline void intersection(RECT &main, const RECT &other)
{
	main.left = beetween(main.left, other.left, other.right);
	main.right = beetween(main.right, other.left, other.right);
	main.top = beetween(main.top, other.top, other.bottom);
	main.bottom = beetween(main.bottom, other.top, other.bottom);
}

RECT FieldState::getRect(bool raw) const
{
	RECT ret = {0};

	if (!visible)
		return ret;

	RECT inside = dialog->getInsideRect();

	ret.left = getLeft() + inside.left;
	ret.right = getRight() + inside.left;
	ret.top = getTop() + inside.top;
	ret.bottom = getBottom() + inside.top;

	if (!raw)
		intersection(ret, inside);

	return ret;
}

RECT FieldState::getInsideRect(bool raw) const
{
	RECT ret = {0};

	if (!visible)
		return ret;

	RECT inside = dialog->getInsideRect();

	ret.left = getLeft() + borders.getLeft() + inside.left;
	ret.right = getRight() - borders.getRight() + inside.left;
	ret.top = getTop() + borders.getTop() + inside.top;
	ret.bottom = getBottom() - borders.getBottom() + inside.top;

	if (!raw)
		intersection(ret, inside);

	return ret;
}

HORIZONTAL_ALIGN FieldState::getHAlign() const
{
	return halign;
}

void FieldState::setHAlign(HORIZONTAL_ALIGN halign)
{
	this->halign = halign;
}

VERTICAL_ALIGN FieldState::getVAlign() const
{
	return valign;
}

void FieldState::setVAlign(VERTICAL_ALIGN valign)
{
	this->valign = valign;
}
