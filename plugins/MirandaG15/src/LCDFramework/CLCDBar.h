#ifndef _CLCDBar_H_
#define _CLCDBar_H_

#define TOP 1
#define CENTER 2
#define BOTTOM 3

#define MODE_PROGRESSBAR 0
#define MODE_SCROLLBAR 1

#define DIRECTION_VERTICAL 0
#define DIRECTION_HORIZONTAL 1

class CLCDBar : public CLCDObject
{
public:
	// constructor
	CLCDBar();
	// destructor
	~CLCDBar();
	
	// initializes the bar
	bool Initialize();
	// deintializes the bar
	bool Shutdown();

	// draws the bar
	bool Draw(CLCDGfx *pGfx);
	// updates the bar
	bool Update();

	// specifies the bar's mode ( scrollbar / progressbar )
	void SetMode(int iMode);
	// specifies the orientation of the bar
	void SetOrientation(int iDirection);

	// sets the alignment of the scrollbar position
	void SetAlignment(int iAlignment);
	// sets the size of the slider
	void SetSliderSize(int iSize);
	
	// scrolls up/left
	bool ScrollUp();
	inline bool ScrollLeft() { return ScrollUp(); };
	// scrolls down/right
	bool ScrollDown();
	inline bool ScrollRight() { return ScrollDown(); };

	// scrolls to the specified position
	bool ScrollTo(int iPosition);
	// specifies the bar's range
	void SetRange(int iMin,int iMax);

private:
	int		m_iOrientation;
	int		m_iMode;
	int		m_iSliderSize;
	int		m_iMax;
	int		m_iMin;
	int		m_iPosition;
	int		m_iAlignment;
};

#endif