-- -*- Mode: C; tab-width: 3; c-basic-offset: 3; indent-tabs-mode: nil -*-
-- vim: set filetype=C tabstop=3 softtabstop=3 shiftwidth=3 expandtab:
require 'Zorya.PlugIn'

-- Don't nuke this plugin namespace if it exists
ExamplePlugin = ExamplePlugin or {}

-- Bring in PlugIn base class functionality.
ExamplePlugin.Native = Zorya.PlugIn:new()

function ExamplePlugin.Native:new(o)
   o = o or {}
   setmetatable(o, self)
   self.__index = self

   return o
end

function ExamplePlugin.Native:onregister(plugin_manager)
   self:reload()
end

function ExamplePlugin.Native:reload()
   -- Get path of this file.
   local fullpath = debug.getinfo(1).source
   local f, l, filepath = string.find(fullpath, "@(.+)%/[A-Za-z]+.lua")

   -- Native library.
   package.cpath = filepath.."/?.dylib;"..package.cpath
   require 'ZoryaPluginExample'
end

function ExamplePlugin.Native:getvisualizations()
   return {{"NativePluginExample", "A native library plugin example"}}
end

function ExamplePlugin.Native:getname()
   return "NativePluginExample"
end

function ExamplePlugin.Native:getvisualization(vis_name)
   if vis_name == "NativePluginExample" then
      return ExamplePlugin.Native.Visualization:new()
   end
   return nil
end

-- Bring in Visualization base class functionality.
ExamplePlugin.Native.Visualization = Zorya.Visualization:new()

function ExamplePlugin.Native.Visualization:new(o)
   o = o or {}
   setmetatable(o, self)
   self.__index = self

   return o
end

function ExamplePlugin.Native.Visualization:activate(render_elem)
   ZoryaPluginExample.helloworld()
end

-- Register PlugIn
Zorya.PlugInManager.register(ExamplePlugin.Native)
