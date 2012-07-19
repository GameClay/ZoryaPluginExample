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
#include <string.h>

#include <FlightlessManicotti/fm.h>
#include <FlightlessManicotti/process/process.h>
#include <FlightlessManicotti/render/mesh/mesh.h>
#include <FlightlessManicotti/render/opengl/gl_render.h>

/**
 * The script render context to use for locking the resource OpenGL
 * context.
 */
extern kl_render_context_t g_script_render_context;

/* Brought in from FlightlessManicotti */
extern int Mesh_pushexisting(lua_State* L, kl_mesh_t* mesh);

/* Forward declaration. */
int KL_API luaopen_ZoryaPluginExample(lua_State* L);

/**
 * The registration name of the metatable for this plugin.
 *
 * Think of this like the name of a class. The rest of the functions
 * in this file will define static methods and instance methods on this 'class'.
 */
const char* ZORYA_PLUGIN_EXAMPLE_LIB = "ZoryaPluginExample";

/**
 * Plugin example struct.
 */
typedef struct ZoryaPluginExample {
   uint32_t pid;     /**< Process id assigned by the FlightlessManicotti process manager */
   uint32_t active;  /**< Boolean indicating if this instance is active in the Manicotti process manager */
} ZoryaPluginExample;

/**
 * Example of a plugin-managed mesh that can be exposed to Lua.
 */
static kl_mesh_t s_ExampleMesh;

/**
 * Callback function for advance time events.
 *
 * Advance time events are sent at a rate of 1 per frame, and occur before
 * each frame is rendered.
 *
 * @param dt      The time, in miliseconds, since the last frame.
 * @param context The context pointer provided to the process manager during kl_reserve_process_id.
 */
static void _ZoryaPluginExample_advance_time(float dt, void* context)
{
   ZoryaPluginExample* example = (ZoryaPluginExample*)context;
   kl_log("Plugin example (pid: %d) advance_time %f", example->pid, dt);
}

/**
 * Function called by Lua to allocate a new instance of the example.
 */
static int ZoryaPluginExample_new(lua_State* L)
{
   /* Allocate a plugin instance as userdata, memory managed by Lua. */
   ZoryaPluginExample* example = lua_newuserdata(L, sizeof(ZoryaPluginExample));

   /* Assign the metatable to the newly allocated userdata. */
   luaL_getmetatable(L, ZORYA_PLUGIN_EXAMPLE_LIB);
   lua_setmetatable(L, -2);

   /* Reserve a process id from the FlightlessManicotti process manager,
       and mark the instance as active */
   example->pid = kl_reserve_process_id(KL_DEFAULT_PROCESS_MANAGER,
      NULL, _ZoryaPluginExample_advance_time, example);
   example->active = 1;

   kl_log("Creating plugin example (pid: %d)", example->pid);

   return 1;
}

static int ZoryaPluginExample_getmesh(lua_State* L)
{
   return Mesh_pushexisting(L, &s_ExampleMesh);
}

static int ZoryaPluginExample_startup(lua_State* L)
{
   KL_UNUSED(L);

   /* Initialize the shared mesh object */
   memset(&s_ExampleMesh, 0, sizeof(kl_mesh_t));
   CGLSetCurrentContext(g_script_render_context->resourceCGLContext);
   CGLLockContext(g_script_render_context->resourceCGLContext);
   kl_mesh_init(&s_ExampleMesh);
   CGLUnlockContext(g_script_render_context->resourceCGLContext);

   return 0;
}

static int ZoryaPluginExample_shutdown(lua_State* L)
{
   KL_UNUSED(L);

   /* Destroy the shared mesh object */
   CGLSetCurrentContext(g_script_render_context->resourceCGLContext);
   CGLLockContext(g_script_render_context->resourceCGLContext);
   kl_mesh_deinit(&s_ExampleMesh);
   CGLUnlockContext(g_script_render_context->resourceCGLContext);

   kl_heap_free(s_ExampleMesh.vertex);
   kl_heap_free(s_ExampleMesh.normal);
   kl_heap_free(s_ExampleMesh.tex0);
   kl_heap_free(s_ExampleMesh.col0);
   kl_heap_free(s_ExampleMesh.index);
   return 0;
}

/**
 * Function called by Lua to deactivate an instance of the example.
 */
static int ZoryaPluginExample_deactivate(lua_State* L)
{
   ZoryaPluginExample* example = (ZoryaPluginExample*)lua_touserdata(L, 1);

   if(example->active == 1)
   {
      /* Release the process id, removing this instance from the FlightlessManicotti
         process manager, and mark the instance as inactive. */
      kl_log("Deactivating plugin example (pid: %d)", example->pid);
      kl_release_process_id(KL_DEFAULT_PROCESS_MANAGER, example->pid);
      example->active = 0;
   }

   return 0;
}

/**
 * Function called automatically by Lua when a plugin instance gets garbage collected.
 */
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

/**
 * Lua function name, and C function pointer pairs which can be called on
 * an instance of the plugin. Think of these as member methods on a class.
 */
static const struct luaL_reg ZoryaPluginExample_instance_methods [] = {
   {"deactivate", ZoryaPluginExample_deactivate},
   {NULL, NULL}
};

/**
 * Lua function name, and C function pointer pairs which can be called on
 * without an instance of the plugin. Think of these as static methods on a class.
 */
static const struct luaL_reg ZoryaPluginExample_class_methods [] = {
   {"new", ZoryaPluginExample_new},
   {"getmesh", ZoryaPluginExample_getmesh},
   {"startup", ZoryaPluginExample_startup},
   {"shutdown", ZoryaPluginExample_shutdown},
   {NULL, NULL}
};

/**
 * This function is called by Lua when the native library is brought into the
 * Lua runtime using 'require'. It registers the static and instance methods
 * for the plugin. */
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
