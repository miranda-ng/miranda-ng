-- core module (m) is included by default
-- include m_genmenu module
-- include m_icolib module
local genmenu = require('m_genmenu')
local icolib = require('m_icolib')

-- Add icon for menu items
local hRestartIcon = icolib.AddIcon('testMenuIcon', 'Lua icon for menus')


-- Subscribe to [[Miranda/System/ModulesLoaded]] event
m.OnModulesLoaded(function()
    -- Add menu item to main menu that allow to restart Miranda NG
    genmenu.AddMainMenuItem("Restart", 0, 0, hRestartIcon, "Miranda/System/Restart")
  end)
