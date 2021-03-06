###############################################################################
# Package: DetEcalMatrix 
# Based on package: lcgeo
################################################################################


find_package(DD4hep COMPONENTS DDRec DDG4 DDParsers REQUIRED)
find_package(Geant4)
include(${Geant4_USE_FILE})

set(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH}  ${DD4hep_ROOT}/cmake )
include( DD4hep )

find_package(ROOT COMPONENTS MathCore GenVector Geom REQUIRED)

# TODO: how to handle 
# install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/compact DESTINATION Detector/DetEcalMatrix)
gaudi_add_module(DetEcalMatrix
                 SOURCES src/calorimeter/EcalMatrix.cpp
		 LINK ${DD4hep_COMPONENT_LIBRARIES} 
                      # ROOT Geant4
)

set(LIBRARY_OUTPUT_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
message(STATUS "LIBRARY_OUTPUT_PATH -> ${LIBRARY_OUTPUT_PATH}")
dd4hep_generate_rootmap(DetEcalMatrix)

install(TARGETS DetEcalMatrix
  EXPORT CEPCSWTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
