#ifndef _CLCDOBJECT_H_
#define _CLCDOBJECT_H_

#include "CLCDGfx.h"

class CLCDObject
{
public:
	// Constructor
	CLCDObject();
	// Destructor
	~CLCDObject();
	
	// Initialize the object
	virtual bool Initialize();
	// Shutdown the object
	virtual bool Shutdown();

	// Set the origin of the object
	void SetOrigin(int iX,int iY);
	void SetOrigin(POINT p);
	// Get the origin of the object
	POINT GetOrigin();
	// Set the size of the object
	void SetSize(int iWidth,int iHeight);
	void SetSize(SIZE s);
	// Get the size of the object
	SIZE GetSize();
	int GetWidth();
	int GetHeight();

	// Set the visibility 
	void Show(bool bShow);
	// Check the visibility
	bool IsVisible();

	// Update the object
	virtual bool Update();
	// Draw the object
	virtual bool Draw(CLCDGfx *pGfx);

protected:
	// Called when the size of the object has changed
	virtual void OnSizeChanged(SIZE OldSize);

private:
	POINT	m_Origin;
	SIZE	m_Size;
	bool	m_bShow;
};

#endif