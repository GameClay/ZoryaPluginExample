/* -*- Mode: C; tab-width: 3; c-basic-offset: 3; indent-tabs-mode: nil -*- */
/* vim: set filetype=C tabstop=3 softtabstop=3 shiftwidth=3 expandtab: */

/* FlightlessManicotti -- Copyright (C) 2010-2012 GameClay LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <lua.h>
#include <lauxlib.h>

#include <FlightlessManicotti/fm.h>
#include <FlightlessManicotti/process/process.h>

int KL_API luaopen_ZoryaPluginExample(lua_State* L);

const char* ZORYA_PLUGIN_EXAMPLE_LIB = "ZoryaPluginExample";

typedef struct ZoryaPluginExample {
   uint32_t pid;
   uint32_t active;
} ZoryaPluginExample;

static void _ZoryaPluginExample_advance_time(float dt, void* context)
{
   ZoryaPluginExample* example = (ZoryaPluginExample*)context;
   kl_log("Plugin example (pid: %d) advance_time %f", example->pid, dt);
}

static int ZoryaPluginExample_new(lua_State* L)
{
   ZoryaPluginExample* example = lua_newuserdata(L, sizeof(ZoryaPluginExample));;
   luaL_getmetatable(L, ZORYA_PLUGIN_EXAMPLE_LIB);
   lua_setmetatable(L, -2);

   example->pid = kl_reserve_process_id(KL_DEFAULT_PROCESS_MANAGER,
      NULL, _ZoryaPluginExample_advance_time, example);
   example->active = 1;

   kl_log("Creating plugin example (pid: %d)", example->pid);

   return 1;
}

static int ZoryaPluginExample_deactivate(lua_State* L)
{
   ZoryaPluginExample* example = (ZoryaPluginExample*)lua_touserdata(L, 1);

   if(example->active == 1)
   {
      kl_log("Deactivating plugin example (pid: %d)", example->pid);
      kl_release_process_id(KL_DEFAULT_PROCESS_MANAGER, example->pid);
      example->active = 0;
   }

   return 0;
}

static int ZoryaPluginExample_gc(lua_State* L)
{
   ZoryaPluginExample* example = (ZoryaPluginExample*)lua_touserdata(L, 1);

   if(example->active == 1)
   {
      ZoryaPluginExample_deactivate(L);
   }

   kl_log("Destroying plugin example (pid: %d)");

   return 0;
}

static const struct luaL_reg ZoryaPluginExample_instance_methods [] = {
   {"deactivate", ZoryaPluginExample_deactivate},
   {NULL, NULL}
};

static const struct luaL_reg ZoryaPluginExample_class_methods [] = {
   {"new", ZoryaPluginExample_new},
   {NULL, NULL}
};

int LUA_API luaopen_ZoryaPluginExample(lua_State* L)
{
   luaL_newmetatable(L, ZORYA_PLUGIN_EXAMPLE_LIB);
   luaL_register(L, 0, ZoryaPluginExample_instance_methods);
   lua_pushvalue(L, -1);
   lua_setfield(L, -2, "__index");
   lua_pushcfunction(L, ZoryaPluginExample_gc);
   lua_setfield(L, -2, "__gc");

   luaL_register(L, ZORYA_PLUGIN_EXAMPLE_LIB, ZoryaPluginExample_class_methods);

   return 1;
}
