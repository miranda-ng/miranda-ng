Note
----
'Actions' is action groups which can be executed through services
'Subactions' is single simple action like 'Call service', 'run program' etc.

Description
-----------  
This service plugin can be used for defining, management and executing some
actions.
Action can be executed later through miranda service.

Settings
--------
Main window consist of two parts. Left part is for action list and subaction list
for selected action. Navigation buttons for action list and subaction list positioned near
these lists.
'New'    - to add new list item
'Delete' - to delete selected item
'Reload' - to reload settings from DB
'Test'   - executes currently selected action
'Up','Down' - for place selected subaction upper or lower current position
With DblClick on action groups list item you can rename this item

When action list is not empty, in right side you can see combobox for subaction type selecting.
At this moment only next types supported:
  Open contact window
  Call service
  Execute program
  Insert text
  Advanced
  Link to action
  Profile
  MessageBox

-- Open contact window
Just select needed contact from combobox.

-- Call service
This is one of hard to understand action type.
wParam and lParam comboboxes in upper part are for parameter type.
  'number'          - just integer number (decimal or hex started from $ sign)
  'ANSI string'     - single-byte character string
  'Unicode string'  - double-byte character string
  'Current contact' - mean what parameter is current user handle.
                      Current user is active window owner
  'Last result'     - result from previous action or calling parameter
  'Parameter'       - parameter from calling service
  'Structure'       - parameter is structure which can be edited in options dialog
                      but will not saving changes in runtime.

wParam and lParam in lower part are for values, passed to service.
If service.ini file presents in plugins directory, you can press F1 to see short help
notes for selected from combobox service name.
Most upper part, 'result action' is for action, what we must to do with result:
show in popup, in message window or paste into text.
Option to translate result are: integer decimal value (signed or not), hex value or
string value (Unicode or ANSI)
'<proto>' in service name will be replaced by protocol name for contact handle in parameter

-- Execute program
More hard for understanding than the upper settings.
Process settings: 'parallel' means executing next subaction or return from executing action
immediately, 'continued' mean what next subaction will be only after finishing this program.
'Process time' option specifies a period of time, after which the process will be shut down.
'Current path' mean what program will run with current directory as start directory.
Window options are for starting window view: minimized, normal, maximized etc.
Several macros can be used in program name:
<param> - parameter
<last> - last result

-- Insert text
You can work with Clipboard or file/message window here.
In field below action type combobox you can write some text which will be inserted
into text field of miranda and other program.
Or, that text can be written to file or some text can be readed.
If you want help for text formatting, you can press F1.
You can use Variables plugin. If you want it, just mark checkbox. help button can
get help notes for available variables and functions.

-- Advanced
This block is the most difficult for understanding. Upper group is conditions for operation executing.
'Value' is value for comparing last subaction result with it.
Next actions will be if condition is true:
'Math'  - mathematic operations with last result and presented in 'value' field value.
After math calculations (or w/o it) can be used next operations:
'Operation' block consist of several operations:
  'BREAK' - break action executing (and return for 'parent' action executing - if exist)
  'JUMP'  - jump to subaction with selected name
  'NOP'   - not operation

As 'Math' alternative, you can use Variables plugin scripts.
Script can be written in text field below and checked by pressing 'Vars' button.
Variables %subject% and %extratext% can be used in the script.

-- Call chain
Select action from combobox below. This action will be executed. After that next subaction
will be executed.

-- Database
This subaction is for database reading/writing (see Operation block).
Value and value type is for type of data, reading or writing from/to database.
'Last result' mean that value is result of previous action.
Group of radioboxes is for contact type.
  'Own settings' - all settings only for our contact (our profile settings)
  'Parameter'    - settings for contact with handle, passed from start parameter
  'Last result'  - contact handle is result of last action
  'Manual'       - contact handle selected from combobox
'<proto>' in module name will be replaced with contact protocol name

-- MessageBox
  Subaction uses for standard windows MessageBox showing. text <last> will be replaced
by last result value.

Using
-----
Executing action from option dialog is possible but not recommended.
First way is assign action to button for toolbar (maybe through modern clist skin engine)
Second way is assign action to hotkey (miranda core not so good for this now)
Third way is to insert action into menu by other plugin.
Pluginmaker Vasilich wrote his plugin (UserActions) which works with ActMan and gives
access to ability to assign actions to some controls (menu, toolbar, hotkey).

Byte/Word/DWord    - numeric integer data
byte/word pointers - pointers to some data (really, like ANSI/Unicode strings)
byte/word array    - array of mixed data (really, like ANSI/Unicode strings)
last result/param  - current working data = dword
