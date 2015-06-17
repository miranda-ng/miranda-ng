--- include m_toptoolbar module
local ttb = require('m_toptoolbar')
--- include m_icolib module
local icolib = require('m_icolib')

local TTBBF_VISIBLE = tonumber("0002", 16)

m.OnModulesLoaded(function()
  ttb.OnTopToolBarLoaded(function()
    local ttbButton =
    {
      -- required field
      Name = "MirLua",

      Service = nil,
      Flags = TTBBF_VISIBLE,

      IconUp = nil,
      TooltipUp = "Up state",
      wParamUp = nil,
      lParamUp = nil,

      IconDown = nil,
      TooltipDown = "Down state",
      wParamDown = nil,
      lParamDown = nil
    }

    --- Add icons for top toolbar
    ttbButton.IconUp = icolib.AddIcon('testTTBIconUp', 'Lua icon for ttbButtonUp')
    ttbButton.IconDown = icolib.AddIcon('testTTBIconDn', 'Lua icon for ttbButtonUp')

    --- Add button on top toolbar
    ttb.Service = "Srv/TTB"
    ttb.AddButton(ttbButton)

    --- Create the top toolbar button which will be deleted below
    local hTTButton = ttb.AddButton({
      Name = "MirLua",
        TooltipUp = "Up state to delete",
        TooltipDown = "Down state to delete"
      })

    --- Remove button from top toolbar
    ttb.RemoveButton(hTTButton)
  end)
end)
