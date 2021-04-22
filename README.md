# StackAbusePC
 ImHex plugin to talk to the switch for cool features

# Build
 - Put this repo in IMHex's plugins folder, and add the plugin "StackAbusePC" in the CMakeLists.txt in the base ImHex folder
 - build the ColdClear C api, and drop it into a folder called "lib" next to the source folder, as well as the header file provided by the C api
 - install the latest libusb for mingw/msys
 - follow ImHex's instructions for building :D

# Cool Features added
 - Live memory watching (pointer chains can be watched live too!) added in 1.0.0 aka first commit
 - grabbing the BuildID of the current process (album, and home menu count as a process, make sure to check that you arent grabbing that!) added in 1.0.0
 - grabbing the address the game is on (this normally changes every time you boot the game) added on 1.0.0

# Features to be added
 - Controller mode to be able to use your keyboard to play games on the switch
 - Live memory poking with iterators in order to poke a crap ton of memory in one go
 - Whatever you want, just dm me on whatever platform, just look up Shakkar23 on all platforms, and I should pop up :) (the ones with pfp's are the ones to dm)