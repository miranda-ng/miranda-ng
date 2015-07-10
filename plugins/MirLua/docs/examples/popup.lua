--- include m_popup module
local popup = require('m_popup')
--- include m_clist module
local clist = require('m_clist')

m.CreateServiceFunction('MirLua/ShowPopup', function()
  local popupData =
  {
    ContactName = 'Contact',
    Text = 'Popup content',
    hContact = 0
  }
  popup.AddPopup(popupData)
end)

m.CreateServiceFunction('MirLua/ShowPopup2', function()
  local popupData =
  {
    Title = 'Title',
    Text = 'Popup content',
    hContact = 0,
    Flags = 1
  }
  popup.AddPopup2(popupData)
end)

clist.AddMainMenuItem({ Name = "Show lua popup", Service = 'MirLua/ShowPopup' })
clist.AddMainMenuItem({ Name = "Show lua popup2", Service = 'MirLua/ShowPopup2' })
