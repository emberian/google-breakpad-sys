Contains pre-built binaries for the breakpad_client on Windows. Built with
Visual Studio 2013 Community Edition.  To create these, the process is manual
and error-prone. First, install gyp and python2. (I recommend using msys2, in
which case you can do a nice pacman -S gyp-svn python2).  Then, cd into
`src\client\windows` and run `gyp --no-circular-check breakpad_client.gyp
-Dwin_release_RuntimeLibrary=2 -Dwin_debug_RuntimeLibrary=2`. The first flag
avoids an issue with newer VS projectr files, and the later flags have it use
the `/MD` (Multi-threaded DLL) C/C++ runtime. Then, open the generated `.sln`
file. In the solution explorer, delete the test app, unittest project, and
sender project. Then, expand `..` in the common project until the source files
are visible. Delete the http upload source file. Right-click each project and in
their properties, disable the security check feature. This reduces the runtime
requirements. If someone can come up with a nice way to keep those on, I would
be thankful. Next, change to Release build and build the solution. Then, we will
generate the first dll. Open a "x86 Native Tools Command Prompt" and cd into the
`Release\obj` directory. Run `link.exe /dll /out:breakpad_client_x86.dll
breakpad_c\*obj crash_generation_client\*.obj crash_generation_server\*.obj
exception_handler\*.obj common\*.obj`. It will generate a `dll`, a `exp`, and a
`lib`. Copy the `dll` and `lib` somewhere safe. Next, go back into Visual Studio
and change the target to `x64`. Repeat the same steps as above, but replace
`x86` with `x64` (and don't forget to use the x64 native tools!). Now, in the
safe place, back in mingw, run `dlltool -U -d breakpad_client.def -l
breakpad_client_x86_mingw.lib`, then edit the `def` file for `x64` and repeat.

And that's it! You should end up with the following files in your safe place:

```
breakpad_client.def
breakpad_client_x64.dll
breakpad_client_x64.lib
breakpad_client_x64_mingw.lib
breakpad_client_x86.dll
breakpad_client_x86_mingw.lib
```
