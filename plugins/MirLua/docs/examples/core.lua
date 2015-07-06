-- core module (m) is included by default
--- include m_clist module
local clist = require('m_clist')
--- include m_genmenu module
local genmenu = require('m_genmenu')
-- include m_icolib module
local icolib = require('m_icolib')

-- Add icon for menu items
local hRestartIcon = icolib.AddIcon('restartIcon', 'Restart')

-- Subscribe to Lua script loaded event
m.OnScriptLoaded(function()
  -- Add menu item to main menu that allow to restart Miranda NG
  hRestartMenuItem = clist.AddMainMenuItem({ Name = "Restart", Icon = hRestartIcon, Service = "Miranda/System/Restart" })
end)

-- Subscribe to Lua script unload event
m.OnScriptUnload(function()
  -- remove menu item from main menu
  hRestartMenuItem = genmenu.RemoveMenuItem(hRestart)
end)
