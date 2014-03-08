{
Weather Protocol plugin for Miranda IM
Copyright (C) 2005-2009 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2002-2005 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
}

{$IFNDEF M_WEATHER}
{$DEFINE M_WEATHER}

// {6B612A34-DCF2-4e32-85CF-B6FD006B745E}
const MIID_WEATHER:TGUID = '{6B612A34-DCF2-4E32-85CF-B6FD006B745E}';

//============  CONSTANTS  ============
const
// name
  WEATHERPROTONAME  = 'Weather';
  WEATHERPROTOTEXT  = 'Weather';
  DEFCURRENTWEATHER = 'WeatherCondition';
  WEATHERCONDITION  = 'Current';

// weather conditions
   SUNNY   = ID_STATUS_ONLINE;
   NA      = ID_STATUS_OFFLINE;
   PCLOUDY = ID_STATUS_AWAY;
   CLOUDY  = ID_STATUS_NA;
   RAIN	   = ID_STATUS_OCCUPIED;
   RSHOWER = ID_STATUS_DND;
   SNOW    = ID_STATUS_FREECHAT;
   LIGHT   = ID_STATUS_INVISIBLE;
   THUNDER = ID_STATUS_INVISIBLE;
   SSHOWER = ID_STATUS_ONTHEPHONE;
   FOG     = ID_STATUS_OUTTOLUNCH;
   UNAVAIL = ID_STATUS_IDLE;


//============  WEATHER CONDITION STRUCT  ============

// weather conditions  (added in v0.1.2.0)
type
  PWEATHERINFO = ^TWEATHERINFO;
  TWEATHERINFO = record
    hContact:TMCONTACT;
    id      :array [0..127] of WideChar;
    city    :array [0..127] of WideChar;
    update  :array [0.. 63] of WideChar;
    cond    :array [0..127] of WideChar;
    temp    :array [0.. 15] of WideChar;
    low     :array [0.. 15] of WideChar;
    high    :array [0.. 15] of WideChar;
    feel    :array [0.. 15] of WideChar;
    wind    :array [0.. 15] of WideChar;
    winddir :array [0.. 63] of WideChar;
    dewpoint:array [0.. 15] of WideChar;
    pressure:array [0.. 15] of WideChar;
    humid   :array [0.. 15] of WideChar;
    vis     :array [0.. 15] of WideChar;
    sunrise :array [0.. 31] of WideChar;
    sunset  :array [0.. 31] of WideChar;
// are the other ones that important!?
    status:word;
  end;


const
// ===============  WEATHER SERVICES  ================

// Enable or disable weather protocol.
// WPARAM = FALSE to toggle, TRUE to use the LPARAM
// LPARAM = TRUE to enable, FALSE to disable
  MS_WEATHER_ENABLED:PAnsiChar = 'Weather/EnableDisable';

// Update all weather info
// WPARAM = LPARAM = NULL
  MS_WEATHER_UPDATEALL:PAnsiChar = 'Weather/UpdateAll';

// Update all weather info + erase the old ones
// WPARAM = LPARAM = NULL
  MS_WEATHER_REFRESHALL:PAnsiChar = 'Weather/RefreshAll';

// Below are the service functions for weather contacts
// The plugin does NOT verify that they are used in weather contact,
// so bad call on these function may cause crashes.

// Update a single station
// WPARAM = (HANDLE)hContact
// LPARAM = NULL
  MS_WEATHER_UPDATE:PAnsiChar = 'Weather/Update';

// Update a single station + delete old settings
// WPARAM = (HANDLE)hContact
// LPARAM = NULL
  MS_WEATHER_REFRESH:PAnsiChar = 'Weather/Refresh';

// View the brief info of a contact
// WPARAM = (HANDLE)hContact
// LPARAM = NULL
  MS_WEATHER_BRIEF:PAnsiChar = 'Weather/Brief';

// Use default browser to open the complete forecast on web
// WPARAM = (HANDLE)hContact
// LPARAM = NULL
  MS_WEATHER_COMPLETE:PAnsiChar = 'Weather/CompleteForecast';

// Use default browser to open the weather map defined for the contact
// WPARAM = (HANDLE)hContact
// LPARAM = NULL
  MS_WEATHER_MAP:PAnsiChar = 'Weather/Map';

// Open the external log of the weather contact
// WPARAM = (HANDLE)hContact
// LPARAM = NULL
  MS_WEATHER_LOG:PAnsiChar = 'Weather/Log';

// Edit weather contact setting
// WPARAM = (HANDLE)hContact
// LPARAM = NULL
  MS_WEATHER_EDIT:PAnsiChar = 'Weather/Edit';

// parse the string to turn it to weather display
// WPARAM = (WEATHERINFO*)hContact
// LPARAM = (char*)display_str
  MS_WEATHER_GETDISPLAY:PAnsiChar = 'Weather/GetDisplay';

// ===============  WEATHER EVENTS  ================

{
HANDLE hContact = (HANDLE)wParam;
bool Condition_Changed = (bool)lParam;

hContact is the handle of updated contact
If the weather condition is differ from the last update (either temperature/condition,
or the last update time, depend what the user choose in the options), then
Condition_Changed is true; otherwise is false.
}
  ME_WEATHER_UPDATED:PAnsiChar = 'Miranda/Weather/Updated';

{
Shows a warning message for Weather PopUp.
wParam = (char*) lpzMessage
lParam = Type
Type can either be SM_WARNING, SM_NOTIFY, or SM_WEATHERALERT

This event is used to avoid the error popup to occurs within a thread, so the "Use
multiply thread" fuction don't have to be enabled for weather popups to work.
}
  SM_WEATHERALERT = 16;
  ME_WEATHER_ERROR:PAnsiChar = 'Miranda/Weather/Error';


{$ENDIF}
