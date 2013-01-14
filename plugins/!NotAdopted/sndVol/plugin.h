#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "../../include/newpluginapi.h"
#include "../../include/m_system.h"
#include "../../include/m_database.h"
#include "../../include/m_protomod.h"
#include "../../include/m_protosvc.h"
#include "../../include/m_langpack.h"
#include "../../include/m_clist.h"
#include "../../include/m_options.h"
#include "../../include/m_clui.h"
#include "../../include/m_clc.h"
#include "../../include/m_utils.h"
#include "../../include/m_skin.h"
#include "../../include/m_button.h"
#include <math.h>


#define SERVICENAME	"sndVol"

extern HINSTANCE hInst;
extern bool isEnabled;
extern HWND hwndFrame, hwndSlider, hwndOptSlider;
extern HICON hSoundOn, hSoundOff;
extern int autoPreview;

#define SLIDER_MIN  1
#define SLIDER_MAX  100
#define SLIDER_DIV	6

#define MS_RADIO_SETVOL "mRadio/SetVol"

#ifdef __cplusplus
extern "C" {
#endif 

int __declspec(dllexport) Load( PLUGINLINK *link );
int __declspec(dllexport) Unload( void );
__declspec(dllexport) PLUGININFO *MirandaPluginInfo( DWORD dwVersion );

#ifdef __cplusplus
}
#endif 
