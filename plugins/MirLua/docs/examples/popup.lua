--- include m_popup module
local popup = require('m_popup')
--- include m_genmenu module
local genmenu = require('m_genmenu')

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

m.OnModulesLoaded(function()
  genmenu.AddMainMenuItem({ Name = "Show lua popup", Service = 'MirLua/ShowPopup' })
end)
