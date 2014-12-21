/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK
© 2010 Merlin_de

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "headers.h"

class MyTestEffect;

class MyTestEffect : public IPopupPlusEffect
{
protected:
	int w, h;
	int alpha0, alpha1;
	int frameCount;
	int frame;

	int stage;
	int alpha;

public:
	virtual void beginEffect(int w, int h, int alpha0, int alpha1, int frameCount)
	{
		this->w = w;
		this->h = h;
		this->alpha0 = alpha0;
		this->alpha1 = alpha1;
		this->frameCount = frameCount;
	}
	virtual void beginFrame(int frame)
	{
		this->frame = frame;
		stage = (frame * 2 > frameCount) ? 1 : 0;
		if (stage == 0)
		{
			alpha = alpha0 + (alpha1 - alpha0) * frame * 2 / frameCount;
		}
		else
		{
			alpha = alpha0 + (alpha1 - alpha0) * (frame * 2 - frameCount) / frameCount;
		}
	}
	virtual int getPixelAlpha(int x, int y)
	{
		if (stage == 0)
		{
			if ((x / 16 + y / 16) % 2) return alpha0;
			return alpha;
		}
		else
		{
			if ((x / 16 + y / 16) % 2) return alpha;
			return alpha1;
		}
	}
	virtual void endFrame() {}
	virtual void endEffect() {}
	virtual void destroy() { delete this; }
};

static INT_PTR svcCreateEffect_MyTestEffect(WPARAM, LPARAM) { return (INT_PTR)(new MyTestEffect); }

void PopupEfectsInitialize()
{
	CreateServiceFunction(MS_POPUP_CREATEVFX LPGEN("Square fading"), svcCreateEffect_MyTestEffect);

	CallService(MS_POPUP_REGISTERVFX, 0, (LPARAM)"Square fading");
}
