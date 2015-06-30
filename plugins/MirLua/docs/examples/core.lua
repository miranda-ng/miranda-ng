-- core module (m) is included by default
--- include m_clist module
local clist = require('m_clist')
-- include m_icolib module
local icolib = require('m_icolib')

-- Add icon for menu items
local hRestartIcon = icolib.AddIcon('restartIcon', 'Restart')

-- Subscribe to [[Miranda/System/ModulesLoaded]] event
m.OnModulesLoaded(function()
    -- Add menu item to main menu that allow to restart Miranda NG
    clist.AddMainMenuItem("Restart", 0, 0, hRestartIcon, "Miranda/System/Restart")
  end)
