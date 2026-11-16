# Runs the Python reference reader's verify command on FILE and fails unless it exits with
# EXPECT. CTest alone can only require exit code 0 or any failure.
execute_process(
    COMMAND "${PYTHON}" "${SCRIPT}" verify "${FILE}"
    RESULT_VARIABLE result
)
if(NOT result STREQUAL EXPECT)
    message(FATAL_ERROR "cncreplay.py verify exited with ${result}, expected ${EXPECT}, for ${FILE}")
endif()
