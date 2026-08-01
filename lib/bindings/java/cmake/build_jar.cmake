# Compiles the SWIG-generated Java proxies into a jar.
#
# Run with cmake -P at *build* time (not configure time): the .java files are
# produced by SWIG during the build, so they cannot be globbed any earlier.
#
# Expects: JAVA_SRC_DIR, CLASSES_DIR, JAR_FILE, JAVAC, JAR.

file(GLOB_RECURSE java_sources "${JAVA_SRC_DIR}/*.java")
if(NOT java_sources)
    message(FATAL_ERROR "No .java files under ${JAVA_SRC_DIR}; did SWIG run?")
endif()

file(REMOVE_RECURSE "${CLASSES_DIR}")
file(MAKE_DIRECTORY "${CLASSES_DIR}")

execute_process(
    COMMAND ${JAVAC} -d "${CLASSES_DIR}" ${java_sources}
    RESULT_VARIABLE javac_result)
if(NOT javac_result EQUAL 0)
    message(FATAL_ERROR "javac failed (${javac_result})")
endif()

execute_process(
    COMMAND ${JAR} cf "${JAR_FILE}" -C "${CLASSES_DIR}" .
    RESULT_VARIABLE jar_result)
if(NOT jar_result EQUAL 0)
    message(FATAL_ERROR "jar failed (${jar_result})")
endif()
