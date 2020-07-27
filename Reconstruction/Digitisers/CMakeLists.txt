gaudi_subdir(Digitisers v0r0)

find_package(CLHEP REQUIRED)
find_package(GEAR REQUIRED)
find_package(GSL REQUIRED ) 
find_package(LCIO REQUIRED ) 
find_package(podio REQUIRED ) 
find_package(K4FWCore REQUIRED)
find_package(EDM4HEP REQUIRED)
find_package(MarlinUtil REQUIRED ) 

gaudi_depends_on_subdirs(
    Service/GearSvc
    Service/EventSeeder
)

set(Digitisers_srcs
  src/TPCDigiAlg.cpp
  src/voxel.cpp
)

# Modules
gaudi_add_module(Digitisers ${Digitisers_srcs}
    INCLUDE_DIRS K4FWCore GaudiKernel GaudiAlgLib CLHEP gear ${GSL_INCLUDE_DIRS} ${MarlinUtil_INCLUDE_DIRS} ${LCIO_INCLUDE_DIRS}
    LINK_LIBRARIES K4FWCore GaudiKernel GaudiAlgLib CLHEP $ENV{GEAR}/lib/libgearsurf.so ${GSL_LIBRARIES}  ${MarlinUtil_LIBRARIES} ${LCIO_LIBRARIES} 
    -Wl,--no-as-needed EDM4HEP::edm4hep EDM4HEP::edm4hepDict -Wl,--as-needed
)
