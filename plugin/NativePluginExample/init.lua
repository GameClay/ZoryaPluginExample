-- -*- Mode: lua; tab-width: 3; c-basic-offset: 3; indent-tabs-mode: nil -*-
-- vim: set filetype=lua tabstop=3 softtabstop=3 shiftwidth=3 expandtab:

-- ZoryaPluginExample -- Copyright (C) 2010-2012 GameClay LLC
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.
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
   -- Get path of this file.
   local fullpath = debug.getinfo(1).source
   local f, l, filepath = string.find(fullpath, "@(.+)%/[A-Za-z]+.lua")

   -- Append the path of this plugin to paths searched to load native packages.
   package.cpath = filepath.."/?.dylib;"..package.cpath

   -- Import the native plugin, this will call the 'luaopen_ZoryaPluginExample'
   -- function in the C library.
   require 'ZoryaPluginExample'

   ZoryaPluginExample.startup()
   self:reload()
end

function ExamplePlugin.Native:onunregister(plugin_manager)
   -- Release native resources when this plugin is shut down
   ZoryaPluginExample.shutdown()
end

function ExamplePlugin.Native:reload()
   -- (Re)load the shader.
   local fullpath = debug.getinfo(1).source
   local f, l, filepath = string.find(fullpath, "@(.+)%/[A-Za-z]+.lua")
   dofile(filepath..'/CQTNativeSpectrum.lua')
end

function ExamplePlugin.Native:getvisualizations()
   return {{"CQT Native Spectrum", "A native library plugin example that duplicates the CQTSpectrum plugin using native code to update a vertex buffer (slower)"}}
end

function ExamplePlugin.Native:getname()
   return "NativePluginExample"
end

function ExamplePlugin.Native:getvisualization(vis_name)
   if vis_name == "CQT Native Spectrum" then
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

   -- Create a new instance of the native plugin, this will call the
   -- ZoryaPluginExample_new function in the native library.
   self.nativePlugin = ZoryaPluginExample.new()

   -- CQT Spectrum from a native mesh
   local effect = ShaderManager.geteffect("CQTNativeSpectrum")
   self.spectrum = RenderInstance.new()
   self.spectrum:settransform()
   self.spectrum:setmesh(ZoryaPluginExample.getmesh()) -- Get the shared mesh from native code
   self.spectrum:seteffect(effect)
   self.spectrum:setdrawtype(RenderInstance.drawtype.points)

   self.constant_buffer = ConstantBuffer.new(effect)
   self.constant_buffer['cqtColors']:set(cqt.colortable)
   self.spectrum:setconstantbuffer(self.constant_buffer)

   return o
end

function ExamplePlugin.Native.Visualization:activate(render_elem)
   render_elem:setrenderinstances({self.spectrum})
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
