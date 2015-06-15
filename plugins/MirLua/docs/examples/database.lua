--- include m_database module
local db = require('m_database')

--- Save value to database
-- @param hContact The handle of contact (can be NULL)
-- @param module The name of section
-- @param setting The name of setting
-- @return value The value
db.WriteContactSetting(nil, 'MirLua', 'testNum', -2342)

--- Return value from database
-- @param hContact The handle of contact (can be NULL)
-- @param module The name of section
-- @param setting The name of setting
-- @param default The value which will be returned if setting doesn't not exists
local bValue = db.GetContactSetting(nil, 'MirLua', 'testByte');

-- print string value if bool value is true
if bValue then
  local sValue = db.GetContactSetting(nil, 'MirLua', 'testString', 'Hello!')
  print(sValue)
  end

--- Delete value from database
-- @param hContact The handle of contact (can be NULL)
-- @param module The name of section
-- @param setting The name of setting
db.DeleteContactSetting(nil, 'MirLua', 'testNum');
