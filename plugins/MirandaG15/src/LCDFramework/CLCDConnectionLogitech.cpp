#include "stdafx.h"
#include "CLCDConnectionLogitech.h"
#include "CLCDOutputManager.h"

DWORD WINAPI softButtonCallback(IN int device,
                                             IN DWORD dwButtons,
											 IN const PVOID pContext) {
	((CLCDConnectionLogitech*)pContext)->OnSoftButtonCB(dwButtons);
	return 0;
}

DWORD WINAPI notificationCallback(IN int connection,
                                              IN const PVOID pContext,
                                              IN DWORD notificationCode,
                                              IN DWORD notifyParm1,
                                              IN DWORD notifyParm2,
                                              IN DWORD notifyParm3,
											  IN DWORD notifyParm4) {
	((CLCDConnectionLogitech*)pContext)->OnNotificationCB(notificationCode,notifyParm1,notifyParm2,notifyParm3,notifyParm4);
	return 0;
}


DWORD WINAPI initializeDrawingThread( LPVOID pParam ) {
	((CLCDConnectionLogitech*)pParam)->runDrawingThread();
	return 0;
}

void CLCDConnectionLogitech::runDrawingThread() {
	m_hStopEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hDrawEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

	DWORD dwRes = 0;

	while(1) {
		HANDLE hArray[2] = { m_hStopEvent, m_hDrawEvent };
		dwRes = WaitForMultipleObjects(2, hArray, FALSE, INFINITE);
		if(dwRes == WAIT_OBJECT_0) {
			break;
		} else if(dwRes == WAIT_OBJECT_0+1) {
			DWORD rc;
			if(GetConnectionState() != CONNECTED) {
				continue;
			}
			// do a sync update if the applet is in the foreground, or every 500 ms
			// the delay is there because sync updates can take up to 33ms to fail
			if(m_dwForegroundCheck < GetTickCount())
			{
				m_dwForegroundCheck = GetTickCount() + 500;
				rc = lgLcdUpdateBitmap(m_hDevice, &m_lcdBitmap.hdr, LGLCD_SYNC_COMPLETE_WITHIN_FRAME(m_iPriority));
				if(rc == ERROR_ACCESS_DENIED)
				{
					rc = ERROR_SUCCESS;
					m_bIsForeground = false;
				}
				else if(rc == ERROR_SUCCESS)
					m_bIsForeground = true;
			}
			else
				rc = lgLcdUpdateBitmap(m_hDevice, &m_lcdBitmap.hdr, LGLCD_ASYNC_UPDATE(m_iPriority));
			
			if(rc != ERROR_SUCCESS) {
				HandleErrorFromAPI(rc);
			}
		}
	}
	CloseHandle(m_hStopEvent);
	CloseHandle(m_hDrawEvent);
}

// the connection instance
CLCDConnectionLogitech *CLCDConnectionLogitech::m_pInstance = NULL;

//************************************************************************
// returns the connection type
//************************************************************************
int CLCDConnectionLogitech::GetConnectionType()
{
	return TYPE_LOGITECH;
}

//************************************************************************
// Constructor
//************************************************************************
CLCDConnectionLogitech::CLCDConnectionLogitech()
{
	m_iNumQVGADevices = 0;
	m_iNumBWDevices = 0;

	m_pDrawingBuffer = NULL;
	m_pConnectedDevice = NULL;
	m_hKeyboardHook = NULL;
	m_bVolumeWheelHook = false;
	
	m_dwButtonState = 0;
	m_bConnected = false;
	m_bSetAsForeground = false;
	m_dwForegroundCheck = 0;

	m_hHIDDeviceHandle = NULL;
	m_hConnection = LGLCD_INVALID_CONNECTION;
	m_hDevice = LGLCD_INVALID_DEVICE;
	
	m_bIsForeground = false;
	
	m_hDrawEvent = NULL;
	m_hStopEvent = NULL;

	CLCDConnectionLogitech::m_pInstance = this;

	m_hDrawingThread = CreateThread( 0, 0, initializeDrawingThread, (void*)this, 0, 0);
}

//************************************************************************
// Destructor
//************************************************************************
CLCDConnectionLogitech::~CLCDConnectionLogitech()
{
	do {
		SetEvent(m_hStopEvent);
	} while(WaitForSingleObject(m_hDrawingThread,500) == WAIT_TIMEOUT);

	if(m_pDrawingBuffer != NULL) {
		free(m_pDrawingBuffer);
	}
	SetVolumeWheelHook(false);
}

//************************************************************************
// Initializes the connection to the LCD
//************************************************************************
bool CLCDConnectionLogitech::Initialize(tstring strAppletName,bool bAutostart, bool bConfigDialog)
{
	
	m_strAppletName = strAppletName;
	// initialize the library
    if(lgLcdInit() != ERROR_SUCCESS)
		return false;

	ZeroMemory(&m_connectContext, sizeof(m_connectContext));
 	m_connectContext.connection = LGLCD_INVALID_CONNECTION;
   
    m_connectContext.appFriendlyName = m_strAppletName.c_str();
    m_connectContext.isAutostartable = bAutostart;
    m_connectContext.isPersistent = bAutostart;
	m_connectContext.dwAppletCapabilitiesSupported = LGLCD_APPLET_CAP_BW | LGLCD_APPLET_CAP_QVGA;
	m_connectContext.onNotify.notificationCallback = notificationCallback;
	m_connectContext.onNotify.notifyContext = (PVOID)this;

	if(bConfigDialog) {
		m_connectContext.onConfigure.configCallback = CLCDOutputManager::configDialogCallback;
	} else {
		m_connectContext.onConfigure.configCallback = NULL;
	}
	m_connectContext.onConfigure.configContext = NULL;
    
	lgLcdSetDeviceFamiliesToUse(m_connectContext.connection,LGLCD_DEVICE_FAMILY_ALL,NULL);

	return true;
}

//************************************************************************
// returns the name of the attached device
//************************************************************************
tstring CLCDConnectionLogitech::GetDeviceName() {
	if(m_pConnectedDevice->GetIndex() == LGLCD_DEVICE_BW) {
		return _T("G15/Z10");
	} else {
		return _T("G19");
	}
}

//************************************************************************
// enumerates all attached devices
//************************************************************************
CLgLCDDevice* CLCDConnectionLogitech::GetAttachedDevice(int iIndex) {
	std::vector<CLgLCDDevice*>::iterator i = m_lcdDevices.begin();
	for(;i!=m_lcdDevices.end();i++) {
		if((*i)->GetIndex() == iIndex) {
			return *i;
		}
	}

	return NULL;
}

//************************************************************************
// disconnects the device
//************************************************************************
bool CLCDConnectionLogitech::Disconnect() {
	if(!m_bConnected)
		return false;

	if(m_pConnectedDevice != NULL) {
		delete m_pConnectedDevice;
		m_pConnectedDevice = NULL;
	}

	m_bReconnect = false;
	
	HIDDeInit();
	lgLcdClose(m_hDevice);
	m_hDevice = LGLCD_INVALID_DEVICE;
	
	CLCDOutputManager::GetInstance()->OnDeviceDisconnected();

	m_bConnected = false;
	return true;
}

//************************************************************************
// returns a pointer to the current device
//************************************************************************
CLgLCDDevice* CLCDConnectionLogitech::GetConnectedDevice() {
	return m_pConnectedDevice;
}

//************************************************************************
// connects to the device
//************************************************************************
bool CLCDConnectionLogitech::Connect(int iIndex)
{
	DWORD rc;
	lgLcdOpenByTypeContext        OpenContext;
	if(m_bConnected && (iIndex == 0 || iIndex == GetConnectedDevice()->GetIndex()))
		return true;

	if(m_hConnection == LGLCD_INVALID_CONNECTION)
	{
		rc =  lgLcdConnectEx(&m_connectContext);
		// establish the connection
		if(ERROR_SUCCESS == rc)
		{
			m_hConnection = m_connectContext.connection;
			m_hDevice = LGLCD_INVALID_CONNECTION;

			TRACE(_T("CLCDConnectionLogitech: Connection to LCDManager established successfully!\n"));
		}
		else {
			return false;
		}
	}

	// check if the specified device exists
	m_pConnectedDevice = GetAttachedDevice(iIndex);
	if(m_pConnectedDevice == NULL) {
		iIndex = (!iIndex || iIndex == LGLCD_DEVICE_BW) ? LGLCD_DEVICE_BW : LGLCD_DEVICE_QVGA;
		m_pConnectedDevice = GetAttachedDevice(iIndex);
		if(m_pConnectedDevice == NULL) {
			return false;
		}
	}

	// close the lcd device before we open up another
    if (LGLCD_INVALID_DEVICE != m_hDevice) {
		Disconnect();
	}

	// Now lets open the LCD. We must initialize the g_OpenContext structure.
    ZeroMemory(&OpenContext, sizeof(OpenContext));
    OpenContext.connection = m_hConnection;
	OpenContext.deviceType = m_pConnectedDevice->GetIndex();//LGLCD_DEVICE_QVGA;
	OpenContext.device = LGLCD_INVALID_DEVICE;

    // softbutton callbacks are not needed
    OpenContext.onSoftbuttonsChanged.softbuttonsChangedCallback = softButtonCallback;
    OpenContext.onSoftbuttonsChanged.softbuttonsChangedContext = (PVOID)this;

	// open the lcd
    rc = lgLcdOpenByType(&OpenContext);
	// failed to open the lcd
	if(rc != ERROR_SUCCESS)
		return false;
	
	m_hDevice = OpenContext.device;
	
	// Create the pixel buffer
	m_lcdBitmap.hdr.Format = OpenContext.deviceType==LGLCD_DEVICE_QVGA?LGLCD_BMP_FORMAT_QVGAx32:LGLCD_BMP_FORMAT_160x43x1;
	if(m_pDrawingBuffer != NULL) {
		free(m_pDrawingBuffer);
	}
	
	m_pPixels = OpenContext.deviceType==LGLCD_DEVICE_QVGA? m_lcdBitmap.bmp_qvga32.pixels:m_lcdBitmap.bmp_mono.pixels;
	m_iPixels =	OpenContext.deviceType==LGLCD_DEVICE_QVGA? sizeof(m_lcdBitmap.bmp_qvga32.pixels):sizeof(m_lcdBitmap.bmp_mono.pixels);
	m_pDrawingBuffer = (PBYTE) malloc(m_iPixels);
	ZeroMemory(m_pDrawingBuffer, m_iPixels);
	
	m_iPriority = LGLCD_PRIORITY_NORMAL;
	m_bConnected = true;

	HIDInit();
	
	m_bReconnect = true;
	
	CLCDOutputManager::GetInstance()->OnDeviceConnected();
	return true;
}

//************************************************************************
// Closes the connection with the LCD
//************************************************************************
bool CLCDConnectionLogitech::Shutdown()
{
	m_bConnected = false;

	SetVolumeWheelHook(false);
	
	Disconnect();

    if (LGLCD_INVALID_CONNECTION != m_hDevice)
		lgLcdDisconnect(m_hConnection);

    lgLcdDeInit();
	
	return true;
}

//************************************************************************
// Reads data from the keyboard HID device
//************************************************************************
bool CLCDConnectionLogitech::HIDReadData(BYTE* data) {
	static OVERLAPPED olRead;
	static HANDLE hReadEvent = CreateEvent(NULL,false,true,_T("ReadEvent"));
	static BYTE privateBuffer[9];
	
	DWORD TransBytes;
	if(!m_bConnected) {
		SetEvent(hReadEvent);
		return false;
	}

	DWORD dwRes = WaitForSingleObject(hReadEvent,0);
	if(dwRes == WAIT_OBJECT_0) {
		bool bRes = false;
		if(GetOverlappedResult(m_hHIDDeviceHandle,&olRead,&TransBytes,false)) {
			memcpy(data,privateBuffer,9*sizeof(BYTE));	
			bRes = true;
		} 

		memset(&olRead,0,sizeof(OVERLAPPED));
		olRead.hEvent = hReadEvent;

		if(!ReadFile(m_hHIDDeviceHandle,privateBuffer,9,&TransBytes,&olRead)) {
			DWORD error = GetLastError();
			if(error != ERROR_IO_PENDING) {
				return false;
			}
		}
		return bRes;
	} 
	
	return false;
}

void CLCDConnectionLogitech::OnSoftButtonCB(DWORD state) {
	m_dwButtonState = state;
}

void CLCDConnectionLogitech::OnNotificationCB( DWORD notificationCode, DWORD notifyParm1, DWORD notifyParm2, DWORD notifyParm3, DWORD notifyParm4) {
	CLgLCDDevice *device;
	
	switch(notificationCode) {
		case LGLCD_NOTIFICATION_DEVICE_ARRIVAL: {
			int *counter = notifyParm1 == LGLCD_DEVICE_QVGA ? &m_iNumQVGADevices : &m_iNumBWDevices;
			if(*counter == 0) {
				SIZE size;
				if(notifyParm1 == LGLCD_DEVICE_QVGA) {
					size.cx = 320;
					size.cy = 240;
					device = new CLgLCDDevice(notifyParm1,size,7,4);
				} else {
					size.cx = 160;
					size.cy = 43;
					device = new CLgLCDDevice(notifyParm1,size,4,1);
				}
				m_lcdDevices.push_back(device);
			}

			(*counter)++;
			break;
		}
		case LGLCD_NOTIFICATION_DEVICE_REMOVAL: {
			int *counter = notifyParm1 == LGLCD_DEVICE_QVGA ? &m_iNumQVGADevices : &m_iNumBWDevices;
			(*counter)--;
			if(*counter == 0) {
				std::vector<CLgLCDDevice*>::iterator i = m_lcdDevices.begin();
				for(;i!=m_lcdDevices.end();i++) {
					if((*i)->GetIndex() == notifyParm1) {
						device = *i;
						
						if(device == m_pConnectedDevice) {
							HandleErrorFromAPI(ERROR_DEVICE_NOT_CONNECTED);
						}
						
						m_lcdDevices.erase(i);
						delete device;
						
						break;
					}
				}
			}
			break;
		}
	}
}

//************************************************************************
// Update function
//************************************************************************
bool CLCDConnectionLogitech::Update()
{
	// check for lcd devices
    if (LGLCD_INVALID_DEVICE == m_hDevice )
    {
		if(m_bReconnect) {
			Connect();
		}
	}

	BYTE buffer[9];
	if(HIDReadData(buffer)) {
		int button = 0;
		// mr key
		if(buffer[7] & 0x40) {
			button = 20;
		// lightbulb key
		} else if(buffer[1] & 0x80) {
			button = 21;
		}
		// m1,m2,m3
		for(int i=0,w=1;i<3;i++,w*=2) {
			if(buffer[6+i] & w) {
				button = 30+i;
			}
		}
		// g1 to g18
		if(buffer[8] & 0x40) {
			button = 18;
		} else {
			for(int j=0;j<3;j++) {
				int p = 1,w = 1;
				if(j == 1) {
					p = 2;
				} else if(j == 2) {
					w = 4;
				}

				for(int i=0;i<6;i++,w*=2) {			
					if(buffer[p+i] & w) {
						button = 1+j*6+i;
					}
				}
			}
		}
		if(button != 0) {
			TRACE(_T("GKey pressed: %d \n"),button);
		}
	}

	return true;
}

//************************************************************************
// returns the id of the specified button
//************************************************************************
int CLCDConnectionLogitech::GetButtonId(int iButton) {
	if(m_pConnectedDevice->GetIndex() == LGLCD_DEVICE_BW) {
		switch(iButton)
		{
		case 0: return LGLCDBUTTON_BUTTON0; break;
		case 1: return LGLCDBUTTON_BUTTON1; break;
		case 2: return LGLCDBUTTON_BUTTON2; break;
		case 3: return LGLCDBUTTON_BUTTON3; break;
		case 4: return LGLCDBUTTON_BUTTON4; break;
		case 5: return LGLCDBUTTON_BUTTON5; break;
		case 6: return LGLCDBUTTON_BUTTON6; break;
		case 7: return LGLCDBUTTON_BUTTON7; break;
		}
	} else {
		switch(iButton)
		{
		case 0: return LGLCDBUTTON_LEFT;	break;
		case 1: return LGLCDBUTTON_RIGHT;	break;
		case 2: return LGLCDBUTTON_OK;		break;
		case 3: return LGLCDBUTTON_CANCEL;	break;
		case 4: return LGLCDBUTTON_UP;		break;
		case 5: return LGLCDBUTTON_DOWN;	break;
		case 6: return LGLCDBUTTON_MENU;	break;
		}
	}

	return 0;
}

//************************************************************************
// Returns the state of the specified Button
//************************************************************************
bool CLCDConnectionLogitech::GetButtonState(int iButton)
{
	if(!GetConnectionState()==CONNECTED)
		return false;

	DWORD dwButton = GetButtonId(iButton);

	if(m_dwButtonState & dwButton)
		return true;
	return false;
}

//************************************************************************
// Hides the applet
//************************************************************************
bool CLCDConnectionLogitech::HideApplet()
{
	if(!GetConnectionState()==CONNECTED)
		return false;

	DWORD rc;

	rc = lgLcdUpdateBitmap(m_hDevice, &m_lcdBitmap.hdr, LGLCD_ASYNC_UPDATE(LGLCD_PRIORITY_IDLE_NO_SHOW));
	if(rc != ERROR_SUCCESS)
		return false;

	return true;
}

//************************************************************************
// Draws the specified bitmap on the LCD
//************************************************************************
bool CLCDConnectionLogitech::Draw()
{
	if(!GetConnectionState()==CONNECTED || !m_hDrawEvent)
		return false;

	memcpy(m_pPixels,m_pDrawingBuffer,m_iPixels);
	SetEvent(m_hDrawEvent);
	return true;
}

//************************************************************************
// Temporarily brings the applet to foreground
//************************************************************************
void CLCDConnectionLogitech::SetAlert(bool bAlert)
{
	m_iPriority = bAlert?LGLCD_PRIORITY_ALERT:LGLCD_PRIORITY_NORMAL;
}

//************************************************************************
// Activates the applet on the LCD
//************************************************************************
void CLCDConnectionLogitech::SetAsForeground(bool bSetAsForeground)
{
	// TODO: Activate when 1.02 is out
    DWORD dwSet = bSetAsForeground ? LGLCD_LCD_FOREGROUND_APP_YES : LGLCD_LCD_FOREGROUND_APP_NO;
    m_bSetAsForeground = bSetAsForeground;
    if (LGLCD_INVALID_DEVICE != m_hDevice)
    {
        lgLcdSetAsLCDForegroundApp(m_hDevice, bSetAsForeground);
    }
}

//************************************************************************
// returns wether the applet is currently activated
//************************************************************************
bool CLCDConnectionLogitech::IsForeground()
{
	return m_bIsForeground;
}

//************************************************************************
// Returns the display size
//************************************************************************
SIZE CLCDConnectionLogitech::GetDisplaySize()
{
	SIZE size = {0,0};

	if(!GetConnectionState()==CONNECTED)
		return size;

	return m_pConnectedDevice->GetDisplaySize();
}

//************************************************************************
// Returns the number of buttons for the display
//************************************************************************
int CLCDConnectionLogitech::GetButtonCount()
{
	if(!GetConnectionState()==CONNECTED)
		return 0;

	return m_pConnectedDevice->GetButtonCount();
}

//************************************************************************
// Returns the number of available colors
//************************************************************************
int CLCDConnectionLogitech::GetColorCount()
{
	if(!GetConnectionState()==CONNECTED)
		return 0;

	return m_pConnectedDevice->GetColorCount();
}

//************************************************************************
// Get the pointer to the pixel buffer
//************************************************************************
PBYTE CLCDConnectionLogitech::GetPixelBuffer()
{
	if(!GetConnectionState()==CONNECTED)
		return NULL;

	return (PBYTE)m_pDrawingBuffer;
}

//************************************************************************
// CLCDConnectionLogitech::HandleErrorFromAPI
//************************************************************************
void CLCDConnectionLogitech::HandleErrorFromAPI(DWORD dwRes)
{
    switch(dwRes)
    {
        // all is well
    case ERROR_SUCCESS:
        break;
        // we lost our device
    case ERROR_DEVICE_NOT_CONNECTED:
		TRACE(_T("CLCDConnectionLogitech::HandleErrorFromAPI(): Device was unplugged, closing device\n"));
		Disconnect();
		SetReconnect(true);
		SetVolumeWheelHook(false);
		
		break;
    default:
		TRACE(_T("CLCDConnectionLogitech::HandleErrorFromAPI(): FATAL ERROR, closing device and connection\n"));
		Disconnect();
		SetReconnect(true);
        
		lgLcdDisconnect(m_hConnection);
        m_hConnection = LGLCD_INVALID_CONNECTION;

		SetVolumeWheelHook(false);
        break;
    }
}

//************************************************************************
// returns the connection state
//************************************************************************
int CLCDConnectionLogitech::GetConnectionState()
{
	return m_bConnected ? CONNECTED : DISCONNECTED;
}

bool CLCDConnectionLogitech::HIDInit()
{
	if(GetConnectionState() != CONNECTED || 
		m_pConnectedDevice->GetIndex() != LGLCD_DEVICE_BW) //LGLCD_DEVICE_FAMILY_KEYBOARD_G15)
		return false;

// Logitech G15 
	int VendorID = 0x046d;
	int ProductID = 0xc222;

	//Use a series of API calls to find a HID with a specified Vendor IF and Product ID.

	HIDD_ATTRIBUTES						Attributes;
	SP_DEVICE_INTERFACE_DATA			devInfoData;
	bool								LastDevice = FALSE;
	int									MemberIndex = 0;
	LONG								Result;	
	
	DWORD Length = 0;
	PSP_DEVICE_INTERFACE_DETAIL_DATA detailData = NULL;
	HANDLE hDevInfo =NULL;
	GUID HidGuid;
	ULONG Required = 0;

	bool MyDeviceDetected = false;

	/*
	API function: HidD_GetHidGuid
	Get the GUID for all system HIDs.
	Returns: the GUID in HidGuid.
	*/

	HidD_GetHidGuid(&HidGuid);	
	
	/*
	API function: SetupDiGetClassDevs
	Returns: a handle to a device information set for all installed devices.
	Requires: the GUID returned by GetHidGuid.
	*/
	
	hDevInfo=SetupDiGetClassDevs 
		(&HidGuid, 
		NULL, 
		NULL, 
		DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);
		
	devInfoData.cbSize = sizeof(devInfoData);

	//Step through the available devices looking for the one we want. 
	//Quit on detecting the desired device or checking all available devices without success.

	MemberIndex = 0;
	LastDevice = FALSE;

	do
	{
		/*
		API function: SetupDiEnumDeviceInterfaces
		On return, MyDeviceInterfaceData contains the handle to a
		SP_DEVICE_INTERFACE_DATA structure for a detected device.
		Requires:
		The DeviceInfoSet returned in SetupDiGetClassDevs.
		The HidGuid returned in GetHidGuid.
		An index to specify a device.
		*/

		Result=SetupDiEnumDeviceInterfaces 
			(hDevInfo, 
			0, 
			&HidGuid, 
			MemberIndex, 
			&devInfoData);

		if (Result != 0)
		{
			//A device has been detected, so get more information about it.

			/*
			API function: SetupDiGetDeviceInterfaceDetail
			Returns: an SP_DEVICE_INTERFACE_DETAIL_DATA structure
			containing information about a device.
			To retrieve the information, call this function twice.
			The first time returns the size of the structure in Length.
			The second time returns a pointer to the data in DeviceInfoSet.
			Requires:
			A DeviceInfoSet returned by SetupDiGetClassDevs
			The SP_DEVICE_INTERFACE_DATA structure returned by SetupDiEnumDeviceInterfaces.
			
			The final parameter is an optional pointer to an SP_DEV_INFO_DATA structure.
			This application doesn't retrieve or use the structure.			
			If retrieving the structure, set 
			MyDeviceInfoData.cbSize = length of MyDeviceInfoData.
			and pass the structure's address.
			*/
			
			//Get the Length value.
			//The call will return with a "buffer too small" error which can be ignored.

			Result = SetupDiGetDeviceInterfaceDetail 
				(hDevInfo, 
				&devInfoData, 
				NULL, 
				0, 
				&Length, 
				NULL);

			//Allocate memory for the hDevInfo structure, using the returned Length.

			detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(Length);
			
			//Set cbSize in the detailData structure.

			detailData -> cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			//Call the function again, this time passing it the returned buffer size.

			Result = SetupDiGetDeviceInterfaceDetail 
				(hDevInfo, 
				&devInfoData, 
				detailData, 
				Length, 
				&Required, 
				NULL);

			// Open a handle to the device.
			// To enable retrieving information about a system mouse or keyboard,
			// don't request Read or Write access for this handle.

			/*
			API function: CreateFile
			Returns: a handle that enables reading and writing to the device.
			Requires:
			The DevicePath in the detailData structure
			returned by SetupDiGetDeviceInterfaceDetail.
			*/

			m_hHIDDeviceHandle=CreateFile 
				(detailData->DevicePath, 
				FILE_GENERIC_READ | FILE_GENERIC_WRITE, 
				FILE_SHARE_READ|FILE_SHARE_WRITE, 
				(LPSECURITY_ATTRIBUTES)NULL,
				OPEN_EXISTING, 
				FILE_FLAG_OVERLAPPED, 
				NULL);

			/*
			API function: HidD_GetAttributes
			Requests information from the device.
			Requires: the handle returned by CreateFile.
			Returns: a HIDD_ATTRIBUTES structure containing
			the Vendor ID, Product ID, and Product Version Number.
			Use this information to decide if the detected device is
			the one we're looking for.
			*/

			//Set the Size to the number of bytes in the structure.

			Attributes.Size = sizeof(Attributes);

			Result = HidD_GetAttributes 
				(m_hHIDDeviceHandle, 
				&Attributes);
			
			//Is it the desired device?
			MyDeviceDetected = FALSE;
			
			if (Attributes.VendorID == VendorID)
			{
				if (Attributes.ProductID == ProductID)
				{
					//Both the Vendor ID and Product ID match.
					MyDeviceDetected = TRUE;
				} 
				else
					CloseHandle(m_hHIDDeviceHandle);

			} 
			else
				CloseHandle(m_hHIDDeviceHandle);

		//Free the memory used by the detailData structure (no longer needed).
		free(detailData);
		}

		else
			LastDevice=TRUE;

		MemberIndex = MemberIndex + 1;
	} //do
	while ((LastDevice == FALSE) && (MyDeviceDetected == FALSE));

	if(MyDeviceDetected)
	{
		PHIDP_PREPARSED_DATA	PreparsedData;

		HidD_GetPreparsedData 
		(m_hHIDDeviceHandle, 
		&PreparsedData);
		
		HidP_GetCaps 
		(PreparsedData, 
		&m_HIDCapabilities);
		
		HidD_FreePreparsedData(PreparsedData);
	}
	//Free the memory reserved for hDevInfo by SetupDiClassDevs.

	SetupDiDestroyDeviceInfoList(hDevInfo);
	
	return MyDeviceDetected;
}

bool CLCDConnectionLogitech::HIDDeInit()
{
	if(!m_hHIDDeviceHandle)
		return false;
	
	CloseHandle(m_hHIDDeviceHandle);
	m_hHIDDeviceHandle = NULL;
	return true;
}

SG15LightStatus CLCDConnectionLogitech::GetLightStatus()
{
	SG15LightStatus status;
	status.bMKey[0] = false;
	status.bMKey[1] = false;
	status.bMKey[2] = false;
	status.bMRKey = false;
	status.eKBDBrightness = KBD_OFF;
	status.eLCDBrightness = LCD_OFF;
	
	if(GetConnectionState() != CONNECTED || 
		m_pConnectedDevice->GetIndex() != LGLCD_DEVICE_BW) //m_lcdDeviceDesc.deviceFamilyId != LGLCD_DEVICE_FAMILY_KEYBOARD_G15)
		return status;

	byte *data = new byte[m_HIDCapabilities.FeatureReportByteLength];

	data[0] = 0x02;
	data[1] = 0x02;
	data[2] = 0x00;
	data[3] = 0x00;

	HidD_GetFeature(m_hHIDDeviceHandle,data,m_HIDCapabilities.FeatureReportByteLength);

	
	// data[1] = Keys
	status.eKBDBrightness = (EKBDBrightness)data[1];
	
	// data[2] = LCD
	switch(data[2])
	{
	case 0x02:
		status.eLCDBrightness = LCD_ON;
		break;
	case 0x01:
		status.eLCDBrightness = LCD_MEDIUM;
		break;
	default:
		status.eLCDBrightness = LCD_OFF;
		break;
	}
	// MKeys
	status.bMKey[0] = !(data[3] & G15_M1_LIGHT);
	status.bMKey[1] = !(data[3] & G15_M2_LIGHT);
	status.bMKey[2] = !(data[3] & G15_M3_LIGHT);
	
	// MRKey
	status.bMRKey = !(data[3] & G15_MR_LIGHT);

	free(data);

	return status;
}

void CLCDConnectionLogitech::SetMKeyLight(bool bM1,bool bM2,bool bM3,bool bMR)
{
	if(GetConnectionState() != CONNECTED || 
		m_pConnectedDevice->GetIndex() != LGLCD_DEVICE_BW) //m_lcdDeviceDesc.deviceFamilyId != LGLCD_DEVICE_FAMILY_KEYBOARD_G15)
		return;

	byte *data = new byte[m_HIDCapabilities.FeatureReportByteLength];
	data[0] = 0x02;
	data[1] = 0x04;
	data[2] = 0x00;
	
	if(!bM1)
		data[2] |= G15_M1_LIGHT;
	if(!bM2)
		data[2] |= G15_M2_LIGHT;
	if(!bM3)
		data[2] |= G15_M3_LIGHT;
	if(!bMR)
		data[2] |= G15_MR_LIGHT;

	data[3] = 0x00; 

	HidD_SetFeature(m_hHIDDeviceHandle, data, m_HIDCapabilities.FeatureReportByteLength); 
	free(data);
}

void CLCDConnectionLogitech::SetLCDBacklight(ELCDBrightness eBrightness)
{
	if(GetConnectionState() != CONNECTED || 
		m_pConnectedDevice->GetIndex() != LGLCD_DEVICE_BW) //m_lcdDeviceDesc.deviceFamilyId != LGLCD_DEVICE_FAMILY_KEYBOARD_G15)
		return;

	byte *data = new byte[m_HIDCapabilities.FeatureReportByteLength];

	data[0] = 0x02;
	data[1] = 0x02;
	data[2] = eBrightness;
    data[3] = 0x00; 

	HidD_SetFeature(m_hHIDDeviceHandle, data, m_HIDCapabilities.FeatureReportByteLength); 

	free(data);
}

void CLCDConnectionLogitech::SetKBDBacklight(EKBDBrightness eBrightness)
{
	if(GetConnectionState() != CONNECTED || 
		m_pConnectedDevice->GetIndex() != LGLCD_DEVICE_BW) //m_lcdDeviceDesc.deviceFamilyId != LGLCD_DEVICE_FAMILY_KEYBOARD_G15)
		return;

	byte *data = new byte[m_HIDCapabilities.FeatureReportByteLength];

	data[0] = 0x02;
	data[1] = 0x01;
	data[2] = eBrightness;
    data[3] = 0x00; 

	HidD_SetFeature(m_hHIDDeviceHandle, data, m_HIDCapabilities.FeatureReportByteLength); 

	free(data);
}

void CLCDConnectionLogitech::SetVolumeWheelHook(bool bEnable)
{
	if(bEnable == m_bVolumeWheelHook)
		return;
	m_bVolumeWheelHook = bEnable;

	if(bEnable)
		m_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, CLCDConnectionLogitech::KeyboardHook, GetModuleHandle(NULL), 0);
	else if(m_hKeyboardHook)
		UnhookWindowsHookEx(m_hKeyboardHook);
}

LRESULT CALLBACK CLCDConnectionLogitech::KeyboardHook(int Code, WPARAM wParam, LPARAM lParam)
{
	if(Code == HC_ACTION && wParam == WM_KEYDOWN)
	{	
		KBDLLHOOKSTRUCT *key = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
		if(key->vkCode == VK_VOLUME_UP || key->vkCode == VK_VOLUME_DOWN)
		{
			if(m_pInstance->IsForeground())
			{	
				if(key->vkCode == VK_VOLUME_UP)
					CLCDOutputManager::GetInstance()->OnLCDButtonDown(LGLCDBUTTON_UP);
				else if(key->vkCode == VK_VOLUME_DOWN)
					CLCDOutputManager::GetInstance()->OnLCDButtonDown(LGLCDBUTTON_DOWN);	
				return 1;
			}
		}
	}
	return CallNextHookEx(m_pInstance->m_hKeyboardHook, Code, wParam, lParam);
}