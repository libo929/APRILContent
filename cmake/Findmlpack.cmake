IF(NOT DEFINED MLPACK_DIR)
   MESSAGE(STATUS "It is mandorary to sepecify mlpack directory.")
ENDIF()

FIND_PATH(MLPACK_INCLUDE_DIRS core.hpp HINTS ${MLPACK_DIR}/include/*)
FIND_LIBRARY(MLPACK_LIBRARIES NAMES mlpack HINTS ${MLPACK_DIR}/*)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(mlpack DEFAULT_MSG MLPACK_LIBRARIES MLPACK_INCLUDE_DIRS)

IF(NOT MLPACK_FOUND)
  MESSAGE(FATAL_ERROR "mlpack not found")
ENDIF()
