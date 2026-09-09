# The detection sources are compiled straight into the harness. They live outside tests/ so
# that the recursive glob building OpenTS cannot pick this target's entry point up.

opents_add_test(CpuDetect
    NAME cpudetect
    SOURCES cpudetect.cpp
    ENGINE
        getcpu.cpp
        mpu.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    FLOAT
    UNPORTED
)
