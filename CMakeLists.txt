cmake_minimum_required (VERSION 3.20)

project ("BIMConvertToGeo")

include_directories( ${CMAKE_SOURCE_DIR}/include/ ) # include json.hpp

find_package(CGAL) # for CGAL
if (CGAL_FOUND)
	include(${CGAL_USE_FILE})
	message(STATUS "CGAL found")
else()
	message(SEND_ERROR "this code requires the CGAL library")
	return()
endif()

add_definitions(
  -DDATA_PATH=\"${PROJECT_SOURCE_DIR}/data\"
  -DINPUT_PATH=\"${PROJECT_SOURCE_DIR}/data/inputData\"
  -DOUTPUT_PATH=\"${PROJECT_SOURCE_DIR}/data/outputData\"
  -DINTER_PATH=\"${PROJECT_SOURCE_DIR}/data/intermediateData\"
)

add_executable (BIMConvertToGeo "src/main.cpp"  "src/LoadOBJ.hpp" "src/Polyhedra.hpp" )
