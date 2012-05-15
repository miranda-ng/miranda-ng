//
//  Register of plugin's user
//
//  wParam = (WPARAM)szSetting - string that describes a user
//           format: Category/ModuleName,
//           eg: "Contact list background/CLUI",
//               "Status bar background/StatusBar"
//  lParam = (LPARAM)dwFlags
//
#define MS_BACKGROUNDCONFIG_REGISTER "BkgrCfg/Register"

//
//  Notification about changed background
//  wParam = ModuleName
//  lParam = 0
#define ME_BACKGROUNDCONFIG_CHANGED "BkgrCfg/Changed"

#define CLBF_TILEVTOROWHEIGHT        0x0100
