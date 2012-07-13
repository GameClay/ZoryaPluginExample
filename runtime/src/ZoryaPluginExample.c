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
#include <FlightlessManicotti/fm.h>
#include <lua.h>
#include <lauxlib.h>

int KL_API luaopen_ZoryaPluginExample(lua_State* L);

static int helloworld(lua_State* L)
{
   KL_UNUSED(L);
   kl_log("hello world!");
   return 0;
}

int LUA_API luaopen_ZoryaPluginExample(lua_State* L)
{
   struct luaL_reg driver[] =
   {
      {"helloworld", helloworld},
      {NULL, NULL},
   };

   luaL_openlib(L, "ZoryaPluginExample", driver, 0);
   return 1;
}
