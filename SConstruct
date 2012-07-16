import os, sys, platform, subprocess

# Set up an environment to specify our own OS/CPU defines
platform_defines = Environment()
platform_defines.SetDefault(ZIG_OS_ANDROID = 'KL_OS_ANDROID')
platform_defines.SetDefault(ZIG_OS_DARWIN = 'KL_OS_DARWIN')
platform_defines.SetDefault(ZIG_OS_IOS = 'KL_OS_IOS')
platform_defines.SetDefault(ZIG_OS_WINDOWS = 'KL_OS_WINDOWS')

platform_defines.SetDefault(ZIG_CPU_ARM = 'KL_ARCH_ARM')
platform_defines.SetDefault(ZIG_CPU_X86 = 'KL_ARCH_X86')
platform_defines.SetDefault(ZIG_CPU_X64 = 'KL_ARCH_X64')

# Include the core
env = SConscript(['Ziggurat/SConscript'], exports = 'platform_defines')

#if sys.platform=="win32":
#   if env.GetOption('sse')==1: env['CCFLAGS']+=[ "/arch:SSE" ]
#   elif env.GetOption('sse')>=2: env['CCFLAGS']+=[ "/arch:SSE2" ]
#      if   env.GetOption('sse')>=3: env['CPPDEFINES']+=[("__SSE3__", 1)]
#      if   env.GetOption('sse')>=4: env['CPPDEFINES']+=[("__SSE4__", 1)]
#else:
#   if env.GetOption('sse'):
#      env['CCFLAGS']+=["-mfpmath=sse"]
#      if env.GetOption('sse')>1: env['CCFLAGS']+=["-msse%s" % str(env.GetOption('sse'))]
#      else: env['CCFLAGS']+=["-msse"]

# Am I building for Windows or POSIX?
if (env['OS_GROUP'] == 'windows'):
   
   #
   # Pre-Processor defines
   #
   env['CPPDEFINES']+=["WIN32", "_WINDOWS", "UNICODE", "_UNICODE", "_CRT_SECURE_NO_WARNINGS"]
   
   #
   # Visual Studio Compiler Flags
   #
   env['CCFLAGS']+=["/nologo"]         # Suppress the version spam before every file is compiled
   
   env['CCFLAGS']+=["/GF"]             # String pooling
   env['CCFLAGS']+=["/Gy"]             # Enable function-level linking
   env['CCFLAGS']+=["/Zi"]             # Program database debug info
   
   env['CCFLAGS']+=["/WX"]             # Warnings as errors
   env['CCFLAGS']+=["/W3"]             # Warning level 3
   
   env['CCFLAGS']+=["/EHs"]            # No exceptions
   
   if env.GetOption('debug'):
       env['CCFLAGS']+=["/Od", "/MTd"]
   else:
       env['CCFLAGS']+=["/O2", "/MT"]
       env['CCFLAGSFORNEDMALLOC']+=["/GL"]         # Do link time code generation
       
   #
   # Visual Studio Linker Settings
   #
   env['LINKFLAGS']+=["/DEBUG"]                # Output debug symbols
   env['LINKFLAGS']+=["/LARGEADDRESSAWARE"]    # Works past 2Gb
   env['LINKFLAGS']+=["/DYNAMICBASE"]          # Doesn't mind being randomly placed
   env['LINKFLAGS']+=["/NXCOMPAT"]             # Likes no execute

   #env['LINKFLAGS']+=["/ENTRY:DllPreMainCRTStartup"]
   env['LINKFLAGS']+=["/VERSION:1.10.0"]        # Version
   #env['LINKFLAGS']+=["/MANIFEST"]             # Be UAC compatible
   env['LINKFLAGS']+=["/WX"]                    # Warnings as errors

   if not env.GetOption('debug'):
       env['LINKFLAGS']+=["/OPT:REF", "/OPT:ICF"]  # Eliminate redundants
else:
   # Common GCC flags
   env.Append(CPPFLAGS = ['-fno-exceptions'])

   if env['OS'] == 'ios':
      env.Append(CPPFLAGS=['-Wno-constant-conversion',
                           '-Wno-unused-value'])

   if env['VARIANT'] == 'release':
      env.Append(CPPFLAGS=['-Os'])
   else:
      env.Append(CPPFLAGS=['-O0','-g'])

# FlightlessManicotti paths
if '' == env.subst('$MANICOTTI'):
   env['MANICOTTI'] = '../FlightlessManicotti/$BUILDDIR/dist'
env.Append(CPPPATH = [env.Dir('$MANICOTTI/include').abspath])
env.Append(LIBPATH = [env.Dir('$MANICOTTI/lib').abspath])

# Zorya paths
if '' == env.subst('$ZORYA'):
   env['ZORYA'] = '../Zorya/$BUILDDIR/dist'
env.Append(CPPPATH = [env.Dir('$ZORYA/include').abspath])
env.Append(LIBPATH = [env.Dir('$ZORYA/lib').abspath])

## Now build the runtime and example
runtime_library = env.SConscript('runtime/SConscript', variant_dir='$OBJDIR/runtime', duplicate = False)

# Install in plugin folder
installed_lib = env.Install('plugin/NativePluginExample', runtime_library)

# Change search path for dependenant libs
def rename_deps(target, source, env):
   for libname in ['libFlightlessManicotti.dylib', 'libZorya.dylib']:
      rename_cmd = "install_name_tool -change " + \
         str(env.Dir('$OBJDIR/runtime')) + "/" + libname + " " +\
         "@executable_path/../Frameworks/" + libname + " " + \
         str(source[0])
      print(rename_cmd)
      subprocess.call(rename_cmd, shell=True)

bldr = Builder(action = rename_deps)
env.Append(BUILDERS = {'RenameDeps' : bldr })
env.RenameDeps('renamed_lib', installed_lib)
