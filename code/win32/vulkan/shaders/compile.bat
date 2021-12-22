@echo off
echo Compiling shaders...
pushd bin
C:\VulkanSDK\1.2.189.2\Bin\glslc.exe ../shader.vert -o vert.spv
C:\VulkanSDK\1.2.189.2\Bin\glslc.exe ../shader.frag -o frag.spv
popd
echo Done.
pause