# The generator is compiled with the recorder core it reports to. The harness supplies the
# engine's recording hook, which a generator that is not recording never calls.

opents_add_test(RandomTest
    NAME random
    SOURCES randomtest.cpp
    ENGINE
        random.cpp
        syncrec.cpp
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    DOCTEST
)
