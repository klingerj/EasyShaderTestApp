### Script documentation

#### Project scripts
Scripts for project setup

<b>proj_shell.bat</b> - opens a cmd window where all scripts should be run within. Recommend double click to run.  

<b>proj_debugger.bat</b> - opens a visual studio solution from the specified exe  
<code>> proj_debugger.bat ..\Build\TinkerApp.exe </code>  
or  
<code>> proj_debugger.bat ..\ToolsBin\TinkerSC.exe </code>  

#### Build scripts
Scripts that compile code

<b>build_app_and_game_dll.bat</b> - builds platform app exe and game dll into <code>Build/</code>  
<code>> build_app_and_game_dll.bat [Release | Debug] [VK | DX] </code>  

<b>build_benchmarks.bat</b> - builds benchmark exe into <code>Build/</code>. Note game dll hotloads!  
<code>> build_benchmarks.bat [Release | Debug] </code>  

<b>build_app.bat</b> - builds platform app exe into <code>Build/</code>  
<code>> build_app.bat [Release | Debug] </code>  

<b>build_game_dll.bat</b> - builds game dll into <code>Build/</code>. Note game dll hotloads!  
<code>> build_game_dll.bat [Release | Debug] [VK | DX] </code>  

<b>build_server.bat</b> - builds server exe into <code>Build/</code>  
<code>> build_server.bat [Release | Debug] </code>  

<b>build_shadercompiler.bat</b> - builds shader compiler exe into <code>ToolsBin/</code>  
<code>> build_shadercompiler.bat [Release | Debug] [VK | DX] </code>  

<b>build_spirv-vm.bat</b> - (.sh also exists) builds unit test exe into <code>Build/</code>  
<code>> build_spirv-vm.bat [Release | Debug] </code>  

<b>build_tests.bat</b> - builds unit test exe into <code>Build/</code>  
<code>> build_tests.bat [Release | Debug] </code>  

<b>ez-build_release.bat</b> - runs multiple release builds by simply calling the aforementioned scripts:  
* <b>build_app.bat Release VK </b>
* <b>build_shadercompiler.bat Release VK</b>
* <b>build_server.bat Release VK</b>
* <b>build_benchmarks.bat Release VK</b>
* <b>build_tests.bat Release VK</b>

<code>> ez-build_release.bat [Release | Debug] </code>  

#### Run scripts
Scripts that run built exe's

<b>run_benchmarks.bat</b> - runs the built benchmark exe  
<code>> run_benchmarks.bat </code>  

<b>run_game.bat</b> - runs the built Tinker engine exe  
<code>> run_game.bat </code>  

<b>run_spirv-vm-demo.bat</b> - runs the build spirv virtual machine exe  
<code>> run_spirv-vm-demo.bat </code>  

<b>run_tests.bat</b> - runs the built unit test exe  
<code>> run_tests.bat </code>  

#### Other scripts
<b>timecmd.bat</b> - runs your batch script and outputs the time it took to run. So far I used this for coarse build performance timing.  
<code>> timecmd.bat <your .bat> <args for your .bat> </code>  
