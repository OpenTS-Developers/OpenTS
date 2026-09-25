# Checks that no engine header outside code/ui/rml/ includes a UI toolkit or renderer header,
# that no engine source outside code/ui/ includes RmlUi or Dear ImGui, and that SDL stays in
# code/sdl/: its sources and private headers may include SDL, sdlwindow.h may not, and nothing
# outside code/sdl/ includes SDL or a code/sdl/ header other than sdlwindow.h.
#
# Expects OPENTS_SOURCE_DIR to be set. Run with `cmake -DOPENTS_SOURCE_DIR=<root> -P`.

if(NOT DEFINED OPENTS_SOURCE_DIR)
    message(FATAL_ERROR "CheckToolkitHeaders.cmake: OPENTS_SOURCE_DIR is not set.")
endif()

set(TOOLKIT_INCLUDE "^[ \t]*#[ \t]*include[ \t]*[<\"](RmlUi/|imgui|bgfx/|bx/|bimg/|stb_|SDL3/)")
set(UI_TOOLKIT_INCLUDE "^[ \t]*#[ \t]*include[ \t]*[<\"](RmlUi/|imgui)")
set(SDL_INCLUDE "^[ \t]*#[ \t]*include[ \t]*[<\"]SDL3/")
set(RENDER_TOOLKIT_INCLUDE "^[ \t]*#[ \t]*include[ \t]*[<\"](RmlUi/|imgui|bgfx/|bx/|bimg/|stb_)")
set(SDL_LAYER_INCLUDE "^[ \t]*#[ \t]*include[ \t]*\"sdl/")
set(SDL_LAYER_PUBLIC "\"sdl/sdlwindow\\.h\"")

# Includes a code/sdl/ header other than sdlwindow.h, which keeps SDL types out of the file.
function(check_sdl_layer_includes file)
    file(STRINGS "${OPENTS_SOURCE_DIR}/${file}" hits REGEX "${SDL_LAYER_INCLUDE}")
    foreach(hit IN LISTS hits)
        if(NOT hit MATCHES "${SDL_LAYER_PUBLIC}")
            list(APPEND violations "${file}: ${hit}")
        endif()
    endforeach()
    set(violations "${violations}" PARENT_SCOPE)
endfunction()

set(violations "")

file(GLOB_RECURSE headers RELATIVE "${OPENTS_SOURCE_DIR}"
    "${OPENTS_SOURCE_DIR}/code/*.h"
    "${OPENTS_SOURCE_DIR}/code/*.hh"
    "${OPENTS_SOURCE_DIR}/code/*.hpp"
)
foreach(header IN LISTS headers)
    if(header MATCHES "^code/ui/rml/")
        set(pattern "${SDL_INCLUDE}")
    elseif(header MATCHES "^code/sdl/" AND NOT header STREQUAL "code/sdl/sdlwindow.h")
        set(pattern "${RENDER_TOOLKIT_INCLUDE}")
    else()
        set(pattern "${TOOLKIT_INCLUDE}")
    endif()
    if(NOT header MATCHES "^code/sdl/")
        check_sdl_layer_includes("${header}")
    endif()
    file(STRINGS "${OPENTS_SOURCE_DIR}/${header}" hits REGEX "${pattern}")
    foreach(hit IN LISTS hits)
        list(APPEND violations "${header}: ${hit}")
    endforeach()
endforeach()

file(GLOB_RECURSE sources RELATIVE "${OPENTS_SOURCE_DIR}"
    "${OPENTS_SOURCE_DIR}/code/*.cpp"
    "${OPENTS_SOURCE_DIR}/code/*.c"
)
foreach(source IN LISTS sources)
    if(NOT source MATCHES "^code/ui/")
        file(STRINGS "${OPENTS_SOURCE_DIR}/${source}" hits REGEX "${UI_TOOLKIT_INCLUDE}")
        foreach(hit IN LISTS hits)
            list(APPEND violations "${source}: ${hit}")
        endforeach()
    endif()
    if(NOT source MATCHES "^code/sdl/")
        file(STRINGS "${OPENTS_SOURCE_DIR}/${source}" hits REGEX "${SDL_INCLUDE}")
        foreach(hit IN LISTS hits)
            list(APPEND violations "${source}: ${hit}")
        endforeach()
        check_sdl_layer_includes("${source}")
    endif()
endforeach()

if(violations)
    list(JOIN violations "\n  " text)
    message(FATAL_ERROR "Toolkit headers included outside their module:\n  ${text}")
endif()

message(STATUS "No toolkit header leaks outside its module.")
