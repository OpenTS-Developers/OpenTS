set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)

set(OPENTS_EXPERIMENTAL_CLANG_CL ON CACHE BOOL "" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
set(OPENTS_MSVC_ROOT "" CACHE PATH "Path to the MSVC and Windows SDK files")
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES OPENTS_MSVC_ROOT)

if(NOT OPENTS_MSVC_ROOT AND DEFINED ENV{OPENTS_MSVC_ROOT})
    set(OPENTS_MSVC_ROOT "$ENV{OPENTS_MSVC_ROOT}" CACHE PATH "" FORCE)
endif()

if(NOT OPENTS_MSVC_ROOT)
    message(FATAL_ERROR
        "Set OPENTS_MSVC_ROOT to the directory containing MSVC and the Windows SDK.")
endif()

set(_opents_msvc_version_file
    "${OPENTS_MSVC_ROOT}/VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt")
if(NOT EXISTS "${_opents_msvc_version_file}")
    message(FATAL_ERROR
        "Default MSVC toolset version not found: ${_opents_msvc_version_file}")
endif()

file(STRINGS "${_opents_msvc_version_file}" _opents_msvc_version LIMIT_COUNT 1)
string(STRIP "${_opents_msvc_version}" _opents_msvc_version)
if(NOT _opents_msvc_version MATCHES "^14\\.([0-9]+)\\.")
    message(FATAL_ERROR "Unsupported MSVC toolset version: ${_opents_msvc_version}")
endif()
set(_opents_msvc_compatibility_version "19.${CMAKE_MATCH_1}")

set(_opents_msvc_dir "${OPENTS_MSVC_ROOT}/VC/Tools/MSVC/${_opents_msvc_version}")
set(_opents_sdk_dir "${OPENTS_MSVC_ROOT}/Windows Kits/10")

file(GLOB _opents_sdk_candidates
    LIST_DIRECTORIES TRUE
    RELATIVE "${_opents_sdk_dir}/Include"
    "${_opents_sdk_dir}/Include/*")
list(SORT _opents_sdk_candidates COMPARE NATURAL ORDER DESCENDING)

unset(_opents_sdk_version)
foreach(_candidate IN LISTS _opents_sdk_candidates)
    if(_candidate MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+(\\.[0-9]+)?$"
       AND IS_DIRECTORY "${_opents_sdk_dir}/Include/${_candidate}/shared"
       AND IS_DIRECTORY "${_opents_sdk_dir}/Include/${_candidate}/ucrt"
       AND IS_DIRECTORY "${_opents_sdk_dir}/Include/${_candidate}/um"
       AND IS_DIRECTORY "${_opents_sdk_dir}/Include/${_candidate}/winrt"
       AND IS_DIRECTORY "${_opents_sdk_dir}/Lib/${_candidate}/ucrt/x86"
       AND IS_DIRECTORY "${_opents_sdk_dir}/Lib/${_candidate}/um/x86")
        set(_opents_sdk_version "${_candidate}")
        break()
    endif()
endforeach()

if(NOT _opents_sdk_version)
    message(FATAL_ERROR "No complete Windows SDK found under: ${_opents_sdk_dir}")
endif()

foreach(_required_path
        "${_opents_msvc_dir}/include"
        "${_opents_sdk_dir}/Include/${_opents_sdk_version}/um")
    if(NOT EXISTS "${_required_path}")
        message(FATAL_ERROR "Required MSVC component not found: ${_required_path}")
    endif()
endforeach()

find_program(_opents_clang_cl clang-cl REQUIRED)
find_program(_opents_lld_link lld-link REQUIRED)
find_program(_opents_llvm_lib llvm-lib REQUIRED)
find_program(_opents_llvm_mt llvm-mt REQUIRED)
find_program(_opents_llvm_rc llvm-rc REQUIRED)

set(CMAKE_C_COMPILER "${_opents_clang_cl}")
set(CMAKE_CXX_COMPILER "${_opents_clang_cl}")
set(CMAKE_C_COMPILER_TARGET i686-pc-windows-msvc)
set(CMAKE_CXX_COMPILER_TARGET i686-pc-windows-msvc)
set(CMAKE_C_FLAGS_INIT
    "/clang:-fms-compatibility-version=${_opents_msvc_compatibility_version}")
set(CMAKE_CXX_FLAGS_INIT
    "/clang:-fms-compatibility-version=${_opents_msvc_compatibility_version}")
set(CMAKE_LINKER "${_opents_lld_link}")
set(CMAKE_AR "${_opents_llvm_lib}")
set(CMAKE_RC_COMPILER "${_opents_llvm_rc}" CACHE FILEPATH "" FORCE)
set(CMAKE_MT "${_opents_llvm_mt}")

set(CMAKE_USER_MAKE_RULES_OVERRIDE
    "${CMAKE_CURRENT_LIST_DIR}/clang-cl-rc-rules.cmake")

set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES
    "${_opents_msvc_dir}/atlmfc/include"
    "${_opents_msvc_dir}/include"
    "${_opents_sdk_dir}/Include/${_opents_sdk_version}/shared"
    "${_opents_sdk_dir}/Include/${_opents_sdk_version}/ucrt"
    "${_opents_sdk_dir}/Include/${_opents_sdk_version}/um"
    "${_opents_sdk_dir}/Include/${_opents_sdk_version}/winrt")
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_STANDARD_INCLUDE_DIRECTORIES})

set(_opents_rc_flags "")
foreach(_include_dir IN LISTS CMAKE_C_STANDARD_INCLUDE_DIRECTORIES)
    string(APPEND _opents_rc_flags " /I\"${_include_dir}\"")
endforeach()
set(CMAKE_RC_FLAGS_INIT "${_opents_rc_flags}")

set(_opents_linker_paths
    "/libpath:\"${_opents_msvc_dir}/atlmfc/lib/x86\""
    "/libpath:\"${_opents_msvc_dir}/lib/x86\""
    "/libpath:\"${_opents_sdk_dir}/Lib/${_opents_sdk_version}/ucrt/x86\""
    "/libpath:\"${_opents_sdk_dir}/Lib/${_opents_sdk_version}/um/x86\"")
string(JOIN " " _opents_linker_flags ${_opents_linker_paths})
set(CMAKE_EXE_LINKER_FLAGS_INIT "${_opents_linker_flags}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${_opents_linker_flags}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${_opents_linker_flags}")
