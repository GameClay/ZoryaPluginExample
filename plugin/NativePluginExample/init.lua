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

   -- Append the path of this plugin to paths searched to load native packages.
   package.cpath = filepath.."/?.dylib;"..package.cpath

   -- Import the native plugin, this will call the 'luaopen_ZoryaPluginExample'
   -- function in the C library.
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
   -- Create a new instance of the native plugin, this will call the
   -- ZoryaPluginExample_new function in the native library.
   print("Creating new native plugin instance...")
   self.nativePlugin = ZoryaPluginExample.new()
end

function ExamplePlugin.Native.Visualization:deactivate(render_elem)
   -- Call deactivate on our instance of the native plugin. Calling an
   -- instance method is done using the ':' operator.

   -- This will invoke the ZoryaPluginExample_deactivate function in the native library,
   -- and automatically push the userdata value for the instance onto the stack.
   self.nativePlugin:deactivate()
   self.nativePlugin = nil
end

-- Register PlugIn with the Zorya PlugIn manager.
Zorya.PlugInManager.register(ExamplePlugin.Native)
