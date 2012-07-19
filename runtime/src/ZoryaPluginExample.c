/* -*- Mode: C; tab-width: 3; c-basic-offset: 3; indent-tabs-mode: nil -*- */
/* vim: set filetype=C tabstop=3 softtabstop=3 shiftwidth=3 expandtab: */

/* ZoryaPluginExample -- Copyright (C) 2010-2012 GameClay LLC
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

#include <Zorya/beat/freq.h>

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
   uint32_t active;  /**< Boolean indicating if this instance is active (for no good reason) */
} ZoryaPluginExample;

/**
 * Example of a plugin-managed mesh that can be exposed to Lua.
 */
static kl_mesh_t s_ExampleMesh;

/**
 * Mesh update PID
 */
static uint32_t s_MeshUpdatePID;

/**
 * Callback function to update the shared mesh.
 *
 * Advance time events are sent at a rate of 1 per frame, and occur before
 * each frame is rendered.
 *
 * @param dt      The time, in miliseconds, since the last frame.
 * @param context The context pointer provided to the process manager during kl_reserve_process_id.
 */
static void _ZoryaPluginExample_MeshUpdate(float dt, void* context)
{
   size_t cqt_sz, i;
   kl_cqt_t cqt = kl_freq_manager_get_cqt(KL_DEFAULT_FREQ_MANAGER);
   float* cqt_spectrum = kl_cqt_get_spectrum(cqt, &cqt_sz);

   KL_UNUSED(context);
   KL_UNUSED(dt);

   for(i = 0; i < cqt_sz; i++)
   {
      float* vert = &s_ExampleMesh.vertex[i * 3];
      vert[1] = cqt_spectrum[i];
   }

   /* Lock resource context, and update mesh */
   CGLSetCurrentContext(g_script_render_context->resourceCGLContext);
   CGLLockContext(g_script_render_context->resourceCGLContext);
   kl_mesh_buffer_data(&s_ExampleMesh,
      kl_mesh_element_vertex, /* Update Mesh only */
      kl_mesh_element_vertex /* And it's a dynamic update */);
   CGLUnlockContext(g_script_render_context->resourceCGLContext);
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

   /* Mark the instance as active */

   example->active = 1;

   return 1;
}

static int ZoryaPluginExample_getmesh(lua_State* L)
{
   return Mesh_pushexisting(L, &s_ExampleMesh);
}

static int ZoryaPluginExample_startup(lua_State* L)
{
   kl_cqt_t cqt = kl_freq_manager_get_cqt(KL_DEFAULT_FREQ_MANAGER);
   int i, spectrum_sz;

   KL_UNUSED(L);

   /* Initialize the shared mesh object */
   memset(&s_ExampleMesh, 0, sizeof(kl_mesh_t));

   /* Allocate mesh vertices and indices */
   spectrum_sz = cqt->binsPerOctave * cqt->num_octaves;
   s_ExampleMesh.vertex = kl_heap_alloc(sizeof(float) * 3 * spectrum_sz);
   s_ExampleMesh.num_verts = spectrum_sz;
   s_ExampleMesh.index = kl_heap_alloc(sizeof(uint16_t) * spectrum_sz);
   s_ExampleMesh.num_indices = spectrum_sz;

   for(i = 0; i < spectrum_sz; i++)
   {
      float* vert = &s_ExampleMesh.vertex[i * 3];
      vert[0] = (float)i / (float)spectrum_sz;
      vert[1] = 0.0f; /* Will be filled in during update */
      vert[2] = (float)i / (float)cqt->binsPerOctave;
      s_ExampleMesh.index[i] = (uint16_t)i;
   }

   /* Do GPU init and update */
   kl_render_resource_lock(g_script_render_context);
   kl_mesh_init(&s_ExampleMesh);
   kl_mesh_buffer_data(&s_ExampleMesh,
      kl_mesh_element_vertex | kl_mesh_element_index, /* Update these elements */
      kl_mesh_element_vertex /* These elements should be marked as dynamic */);
   kl_render_resource_unlock(g_script_render_context);

   /* Wire in to the process manager */
   s_MeshUpdatePID = kl_reserve_process_id(KL_DEFAULT_PROCESS_MANAGER,
      NULL, _ZoryaPluginExample_MeshUpdate, NULL);

   return 0;
}

static int ZoryaPluginExample_shutdown(lua_State* L)
{
   KL_UNUSED(L);

   /* Remove self from process manager */
   kl_release_process_id(KL_DEFAULT_PROCESS_MANAGER, s_MeshUpdatePID);

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
      kl_log("Deactivating plugin example %p", example);
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

   /* Note that there is no need to delete things allocated with lua_newuserdata,
   that memory is managed by Lua. */
   KL_UNUSED(example);

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
