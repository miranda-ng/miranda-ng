Document updated: 28/9/06

******
Tipper - shows a tooltip when you hover the mouse over a contact in your contact list
******

Most options are self explanitory...except for 'items' and 'substitutions'.

If you want to set this up yourself, you need a moderate understanding of the miranda database (profile) and the Database Editor++ plugin.

The easiest way is to copy the autoexec_tipper.ini file (in the same folder as this document) to the miranda program folder and restart - 
it will (normally) ask you if you wish to import the settings. If you click yes, you will find several examples in your Tipper options that 
will get you started. You can also ask your nerdier miranda-using buddies to create such a file for you, if they have a good setup.

To get an idea of how tipper works, try playing with items. Items are simply a label and some text (the value). Try adding some items. Once
you've played around a bit you'll get the idea, and then you'll understand the need for substitutions.

Substitutions allow you to put useful, contact related, information into the label or value of an item. To create a substitution you need
to have a relatively good understanding of the miranda database - a few hours browsing with dbeditor++ will give you a clue. You create a 
substitution by specifying a name and the database module and setting names for the data it will contain. You can then put this data into
any item (label or value) by enclosing the substitution name in '%' symbols. Many database values are not terribly friendly to humans, so
when creating a substitution, you can also choose a translation which will convert the value into a more readable format.

To get technical:

A 'Substitution' is a name for a database value, represented according to a particular translation. When creating new substitutions, you 
specify its name, the database module (or the contact's protocol module) and the setting name. Then you select a translation from the drop 
down list. This transformation will convert the database value into a string.

An 'Item' is just a label and a value. However, any substitution names (surrounded by % symbols) occuring in either a label or a value will 
be substituted as described above. If you want to put a % symbol in a value or label, use '%%' instead.

A good example is representing a contacts status (as 'Online' etc) in the tooltip. 

First, create a substitution called 'status' (without quotes) - the module will be the contact's protocol module, the setting name will be 
'Status' (without quotes - note the capital 'S') and the translation will be 'WORD to status description' (since this value is a WORD value 
in the database). Then, create an item and specify 'Status:' for the label and '%status%' for the value. Done.

There is also a built in substitution, called a 'raw' substitution. It is not listed in the substitution list, but it is available in all 
labels and values. It's format is:

%raw:<db module>/<db setting>%

No translation is performed on 'raw' values. For example, to display a contact's contact list group in the tooltip, add an item with the 
label 'Group:' and the value '%raw:CList/Group%'. If you do not specify a module name (you must still include the '/'), the contact's 
protocol module will be used. This is ultimately just a shortcut for the 'null translation'.

There are also 'system' substitutions (thanks to y_b), also not listed but available in all item labels and values, with the following 
format:

%sys:<name>%

Were name can be any of the following:

uid                     - contact's unique identifier
uidname                 - name of unique identifier
proto                   - contact's protocol
time                    - contact's time
status_msg              - contact's status message
last_msg                - last message received from contact
last_msg_time           - time of last received message
last_msg_date           - date of last received message
last_msg_reltime        - relative time of last message (i.e. time between then and now)
meta_subname		- nickname of active subcontact (for metacontacts)
meta_subuid             - unique id of active subcontact
meta_subproto		- active subcontact protocol (also for metacontacts)

If a substitution's value results in no text (or nothing but spaces, tabs, newlines, and carriage returns), the whole item containing that 
substitution will not be displayed. If there is an error during the substitution process (e.g. a substitution name that does not exist, an odd 
number of % symbols, etc) then the value of that substitution will be "*". Note that you can use double-percent ("%%") in plain text (not in 
substitutions) if you want an actual percent symbol.

ADVANCED
--------

Alternative Text:
In any substitution you can have 'alternate text' to use if the substitution fails (missing db value, etc). The format is:

%x|y%

where x is the original substitution and y is the alternative text. Note that you can use '|' in the alternative text, since it uses the 
*first* occurence to determine where the alternative text begins. Normally if any substitution results in no value, the entire item will not 
be displayed - but if you omit the 'y' value above (i.e. have nothing for the 'alternate' text) then the substitution process will continue. 
As an example, consider the following item value:

%raw:/FirstName|% %raw:/LastName%

The above value will display the contact's first name if it's available, and then their last name - but if the last name is not available, the
entire item will not be displayed.

Specific protocol:
If you end a substitution with '^' and a protocol name, then that substitution will only be displayed if the contact belongs to the given 
protocol:

%x^y%

where y is the protocol name. If you want to display an item for every protocol *except* one, use

%x^!y%

If you use alternative text and specific protocol together, specify the alternative text first:

%x|y^z%

In such substitutions you can use a '^' symbol in the alternative text, as Tipper will take the *last* '^' symbol as the start of the protocol 
specifier. If you want to use a '^' symbol in alternative text without a specific protocol, just append a '^' to the end of the descriptor, 
e.g.:

%x|y^%


'HIDDEN' DB SETTINGS
--------------------

Due to space constraints in the options page and my own laziness, the following settings are only accessible via the database (i.e. using dbeditor++):

DWORD Tipper/MinHeight
DWORD Tipper/MinWidth
WORD Tipper/AvatarPadding
WORD Tipper/TextPadding		(space between lines of text)
DWORD Tipper/SidebarWidth
BYTE Tipper/MouseTollerance  (distance mouse can move while tip is shown, before it is automatically closed, in pixels)
BYTE Tipper/AvatarRoundCorners


********************
'Variables' support:
********************

This plugin supports the variables plugin by UnregistereD (http://www.cs.vu.nl/~pboon/variables.zip)

Be sure to use the unicode version!

All text in 'Items' - that is, labels and values, will be processed by variables BEFORE Tipper substitutions are applied.


Good luck and have fun.


Scott
mail@scottellis.com.au
www.scottellis.com.au

p.s. Thanks to Omniwolf and Tweety for compiling autoexec_Tipper(*).ini files
