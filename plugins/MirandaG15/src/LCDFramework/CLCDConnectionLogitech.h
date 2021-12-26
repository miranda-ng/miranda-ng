#ifndef _CLCDCONNECTIONLOGITECH_H_
#define _CLCDCONNECTIONLOGITECH_H_

#include "CLCDDevice.h"
#include "CLCDConnection.h"
#include "g15sdk/lglcd.h"

#define G15_M1_LIGHT 0x01
#define G15_M2_LIGHT 0x02
#define G15_M3_LIGHT 0x04
#define G15_MR_LIGHT 0x08

enum ELCDBrightness { LCD_ON = 0x20, LCD_MEDIUM = 0x10, LCD_OFF = 0x00 };
enum EKBDBrightness { KBD_ON = 0x02, KBD_MEDIUM = 0x01, KBD_OFF = 0x00 };


struct SG15LightStatus
{
	bool bMKey[3];
	bool bMRKey;
	ELCDBrightness eLCDBrightness;
	EKBDBrightness eKBDBrightness;
};

class CLCDConnectionLogitech : public CLCDConnection
{
public:
	// returns the connection type
	int GetConnectionType();

	// Constructor / Destructor
	CLCDConnectionLogitech();
	~CLCDConnectionLogitech();

	// returns the name of the attached device
	tstring GetDeviceName();

	// returns a pointer to the device with that index
	CLgLCDDevice* GetAttachedDevice(int iIndex);
	// returns a pointer to the connected device
	CLgLCDDevice* GetConnectedDevice();

	// returns the connection state
	int GetConnectionState();

	// Initializes the connection to the LCD
	bool Initialize(tstring strAppletName, bool bAutostart = false, bool bConfigDialog = false);
	// connects to the device
	bool Connect(int iIndex = 0);
	// disconnects to the device
	bool Disconnect();

	// Closes the connection with the LCD
	bool Shutdown();
	// Update function
	bool Update();
	// Hides the applet
	bool HideApplet();
	// Draws the specified bitmap on the LCD
	bool Draw();

	// returns the id of the specified button
	int GetButtonId(int iButton);
	// Returns the state of the specified button
	bool GetButtonState(int iButton);
	// Temporarily brings the applet to foreground
	void SetAlert(bool bAlert);
	// Activates the applet on the LCD
	void SetAsForeground(bool bSetAsForeground);
	// returns wether the applet is currently activated
	bool IsForeground();

	// Returns the display size
	SIZE GetDisplaySize();
	// Returns the number of buttons for the display
	int GetButtonCount();
	// Returns the number of available colors
	int GetColorCount();

	// Returns a pointer to the pixel buffer
	uint8_t *GetPixelBuffer();

	//----------------------------------------------
	// Special functions to control the lights
	SG15LightStatus GetLightStatus();

	void SetMKeyLight(bool bM1, bool bM2, bool bM3, bool bMR);
	void SetLCDBacklight(ELCDBrightness eBrightness);
	void SetKBDBacklight(EKBDBrightness eBrightness);
	//----------------------------------------------
	//----------------------------------------------
	// Special functions for callbacks
	void OnSoftButtonCB(uint32_t state);
	void OnNotificationCB(uint32_t notificationCode, uint32_t notifyParm1, uint32_t notifyParm2, uint32_t notifyParm3, uint32_t notifyParm4);
	//----------------------------------------------
	// Special functions for the volume wheel hook
		// Activates/Deactivates the volumewheel hook
	void SetVolumeWheelHook(bool bEnable);
	// the keyboard hook callback
	static LRESULT CALLBACK KeyboardHook(int Code, WPARAM wParam, LPARAM lParam);
	//----------------------------------------------
	void runDrawingThread();
private:
	uint8_t *m_pDrawingBuffer;

	CLgLCDDevice *m_pConnectedDevice;
	// the connection instance
	static CLCDConnectionLogitech *m_pInstance;

	// HID variables
	HANDLE m_hHIDDeviceHandle;
	HIDP_CAPS m_HIDCapabilities;

	// HID functions
	bool HIDInit();
	bool HIDDeInit();
	bool HIDReadData(uint8_t* data);

	// Keyboard hook
	HHOOK m_hKeyboardHook;
	bool m_bVolumeWheelHook;

	// handles API Errors
	void HandleErrorFromAPI(uint32_t dwRes);

	int						m_iNumQVGADevices;
	int						m_iNumBWDevices;

	bool					m_bIsForeground;
	lgLcdConnectContextExW	m_connectContext;
	tstring					m_strAppletName;
	bool					m_bConnected;
	lgLcdBitmap				m_lcdBitmap;
	uint8_t					*m_pPixels;
	int						m_iPixels;
	int						m_hDevice;
	int						m_hConnection;
	uint32_t					m_dwButtonState;
	int						m_iPriority;

	uint32_t					m_dwForegroundCheck;
	bool					m_bSetAsForeground;
	std::vector<CLgLCDDevice*> m_lcdDevices;

	HANDLE					m_hDrawingThread;
	HANDLE					m_hStopEvent, m_hDrawEvent;
};

#endif
