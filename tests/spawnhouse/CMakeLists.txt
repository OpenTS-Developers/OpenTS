# The spawn house helpers are compiled straight into the harness. They live in code/ but have no
# engine dependencies, so the harness compiles them alone. The entry point stays here so the
# recursive glob building OpenTS cannot pick it up.

opents_add_test(SpawnHouse
    NAME spawnhouse
    SOURCES spawnhousetest.cpp
    ENGINE
        spawnhouse.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
)
