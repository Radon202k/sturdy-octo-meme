@echo off

set vk_inc=C:\VulkanSDK\1.2.189.2\Include
set vk_lib=C:\VulkanSDK\1.2.189.2\Lib
set flags=-FC -nologo -Z7 -WX -W4 -wd4100 -wd4189 -wd4090 -wd4101 -wd4201
set file=..\code\win32_test.c
set libs=user32.lib vulkan-1.lib

pushd ..\bin
cl %flags% %file% -I%vk_inc% -link -LIBPATH:%vk_lib% %libs%
popd