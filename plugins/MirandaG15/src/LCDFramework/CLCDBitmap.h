#ifndef _CLCDBITMAP_H_
#define _CLCDBITMAP_H_

#include "CLCDGfx.h"
#include "CLCDObject.h"

class CLCDBitmap : public CLCDObject
{
	HBITMAP m_hBitmap = nullptr;

public:
	// constructor
	CLCDBitmap();
	// destructor
	~CLCDBitmap();

	// Initializes the bitmap
	bool Initialize();
	// Deinitializes the bitmap
	bool Shutdown();

	// updates the bitmap
	bool Update();
	// draws the bitmap
	bool Draw(CLCDGfx *pGfx);

	// Sets the bitmap
	void SetBitmap(HBITMAP hBitmap);
};

#endif