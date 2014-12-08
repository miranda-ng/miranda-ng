{$IFNDEF M_SPLASH}
{$DEFINE M_SPLASH}

// Shows splash image
// wParam = (char *) image filename, either relative to Miranda dir or absolute
// lParam = (int) time to display in milliseconds, 0 - infinite (hangs on screen until clicked)
// returns: 1 if success and 0 if failed
const
  MS_SHOWSPLASH:PAnsiChar = 'SplashScreen/Show';

{$ENDIF}
