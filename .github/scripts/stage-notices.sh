#!/bin/bash
# Copy the license notices that ship beside a release client into the artifact directory.
set -euo pipefail

dest="${1:?artifact directory}"
mkdir -p "${dest}/OpenTS_THIRD_PARTY_LICENSES"
cp LICENSE.md THIRD_PARTY_NOTICES.md "${dest}/"
cp thirdparty/bgfx.cmake/bgfx/LICENSE "${dest}/OpenTS_THIRD_PARTY_LICENSES/bgfx.txt"
cp thirdparty/bgfx.cmake/bx/LICENSE "${dest}/OpenTS_THIRD_PARTY_LICENSES/bx.txt"
cp thirdparty/bgfx.cmake/bimg/LICENSE "${dest}/OpenTS_THIRD_PARTY_LICENSES/bimg.txt"
cp thirdparty/bgfx.cmake/bgfx/3rdparty/directx-headers/LICENSE "${dest}/OpenTS_THIRD_PARTY_LICENSES/directx-headers.txt"
cp thirdparty/bgfx.cmake/bx/include/tinystl/LICENSE "${dest}/OpenTS_THIRD_PARTY_LICENSES/tinystl.txt"
cp thirdparty/bgfx.cmake/bimg/3rdparty/astc-encoder/LICENSE.txt "${dest}/OpenTS_THIRD_PARTY_LICENSES/astc-encoder.txt"
cp thirdparty/licenses/khronos-opengl.txt "${dest}/OpenTS_THIRD_PARTY_LICENSES/khronos-opengl.txt"
cp thirdparty/miniaudio/LICENSE "${dest}/OpenTS_THIRD_PARTY_LICENSES/miniaudio.txt"
cp thirdparty/licenses/stb-vorbis.txt "${dest}/OpenTS_THIRD_PARTY_LICENSES/stb-vorbis.txt"
cp thirdparty/licenses/lzo.txt "${dest}/OpenTS_THIRD_PARTY_LICENSES/lzo.txt"
cat thirdparty/licenses/khronos-vulkan-notice.txt \
    thirdparty/bgfx.cmake/bimg/3rdparty/astc-encoder/LICENSE.txt \
    > "${dest}/OpenTS_THIRD_PARTY_LICENSES/khronos-vulkan.txt"
cp thirdparty/RmlUi/LICENSE.txt "${dest}/OpenTS_THIRD_PARTY_LICENSES/rmlui.txt"
cp thirdparty/RmlUi/Include/RmlUi/Core/Containers/LICENSE.txt "${dest}/OpenTS_THIRD_PARTY_LICENSES/rmlui-containers.txt"
cat thirdparty/freetype/LICENSE.TXT thirdparty/freetype/docs/FTL.TXT \
    > "${dest}/OpenTS_THIRD_PARTY_LICENSES/freetype.txt"
cp thirdparty/licenses/freetype-zlib.txt "${dest}/OpenTS_THIRD_PARTY_LICENSES/zlib.txt"
cp thirdparty/imgui/LICENSE.txt "${dest}/OpenTS_THIRD_PARTY_LICENSES/dear-imgui.txt"
cp thirdparty/licenses/stb-imgui.txt "${dest}/OpenTS_THIRD_PARTY_LICENSES/stb-imgui.txt"
cp thirdparty/licenses/stb-image.txt "${dest}/OpenTS_THIRD_PARTY_LICENSES/stb-image.txt"
cp thirdparty/SDL/LICENSE.txt "${dest}/OpenTS_THIRD_PARTY_LICENSES/sdl.txt"
cp thirdparty/SDL/src/video/yuv2rgb/LICENSE "${dest}/OpenTS_THIRD_PARTY_LICENSES/yuv2rgb.txt"
cp thirdparty/licenses/sdl-fdlibm.txt "${dest}/OpenTS_THIRD_PARTY_LICENSES/fdlibm.txt"
cp ui/OFL.txt "${dest}/OpenTS_THIRD_PARTY_LICENSES/arimo.txt"
