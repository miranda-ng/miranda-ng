--- include m_genmenu module
local genmenu = require('m_genmenu')
--- include m_icolib module
local icolib = require('m_icolib')

local menuItem =
{
  -- required field
  Name = "Menu item",
  Flags = 0,
  Position = 0,
  Icon = nil,
  Service = nil,
  Parent = nil
}

--- Add icon for menu items
local hIcon = icolib.AddIcon('testMenuIcon', 'Lua icon for menus')

--- Add menu item to main menu
menuItem.Name = "Main menu item"
menuItem.Icon = hIcon
menuItem.Service = "Srv/MMI"
genmenu.AddMainMenuItem(menuItem)

--- Add menu item to main menu
menuItem.Name = "Tray menu item"
menuItem.Service = "Srv/TMI"
genmenu.AddTrayMenuItem(menuItem)

--- Add menu item to contact menu
menuItem.Name = "Contact menu item"
menuItem.Service = "Srv/CMI"
genmenu.AddContactMenuItem(menuItem)

--- Create the contact menu item which will be deleted below
menuItem.Name = "testRemove"
menuItem.Service = "Srv/TestRemove"
local hMenuItem = genmenu.AddContactMenuItem(menuItem)

--- Remove menu item from parent menu
genmenu.RemoveMenuItem(hMenuItem)

--- Add root menu item
local CMIF_ROOTHANDLE = 384
local hRoot = genmenu.AddMainMenuItem({ Name = "Main menu root", Flags = CMIF_ROOTHANDLE })

--- Add child menu item
menuItem.Name = "Main menu child wierd"
menuItem.Flags = CMIF_ROOTHANDLE
menuItem.Service = 'Srv/SMI'
menuItem.Parent = hRoot
local hChild = genmenu.AddMainMenuItem(menuItem)

--- Modify menu item
local CMIM_NAME = tonumber("80000000", 16)
genmenu.ModifyMenuItem(hChild, "Main menu child", hIcon, CMIM_NAME)


local hDisabledMenuItem = genmenu.AddMainMenuItem({ Name = "Disabled main menu item" })
genmenu.EnableMenuItem(hDisabledMenuItem, false)

local hHiddenMenuItem = genmenu.AddMainMenuItem({ Name = "Hidden main menu item" })
genmenu.ShowMenuItem(hHiddenMenuItem, false)

local hCheckedMenuItem = genmenu.AddMainMenuItem({ Name = "Checked main menu item" })
genmenu.CheckMenuItem(hCheckedMenuItem, true)
