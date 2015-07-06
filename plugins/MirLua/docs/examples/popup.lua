--- include m_popup module
local popup = require('m_popup')
--- include m_clist module
local clist = require('m_clist')

m.CreateServiceFunction('MirLua/ShowPopup', function()
  local popupData =
  {
    Title = 'Title',
    Text = 'Popup content',
    hContact = 0,
    Flags = 1
  }
  popup.AddPopup(popupData)
end)

clist.AddMainMenuItem({ Name = "Show lua popup", Service = 'MirLua/ShowPopup' })
