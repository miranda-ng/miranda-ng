--- include m_database module
local db = require('m_database')

--- Save value to database
-- @param hContact The handle of contact (can be NULL)
-- @param module The name of section
-- @param setting The name of setting
-- @return value The value
db.WriteContactSetting(nil, 'MirLua', 'testByte', true)
db.WriteContactSetting(nil, 'MirLua', 'testNum', -2342)
db.WriteContactSetting(nil, 'MirLua', 'testString', "Hello!")

--- Return value from database
-- @param hContact The handle of contact (can be NULL)
-- @param module The name of section
-- @param setting The name of setting
local str = db.GetContactSetting(nil, 'MirLua', 'testString');

-- print string value if bool value is true
if db.GetContactSetting(nil, 'MirLua', 'testByte') then
  print(str)
  end

--- Delete value from database
-- @param hContact The handle of contact (can be NULL)
-- @param module The name of section
-- @param setting The name of setting
db.DeleteContactSetting(nil, 'MirLua', 'testString');
