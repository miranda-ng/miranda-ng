--- include m_icons module
require('m_icons')

--- Add icon to icoLib
-- @param name The name of icon
-- @param description The description of icon
-- @param section The section in witch icon will be stored (default 'MirLua')
-- @return handle of icon
M.Icons.AddIcon('testIcon', 'Lua icon', 'MirLua')

--- Create the icon which will be deleted below
M.Icons.AddIcon('testRemoved', 'Lua temporary icon')

--- Get icon by name
-- @param name The name of icon
-- @return handle of icon
local hIcon = M.Icons.GetIcon('testRemoved')

--- Remove icon from iconLib
-- @param handle The handle of icon (or name)
-- @return 0 on success
M.Icons.RemoveIcon('testRemoved')
