# Third-party notices

OpenTS binaries include software from the following projects. Each project
remains under its own license and copyright notices.

| Project                                                            | Use                                       | License      |
| ------------------------------------------------------------------ | ----------------------------------------- | ------------ |
| [bgfx](https://github.com/bkaradzic/bgfx)                          | Rendering                                 | BSD 2-Clause |
| [bx](https://github.com/bkaradzic/bx)                              | Foundation library used by bgfx           | BSD 2-Clause |
| [bimg](https://github.com/bkaradzic/bimg)                          | Image and texture processing used by bgfx | BSD 2-Clause |
| [DirectX-Headers](https://github.com/microsoft/DirectX-Headers)    | Direct3D API headers used by bgfx         | MIT          |
| [tinystl](https://github.com/mendsley/tinystl)                     | Containers used internally by bgfx        | BSD 2-Clause |
| [astc-encoder](https://github.com/ARM-software/astc-encoder)       | ASTC texture processing used by bimg      | Apache-2.0   |
| [OpenGL Registry](https://github.com/KhronosGroup/OpenGL-Registry) | OpenGL API headers used by bgfx           | MIT          |
| [Vulkan Headers](https://github.com/KhronosGroup/Vulkan-Headers)   | Vulkan API headers used by bgfx           | Apache-2.0   |

The source checkout keeps the license texts under `thirdparty/`. Binary
packages reproduce the license texts for the components used by OpenTS under
`OpenTS_THIRD_PARTY_LICENSES/`.
