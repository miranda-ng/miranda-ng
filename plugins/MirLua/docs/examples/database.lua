--- include m_database module
local db = require('m_database')

--- Iterate all contact stored in db
-- @param protoName The name of protocol account or nothing
for hContact in db.AllContacts() do
  --print(hContact)
end

local hContact = 15

--- Iterate all contact events stored in db
-- param hContact The handle of contact
for hEvent in db.AllEvents(hContact) do
  --print(hEvent)
end

--- Iterate all setting names stored in db
-- param module The name of module
-- param hContact The handle of contact or nothing
for setting in db.AllSettings('CList') do
  --print(setting)
end

--- Save value to database
-- @param hContact The handle of contact (can be NULL)
-- @param module The name of section
-- @param setting The name of setting
-- @return value The value
db.WriteSetting(nil, 'MirLua', 'testNum', -2342)

--- Return value from database
-- @param hContact The handle of contact (can be NULL)
-- @param module The name of section
-- @param setting The name of setting
-- @param default The value which will be returned if setting doesn't not exists
local bValue = db.GetSetting(nil, 'MirLua', 'testByte');

-- print string value if bool value is true
if bValue then
  local sValue = db.GetSetting(nil, 'MirLua', 'testString', 'Hello!')
  print(sValue)
  end

--- Delete value from database
-- @param hContact The handle of contact (can be NULL)
-- @param module The name of section
-- @param setting The name of setting
db.DeleteSetting(nil, 'MirLua', 'testNum');
