# The catalog and checksum are compiled straight into the harness. The download and the
# screen stay in the game.

file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}/workers/content/manifest.json" CONTENT_MANIFEST_PATH)

opents_add_test(ContentInstall
    NAME contentinstall
    SOURCES contentinstall.cpp
    ENGINE
        contentcatalog.cpp
        contentsha256.cpp
    DEFINITIONS "CONTENT_MANIFEST=\"${CONTENT_MANIFEST_PATH}\"" WIN32 _WINDOWS NOMINMAX
    DOCTEST
)
