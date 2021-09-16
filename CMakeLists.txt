# The cmake version - preferably = `cmake --version`
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# Check if it's a Windows
IF(WIN32)
    MESSAGE(STATUS "It's a Windows... Good luck fellas")
    # Search the compilers installed by MSYS2
    FIND_PROGRAM(CMAKE_C_COMPILER "gcc")
    FIND_PROGRAM(CMAKE_CXX_COMPILER "g++")
ENDIF()

# The name of our program
PROJECT(BIM2SE VERSION 0.1)

# Set some variables for later use or
# include inside the binaries
SET(AUTHOR "Gilles Trenson")

# Add an optional "sandbox"
OPTION(USE_SANDBOX "Enable sandbox / playground mode" ON)

# Configure a header that will be used
# to insert the version number indide our
# program and possibly other vars as well
CONFIGURE_FILE(BIM2SE.h.in BIM2SE.h)

# Specify search path for modules, semicolon-seperated list
# SET(OpenCASCADE_DIR "/tmp/occt/")

# specify the C++ standard
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED True)

# Check if the User defined the OCCT or OCE directory
# OCE = Open Cascade Community Edition; OCCT is the original
# Inspiration https://github.com/FreeCAD/FreeCAD/blob/5d49bf78de785a536f941f1a6d06d432582a95d3/cMake/FindOpenCasCade.cmake
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # Detect Homebrew - find the Homebrew Path
    # Stolen from FreeCAD - SetupPython.cmake
    FIND_PROGRAM(HOMEBREW_EXECUTABLE brew)
    IF(EXISTS ${HOMEBREW_EXECUTABLE})
        STRING(REPLACE "/bin/brew" ""
            HOMEBREW_PREFIX ${HOMEBREW_EXECUTABLE})
        MESSAGE(STATUS "Homebrew detected @${HOMEBREW_PREFIX}")
    ENDIF()
# Might be Linux?
ELSEIF(UNIX)
    SET(OpenCASCADE_DIR "/usr/local/share/cmake/")
# Might be Windows?
ELSEIF(WIN32)
    # Update this if necessary / C:/Users/BEGILT/AppData/Local/MSYS2/mingw64
    SET(OpenCASCADE_DIR "C:/Users/BEGILT/AppData/Local/MSYS2/mingw64")
ENDIF()


# Find packages, see available find
# modules using `cmake --help-module-list`
# and `cmake --help-module <MODULE>` to see
# which variables will be exposed

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # Check if Homebrew has been found
    IF(DEFINED HOMEBREW_PREFIX)
        FIND_PACKAGE(OpenCASCADE REQUIRED HINTS ${HOMEBREW_PREFIX}/Cellar/opencascade/*)
        message(STATUS "OpenCASCADE (OCCT) has been found.")
        # Have a look at OpenCASCADEConfig.cmake what has been exposed
        INCLUDE_DIRECTORIES(${OpenCASCADE_INCLUDE_DIR})
    ENDIF()
ELSE()
    # OpenCascade_DIR is defined
    FIND_PACKAGE(OCE QUIET)
    IF(NOT OCE_FOUND)
        # First try... Maybe another UNIX path
        FIND_PACKAGE(OCE QUIET HINTS ${OpenCASCADE_DIR})
    ENDIF()
    IF(NOT OCE_FOUND)
        # Second try... 
        FIND_PACKAGE(OpenCASCADE REQUIRED HINTS ${OpenCASCADE_DIR}) 
    ENDIF()
    
    # Have a look at OpenCASCADEConfig.cmake what has been exposed
    IF(OCE_FOUND)
        # Check if OCE has been found
        message(STATUS "OpenCASCADE Community Edition (OCE) has been found.")
        INCLUDE_DIRECTORIES(${OCE_INCLUDE_DIR})
    ENDIF()
    IF(OpenCASCADE_FOUND)
        # Check if OCCT has been found
        message(STATUS "OpenCASCADE Technology (OCCT) has been found.")
        INCLUDE_DIRECTORIES(${OpenCASCADE_INCLUDE_DIR})
    ENDIF()
ENDIF()


# If option USE_SANDBOX is set
IF(USE_SANDBOX)
    # Add a library in a subfolder
    ADD_SUBDIRECTORY(sandbox)
    # Append the name of the library to a list
    list(APPEND EXTRA_LIBS sandbox)
    # Append the location of the header files of the library
    list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/sandbox")
ENDIF()

# Add out executable - using variables to 
# refer to our ${PROJECT_NAME}
ADD_EXECUTABLE(${PROJECT_NAME} BIM2SE.cpp) 

# Add the location src files
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC
    ${EXTRA_LIBS}
    TKernel
    TKG2d
    TKG3d
    TKGeomBase
    TKMath
    TKPrim
    TKBO
    TKSTEP
    TKSTL
    TKMesh
    TKShHealing
)

# The configure file will be read in the 
# binary tree, that's why we need to add the
# directory for Linker - should only be 
# available to the current target!
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC
    ${PROJECT_BINARY_DIR}
    ${EXTRA_INCLUDES}
)