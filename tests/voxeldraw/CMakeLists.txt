# The voxel library is compiled straight into the harness. It lives outside code/ so that the
# recursive glob building OpenTS cannot pick this target's entry point up.

opents_add_test(VoxelDraw
    NAME voxeldraw
    SOURCES voxeldraw.cpp
    ENGINE
        voxlib.cpp
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
    DEFINITIONS WIN32 _WINDOWS _MBCS
    FLOAT
    UNPORTED
)
