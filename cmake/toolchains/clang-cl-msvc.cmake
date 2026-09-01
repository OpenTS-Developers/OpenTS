set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)

set(OPENTS_EXPERIMENTAL_CLANG_CL ON CACHE BOOL "" FORCE)
set(OPENTS_MSVC_ROOT "" CACHE PATH "Path to the MSVC and Windows SDK files")
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES OPENTS_MSVC_ROOT)

if(NOT OPENTS_MSVC_ROOT AND DEFINED ENV{OPENTS_MSVC_ROOT})
    set(OPENTS_MSVC_ROOT "$ENV{OPENTS_MSVC_ROOT}" CACHE PATH "" FORCE)
endif()

if(NOT OPENTS_MSVC_ROOT)
    message(FATAL_ERROR
        "Set OPENTS_MSVC_ROOT to the directory containing MSVC and the Windows SDK.")
endif()

set(_opents_msvc_version "14.44.35207")
set(_opents_sdk_version "10.0.26100.0")
set(_opents_msvc_dir "${OPENTS_MSVC_ROOT}/VC/Tools/MSVC/${_opents_msvc_version}")
set(_opents_sdk_dir "${OPENTS_MSVC_ROOT}/Windows Kits/10")

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
find_program(_opents_uasm uasm REQUIRED)

set(CMAKE_C_COMPILER "${_opents_clang_cl}")
set(CMAKE_CXX_COMPILER "${_opents_clang_cl}")
set(CMAKE_C_COMPILER_TARGET i686-pc-windows-msvc)
set(CMAKE_CXX_COMPILER_TARGET i686-pc-windows-msvc)
set(CMAKE_C_FLAGS_INIT "/clang:-fms-compatibility-version=19.44")
set(CMAKE_CXX_FLAGS_INIT "/clang:-fms-compatibility-version=19.44")
set(CMAKE_LINKER "${_opents_lld_link}")
set(CMAKE_AR "${_opents_llvm_lib}")
set(CMAKE_RC_COMPILER "${_opents_llvm_rc}" CACHE FILEPATH "" FORCE)
set(CMAKE_MT "${_opents_llvm_mt}")
set(CMAKE_ASM_MASM_COMPILER "${_opents_uasm}" CACHE FILEPATH "" FORCE)

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
