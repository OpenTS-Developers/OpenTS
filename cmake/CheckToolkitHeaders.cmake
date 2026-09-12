# Checks that no engine header outside code/ui/rml/ includes a UI toolkit or renderer header,
# and that no engine source outside code/ui/ includes RmlUi or Dear ImGui. docs/UI_DESIGN.md
# owns the rule; this script only enforces it.
#
# Expects OPENTS_SOURCE_DIR to be set. Run with `cmake -DOPENTS_SOURCE_DIR=<root> -P`.

if(NOT DEFINED OPENTS_SOURCE_DIR)
    message(FATAL_ERROR "CheckToolkitHeaders.cmake: OPENTS_SOURCE_DIR is not set.")
endif()

set(TOOLKIT_INCLUDE "^[ \t]*#[ \t]*include[ \t]*[<\"](RmlUi/|imgui|bgfx/|bx/|bimg/|stb_)")
set(UI_TOOLKIT_INCLUDE "^[ \t]*#[ \t]*include[ \t]*[<\"](RmlUi/|imgui)")

set(violations "")

file(GLOB_RECURSE headers RELATIVE "${OPENTS_SOURCE_DIR}"
    "${OPENTS_SOURCE_DIR}/code/*.h"
    "${OPENTS_SOURCE_DIR}/code/*.hh"
    "${OPENTS_SOURCE_DIR}/code/*.hpp"
)
foreach(header IN LISTS headers)
    if(header MATCHES "^code/ui/rml/")
        continue()
    endif()
    file(STRINGS "${OPENTS_SOURCE_DIR}/${header}" hits REGEX "${TOOLKIT_INCLUDE}")
    foreach(hit IN LISTS hits)
        list(APPEND violations "${header}: ${hit}")
    endforeach()
endforeach()

file(GLOB_RECURSE sources RELATIVE "${OPENTS_SOURCE_DIR}"
    "${OPENTS_SOURCE_DIR}/code/*.cpp"
    "${OPENTS_SOURCE_DIR}/code/*.c"
)
foreach(source IN LISTS sources)
    if(source MATCHES "^code/ui/")
        continue()
    endif()
    file(STRINGS "${OPENTS_SOURCE_DIR}/${source}" hits REGEX "${UI_TOOLKIT_INCLUDE}")
    foreach(hit IN LISTS hits)
        list(APPEND violations "${source}: ${hit}")
    endforeach()
endforeach()

if(violations)
    list(JOIN violations "\n  " text)
    message(FATAL_ERROR "Toolkit headers included outside their module:\n  ${text}")
endif()

message(STATUS "No toolkit header leaks outside code/ui/rml/.")
