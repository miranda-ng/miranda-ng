--- include m_icolib module
local icolib = require('m_icolib')

--- Add icon to icoLib
-- @param name The name of icon
-- @param description The description of icon
-- @param section The section in witch icon will be stored (default 'MirLua')
-- @return handle of icon
icolib.AddIcon('testIcon', 'Lua icon', 'MirLua')

--- Create the icon which will be deleted below
icolib.AddIcon('testRemoved', 'Lua temporary icon')

--- Get icon by name
-- @param name The name of icon
-- @return handle of icon
local hIcon = icolib.GetIcon('testRemoved')

--- Remove icon from iconLib
-- @param handle The handle of icon (or name)
-- @return 0 on success
icolib.RemoveIcon('testRemoved')
