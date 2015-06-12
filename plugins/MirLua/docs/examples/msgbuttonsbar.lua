--- include m_msg_buttonsbar module
local mbb = require('m_msg_buttonsbar')
--- include m_icolib module
local icolib = require('m_icolib')

local BBBF_ISLSIDEBUTTON = 64

m.OnModulesLoaded(function()
  local bbButton =
  {
    -- required fields
    Module = "MirLua",
    ButtonID = 1,

    Flags = BBBF_ISLSIDEBUTTON,
    Tooltip = "Msg button",
    Icon = icolib.AddIcon('testBBBIcon', 'Lua icon for bbbButton')
  }

  mbb.OnMsgToolBarLoaded(function()
    --- Add button on msg buttons bar
    mbb.AddButton(bbButton)

    --- Create the msg buttons bar button which will be deleted below
    mbb.AddButton({
        Module = "MirLua",
        ButtonID = 2,
        Flags = BBBF_ISLSIDEBUTTON,
        Tooltip = "Msg button for deletion"
      })
  end)

  mbb.OnMsgToolBarButtonPressed(function(w, l)
      if l.Module == "MirLua" and l.ButtonID == 1 then
        --- Remove button from msg buttons bar
        mbb.RemoveButton("MirLua", 2)
      end
    end)
end)
