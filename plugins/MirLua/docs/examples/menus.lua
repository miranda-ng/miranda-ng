--- include m_menus module
require('m_menus')

--- Add icon for menu items
local hIcon = M.Icons.AddIcon('testMenuIcon', 'Lua icon for menus')

--- Add menu item to main menu
-- @param name The name of menu item
-- @param flags The flugs that determine behaviour of menu item (default 0)
-- @param position The position of menu item in main menu (default 0)
-- @param icon The handle of icon of menu item (default NULL)
-- @param service The name of service which will be called (default '')
-- @return handle of menu item
M.Menus.AddMainMenuItem('Main menu item', 0, 0, hIcon, 'Srv/MMI')

--- Add menu item to contact menu
-- @param name The name of menu item
-- @param flags The flugs that determine behaviour of menu item (default 0)
-- @param position The position of menu item in main menu (default 0)
-- @param icon The handle of icon of menu item (default NULL)
-- @param service The name of service which will be called (default '')
-- @return handle of menu item
M.Menus.AddContactMenuItem('Contact menu item', 0, 0, hIcon, 'Srv/CMI')

--- Create the contact menu item which will be deleted below
local hMenuItem = M.Menus.AddContactMenuItem('testRemove', 0, 0, 0, 'Srv/TestRemove')

--- Remove menu item from parent menu
-- @param handle The handle of menu item
-- @return 0 on success
M.Menus.RemoveMenuItem(hMenuItem);
