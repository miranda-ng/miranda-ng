#include "stdafx.h"
#include "CLCDGfx.h"
#include "CLCDObject.h"

//************************************************************************
// Constructor
//************************************************************************
CLCDObject::CLCDObject()
{
	m_Size.cx = 0;
	m_Size.cy = 0;
	m_Origin.x = 0;
	m_Origin.y = 0;
	m_bShow = true;
}

//************************************************************************
// Destructor
//************************************************************************
CLCDObject::~CLCDObject()
{
}

//************************************************************************
// Initialize the object
//************************************************************************
bool CLCDObject::Initialize()
{
	return false;
}

//************************************************************************
// Shutdown the object
//************************************************************************
bool CLCDObject::Shutdown()
{
	return false;
}

//************************************************************************
// Set the origin of the object
//************************************************************************
void CLCDObject::SetOrigin(int iX,int iY)
{
	m_Origin.x = iX;
	m_Origin.y = iY;
}

void CLCDObject::SetOrigin(POINT p)
{
	m_Origin = p;
}

//************************************************************************
// Get the origin of the object
//************************************************************************
POINT CLCDObject::GetOrigin()
{
	return m_Origin;
}

//************************************************************************
// Set the size of the object
//************************************************************************
void CLCDObject::SetSize(int iWidth,int iHeight)
{
	SIZE OldSize = m_Size;
	m_Size.cx = iWidth;
	m_Size.cy = iHeight;
	OnSizeChanged(OldSize);
}

void CLCDObject::SetSize(SIZE s)
{
	SIZE OldSize = m_Size;
	m_Size = s;
	OnSizeChanged(OldSize);
}

//************************************************************************
// Get the size of the object
//************************************************************************
SIZE CLCDObject::GetSize()
{
	return m_Size;
}

int CLCDObject::GetWidth()
{
	return m_Size.cx;
}

int CLCDObject::GetHeight()
{
	return m_Size.cy;
}

//************************************************************************
// Set the visibility 
//************************************************************************
void CLCDObject::Show(bool bShow)
{
	m_bShow = bShow;
}

//************************************************************************
// Check the visibility
//************************************************************************
bool CLCDObject::IsVisible()
{
	return m_bShow;
}

//************************************************************************
// Update the object
//************************************************************************
bool CLCDObject::Update()
{
	return true;
}


//************************************************************************
// Draw the object
//************************************************************************
bool CLCDObject::Draw(CLCDGfx *pGfx)
{
	return true;
}

//************************************************************************
// Called when the size of the object changed
//************************************************************************
void CLCDObject::OnSizeChanged(SIZE OldSize)
{

}