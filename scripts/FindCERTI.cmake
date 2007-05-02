#
# - Try to find CERTI installation
#
# See https://savannah.nongnu.org/projects/certi
#  or http://www.cert.fr/CERTI/
#
# You may help this script to find the appropriate
# CERTI installation by setting CERTI_HOME
# before calling FIND_PACKAGE(CERTI REQUIRED)
#
# Once done this will define:
#
# CERTI_FOUND           - true if all necessary components are found
# CERTI_RTIG_EXECUTABLE - the RTIG program 
# CERTI_RTIA_EXECUTABLE - the RTIA program 
# CERTI_LIBRARIES       - The libraries to link against to use CERTI
# CERTI_INCLUDE         - the 
# CERTI_BINARY_DIRS     - the directory(ies) where the rtig, rtia programs resides
# CERTI_LIBRARY_DIRS    - the directory(ies) where libraries reside
# CERTI_INCLUDE_DIRS    - the directory(ies) where ONC RPC includes file are found

MESSAGE(STATUS "Looking for CERTI library and programs...")

IF (NOT CERTI_HOME)
  SET(CERTI_HOME "/usr/local")
ENDIF (NOT CERTI_HOME)

SET(PATH_DIRS "${CERTI_HOME}/bin;/usr/local/bin")
SET(PATH_LIBS "${CERTI_HOME}/lib;/usr/local/lib")
SET(PATH_INCLUDE "${CERTI_HOME}/include;/usr/local/include")

FIND_PROGRAM(CERTI_RTIG_EXECUTABLE 
  NAMES rtig 
  PATHS ${PATH_DIRS} 
  PATH_SUFFIXES bin
  DOC "The RTI g???")
IF (CERTI_RTIG_EXECUTABLE) 
  MESSAGE(STATUS "Looking for CERTI... - found rtig is ${CERTI_RTIG_EXECUTABLE}")
  SET(CERTI_RPCGEN_FOUND "YES")
  GET_FILENAME_COMPONENT(CERTI_BINARY_DIRS ${CERTI_RTIG_EXECUTABLE} PATH)
ELSE (CERTI_RTIG_EXECUTABLE) 
  SET(CERTI_RRIG_FOUND "NO")
  MESSAGE(STATUS "Looking for CERTI... - rtig NOT FOUND")
ENDIF (CERTI_RTIG_EXECUTABLE) 

FIND_PROGRAM(CERTI_RTIA_EXECUTABLE 
  NAMES rtia
  PATHS ${PATH_DIRS} 
  PATH_SUFFIXES bin
  DOC "The RTI Ambassador")
IF (CERTI_RTIA_EXECUTABLE) 
  MESSAGE(STATUS "Looking for CERTI... - found rtia is ${CERTI_RTIA_EXECUTABLE}")
  SET(CERTI_RTIA_FOUND "YES")
  GET_FILENAME_COMPONENT(CERTI_BINARY_DIRS ${CERTI_RTIA_EXECUTABLE} PATH)
ELSE (CERTI_RTIA_EXECUTABLE) 
  SET(CERTI_RTIA_FOUND "NO")
  MESSAGE(STATUS "Looking for CERTI... - rtia NOT FOUND")
ENDIF (CERTI_RTIA_EXECUTABLE) 

FIND_LIBRARY(CERTI_CERTI_LIBRARY
	NAMES CERTI
	PATHS ${PATH_LIBS}	
	PATH_SUFFIXES lib
	DOC "The CERTI Library")

IF (CERTI_CERTI_LIBRARY) 
  GET_FILENAME_COMPONENT(CERTI_LIBRARY_DIRS ${CERTI_CERTI_LIBRARY} PATH) 
ENDIF (CERTI_CERTI_LIBRARY)

FIND_LIBRARY(CERTI_RTI_LIBRARY
	NAMES RTI
	PATHS ${PATH_LIBS}	
	PATH_SUFFIXES lib
	DOC "The RTI Library")

IF (CERTI_RTI_LIBRARY) 
  GET_FILENAME_COMPONENT(TEMP ${CERTI_CERTI_LIBRARY} PATH) 
  IF (NOT "${TEMP}" STREQUAL "${CERTI_LIBRARY_DIRS}")
    SET(CERTI_LIBRARY_DIRS "${CERTI_LIBRARY_DIRS};${TEMP}")
  ENDIF(NOT "${TEMP}" STREQUAL "${CERTI_LIBRARY_DIRS}")
ENDIF (CERTI_RTI_LIBRARY)

FIND_FILE(CERTI_INCLUDE
  NAMES RTI.hh
  PATHS ${PATH_INCLUDE}
  PATH_SUFFIXES include
  DOC "The CERTI Include Files")

IF (CERTI_INCLUDE) 
  GET_FILENAME_COMPONENT(CERTI_INCLUDE_DIRS ${CERTI_INCLUDE} PATH) 
ENDIF (CERTI_INCLUDE)

IF (CERTI_RTI_LIBRARY     AND 
    CERTI_RTIA_EXECUTABLE AND
    CERTI_RTIG_EXECUTABLE AND
    CERTI_INCLUDE)
  
  SET(CERTI_FOUND TRUE)
  MESSAGE(STATUS "Looking for CERTI... - All components FOUND") 
ELSE (CERTI_RTI_LIBRARY     AND 
    CERTI_RTIA_EXECUTABLE AND
    CERTI_RTIG_EXECUTABLE AND
    CERTI_INCLUDE)
  SET(CERTI_FOUND FALSE)
  MESSAGE(STATUS "Looking for CERTI... - NOT all CERTI components were FOUND")   
  IF (CERTI_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Cannot find complete CERTI installation, please try to set CERTI_HOME and rerun")
  ENDIF (CERTI_FIND_REQUIRED)
ENDIF(CERTI_RTI_LIBRARY     AND 
  CERTI_RTIA_EXECUTABLE AND
  CERTI_RTIG_EXECUTABLE AND
  CERTI_INCLUDE)


MARK_AS_ADVANCED(
  CERTI_INCLUDE
  CERTI_LIBRARY
  CERTI_BINARY_DIRS
  CERTI_LIBRARY_DIRS
  CERTI_INCLUDE_DIRS
  )
