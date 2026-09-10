# The queue is a template with no source of its own, so the harness compiles nothing from
# code/. It lives outside code/ so that the recursive glob building OpenTS cannot pick this
# target's entry point up.
#
# FLOAT is not decoration here: the order two nodes of equal score come out in turns on
# exact float comparison, so the harness has to carry the float semantics the engine builds
# with to measure what the engine measures.

opents_add_test(PriorityQueue
    NAME priorityqueue
    SOURCES priorityqueuetest.cpp
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
    DEFINITIONS WIN32 _WINDOWS _MBCS NOMINMAX
    FLOAT
)
