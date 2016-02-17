--[[
Author: Julio Manuel Fernandez-Diaz
Date:   January 12, 2007
(For Lua 5.1)

Modified slightly by RiciLake to avoid the unnecessary table traversal in tablecount()

Formats tables with cycles recursively to any depth.
The output is returned as a string.
References to other tables are shown as values.
Self references are indicated.

The string returned is "Lua code", which can be procesed
(in the case in which indent is composed by spaces or "--").
Userdata and function keys and values are shown as strings,
which logically are exactly not equivalent to the original code.

This routine can serve for pretty formating tables with
proper indentations, apart from printing them:

print(table.show(t, "t"))   -- a typical use

Heavily based on "Saving tables with cycles", PIL2, p. 113.

Arguments:
t is the table.
name is the name of the table (optional)
indent is a first indentation (optional).
--]]
local debug = require('debug')
local dbg_getfenv = debug.getfenv or debug.getuservalue

function table.show(t, name, indent)
  local cart     -- a container
  local autoref  -- for self references

  --[[ counts the number of elements in a table
  local function tablecount(t)
  local n = 0
  for _, _ in pairs(t) do n = n+1 end
  return n
  end
  ]]
  -- (RiciLake) returns true if the table is empty
  local function isemptytable(t) return type(t) == "table" and next(t) == nil end

  local function basicSerialize (o)
    local so = tostring(o)
    if type(o) == "function" then
      local info = debug.getinfo(o, "S")
      -- info.name is nil because o is not a calling level
      if info.what == "C" then
        return string.format("%q", so .. ", C function")
      else
        -- the information is defined through lines
        return string.format("%q", so .. ", defined in (" ..
        info.linedefined .. "-" .. info.lastlinedefined ..
        ")" .. info.source)
      end
    elseif type(o) == "number" or type(o) == "boolean" then
      return so
    else
      return string.format("%q", so)
    end
  end

  local function addtocart (value, name, indent, saved, field)
    indent = indent or ""
    saved = saved or {}
    field = field or name

    cart = cart .. indent .. field

    if type(value) == "table" then
      if saved[value] then
        cart = cart .. " = {}; -- " .. saved[value] .. " (self reference)\n"
        autoref = autoref ..  name .. " = " .. saved[value] .. ";\n"
      else
        saved[value] = name
        --if tablecount(value) == 0 then
        if isemptytable(value) then
          cart = cart .. " = {};\n"
        else
          cart = cart .. " = {\n"
          for k, v in pairs(value) do
            k = basicSerialize(k)
            local fname = string.format("%s[%s]", name, k)
            field = string.format("[%s]", k)
            -- three spaces between levels
            addtocart(v, fname, indent .. "   ", saved, field)
          end
          for k, v in pairs{env = dbg_getfenv(value), mt = debug.getmetatable(value)} do
            k = basicSerialize(k)
            local fname = string.format("%s[%s]", name, k)
            field = string.format("[%s]", k)
            -- three spaces between levels
            addtocart(v, fname, indent .. "   ", saved, field)
          end
          cart = cart .. indent .. "};\n"
        end
      end
    elseif type(value) == "userdata" then
      if saved[value] then
        cart = cart .. " = " .. basicSerialize(value) .. "; -- " .. saved[value] .. " (self reference)\n"
        autoref = autoref ..  name .. " = " .. saved[value] .. ";\n"
      else
        saved[value] = name
        cart = cart .. " = " .. basicSerialize(value) .. " {\n"
        for k, v in pairs{env = dbg_getfenv(value), mt = debug.getmetatable(value)} do
          k = basicSerialize(k)
          local fname = string.format("%s[%s]", name, k)
          field = string.format("[%s]", k)
          -- three spaces between levels
          addtocart(v, fname, indent .. "   ", saved, field)
        end
        cart = cart .. indent .. "};\n"
      end
    elseif type(value) == "function" then
      cart = cart .. " = " .. basicSerialize(value)
      if debug.getupvalue(value, 1) == nil then
        cart = cart .. ";\n"
      else
        cart = cart .. " {\n"
        local i = 1
        while true do
          local k, v = debug.getupvalue(value, i)
          if k == nil and v == nil then break end
          k = basicSerialize(i)
          local fname = string.format("%s[%s]", name, k)
          field = string.format("[%s]", k)
          -- three spaces between levels
          addtocart(v, fname, indent .. "   ", saved, field)
          i = i + 1
        end
        cart = cart .. indent .. "};\n"
      end
    else
      cart = cart .. " = " .. basicSerialize(value) .. ";\n"
    end
  end

  name = name or "__unnamed__"
  if type(t) ~= "table" and type(t) ~= 'userdata' then
    return name .. " = " .. basicSerialize(t)
  end
  cart, autoref = "", ""
  addtocart(t, name, indent)
  return cart .. autoref
end

