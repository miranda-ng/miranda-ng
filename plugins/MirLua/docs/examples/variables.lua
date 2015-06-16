local vars = require('m_variables')

m.OnModulesLoaded(function()
    print(vars.FormatString('?add(2,2)'))
  end)
