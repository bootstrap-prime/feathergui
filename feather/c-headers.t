local acg = require 'feather.abstract-codegen'
local F = require 'feather.shared'

local headers_gen = acg.generator {
  file = [[//THIS FILE AUTOGENERATED
#ifndef $guard
#define $guard

#include <stdint.h> // for integers
#include <stdbool.h> // for bool

$(
)decls

#endif]],
  typedef = "typedef $type $name;",
  struct_fd = "typedef struct $name__ $name;",
  ['struct'] = "struct $name__ {\n  $(;\n  )entries;\n};",
  customstruct = "struct $name__ {\n  $inner\n};",
  ['union'] = "union {\n  $(;\n  )entries;\n};",
  variable = "$type $name",
  variable_fp = "$rettype (* $name)($(, )args$vararg)",
  ['function'] = "$result $name($(, )args$vararg);",
  fp_type = "typedef $rettype (* $name)($(, )parameters$vararg);",
  arg = "$type $name",
  enum = "enum $name {\n  $(,\n  )entries\n};",
  enum_entry = "$name = $value",
  arraytype = "typedef $type $name[$size];"
}

local anon_idx = 0
local function anon_name()
  anon_idx = anon_idx + 1
  return "anon_"..anon_idx
end

-- construct serialization representations of a declaration
-- ensures that the cache contains an entry from this type to the serialized name
-- appends the serialization data and any dependencies to the sequence in the correct order
-- returns the serialized name of the declaration
function handle_declaration(object, prefix, cache, sequence, symbols)
  if terralib.types.istype(object) then
    if cache[object] and cache[object].gen then
      return cache[object].name
    end
    if object == terralib.types.unit then
      return "void"
    end
    if object == F.conststring then
      return "const char*"
    end
    if object:isstruct() then
      if object.convertible == "enum" then
        local name = cache[object].name or prefix .. anon_name()
        local entries = terralib.newlist()
        for k, v in pairs(object.enum_values) do
          --print(k, v)
          entries:insert{ kind = "enum_entry", name = name.."_"..k, value = tostring(v) }
        end
        sequence:insert{ kind = "enum", name = name, entries = entries }
        cache[object].gen = true
        cache[object].name = name
        if terralib.sizeof(object.entries[1].type) ~= 4 then
          cache[object].name = handle_declaration(object.entries[1].type, prefix, cache, sequence, symbols)
        end
        return cache[object].name
      end
      local name = cache[object].name or prefix .. anon_name()
      sequence:insert { kind = "struct_fd", name = name}
      cache[object] = { name = name, gen = true }
      if object.c_export == nil then
        local entries = terralib.newlist()
        for i, entry in ipairs(object.entries) do
          if entry.type ~= nil then
            entries:insert { kind = "variable", type = handle_declaration(entry.type, prefix, cache, sequence, symbols), name = entry.field }
          else -- this is a union
            local nset = terralib.newlist()
            for j, element in ipairs(entry) do
              nset:insert { kind = "variable", type = handle_declaration(element.type, prefix, cache, sequence, symbols), name = element.field }
            end
            entries:insert { kind = "union", entries = nset }
          end
        end
        sequence:insert { kind = "struct", name = name, entries = entries }
      else
        -- Even if we have a custom export string, make sure all the types we are going to use still exist
        for i, entry in ipairs(object.entries) do
          if entry.type ~= nil then
            handle_declaration(entry.type, prefix, cache, sequence, symbols)
          else -- this is a union
            for j, element in ipairs(entry) do
              handle_declaration(element.type, prefix, cache, sequence, symbols)
            end
          end
        end
        sequence:insert { kind = "customstruct", name = name, inner = object.c_export }
      end
      return name
    end
    if object:ispointertofunction() then
      local name = cache[object].name or (prefix .. anon_name())
      cache[object] = { name = name, gen = false }
      local rettype = handle_declaration(object.type.returntype, prefix, cache, sequence, symbols)
      local parameters = terralib.newlist()
      for i, parameter in ipairs(object.type.parameters) do
        parameters:insert(handle_declaration(parameter, prefix, cache, sequence, symbols))
      end
      local vararg = ""
      if object.type.isvararg then 
        vararg = ", ..."
      end

      sequence:insert{ kind = "fp_type", name = name, rettype = rettype, parameters = parameters, vararg = vararg}
      cache[object].gen = true
      return name
    end
    if object:isintegral() then
      return object.name .. "_t"
    end
    if object:isprimitive() then
      return object.name
    end
    if object == &opaque then
      return "void *"
    end
    if object:ispointer() then
      return handle_declaration(object.type, prefix, cache, sequence, symbols) .. " *"
    end
    if object:isarray() then
      local name = prefix .. anon_name()
      cache[object] = { name = name, gen = false }
      sequence:insert{ kind = "arraytype", name = name, type = handle_declaration(object.type, prefix, cache, sequence, symbols), size = object.N }
      cache[object].gen = true
      return name
    end
  end
  if terralib.isfunction(object) then
    local name = cache[object].name
    local rettype = object:gettype().returntype
    local parameters = terralib.newlist()
    for i, parameter in ipairs(object:gettype().parameters) do
      parameters:insert(handle_declaration(parameter, prefix, cache, sequence, symbols))
    end
    local vararg = ""
    if object.type.isvararg then 
      vararg = ", ..."
    end

    sequence:insert { kind = "function", name = prefix..name, rettype = rettype, parameters = parameters, vararg = vararg}
    return prefix .. name
  end
  print("unclassified object", object)
end

local function export(options, symbols)
  options = options or {}
  options.short_name = options.short_name .. "_" or ""
  options.long_name = options.long_name .. "_" or options.short_name
  local cache = setmetatable({}, {__index = function(s, k) s[k] = {name = nil, gen = false}; return s[k] end})
  for k, v in pairs(symbols) do
    cache[v] = {name = options.short_name .. k, gen = false}
  end
  local sequence = terralib.newlist{}
  for k, v in pairs(symbols) do
    handle_declaration(v, options.short_name, cache, sequence)
  end
  return headers_gen {
    kind = "file",
    guard = options.long_name.."H",
    decls = sequence
  }
end

 return export
