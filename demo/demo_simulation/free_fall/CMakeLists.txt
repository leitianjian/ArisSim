set(SOURCES main.cpp
	test_events.hpp test_events.cpp
)
set(sire_demo "demo_simulation_free_fall")
set(CONFIG_EXISTS ON)
set(THREE_BALL_FREE_FALLING_EVENTS "WITH_PA" CACHE STRING "")
set_property(CACHE THREE_BALL_FREE_FALLING_EVENTS PROPERTY STRINGS "GLOBAL_STEP", "WITHOUT_PA", "WITH_PA")
if("${THREE_BALL_FREE_FALLING_EVENTS}" STREQUAL "GLOBAL_STEP")
	add_definitions(-DSIRE_THREE_BALL_FREE_FALLING_USING_GLOBAL_STEP)
elseif("${THREE_BALL_FREE_FALLING_EVENTS}" STREQUAL "WITHOUT_PA")
	add_definitions(-DSIRE_THREE_BALL_FREE_FALLING_WITHOUT_PA)
elseif("${THREE_BALL_FREE_FALLING_EVENTS}" STREQUAL "WITH_PA")
endif()
add_executable(${sire_demo} ${SOURCES})
target_compile_features(${sire_demo} PUBLIC cxx_std_17)
target_include_directories(${sire_demo} 
	PRIVATE 
		${aris_INCLUDE_DIRS} 
		${PROJECT_BINARY_DIR} 
		${hpp-fcl_INCLUDE_DIRS}
		${ASSIMP_INCLUDE_DIRS}
	)
target_link_libraries(${sire_demo}
	PRIVATE
		sire_core 
		sire_server 
		sire_middleware 
		sire_transfer 
		sire_integrator
		sire_simulator
		sire_sensor
		sire_physics 
		sire_cam_backend 
		${PUB_RELY_LINK_LIB}
	)
# DEBUG INSTALL 
install(
	TARGETS 
		sire_lib 
		${sire_demo} 
	CONFIGURATIONS
		Debug
	DESTINATION 
		demo/debug/${sire_demo}
	)
install(
FILES 
	"${aris_DIR}/../../../lib/${CMAKE_BUILD_TYPE}/aris_lib.dll" 
	"${TARGET_HPP_FCL_PATH}/bin/hpp-fcl.dll"
	"${assimp_DIR}/../../debug/bin/assimp-vc${MSVC_TOOLSET_VERSION}-mtd.dll"
	"${assimp_DIR}/../../debug/bin/zlibd1.dll"
	"${assimp_DIR}/../../debug/bin/pugixml.dll"
CONFIGURATIONS
	Debug
DESTINATION 
	demo/debug/${sire_demo}
)
if(${CONFIG_EXISTS})
	install(DIRECTORY config CONFIGURATIONS Debug DESTINATION demo/debug/${sire_demo})
endif()

# RELEASE INSTALL
install(
	TARGETS 
		sire_lib 
		${sire_demo} 
	CONFIGURATIONS
		Release
	DESTINATION 
		demo/release/${sire_demo}
	)
install(
	FILES 
		"${aris_DIR}/../../../lib/${CMAKE_BUILD_TYPE}/aris_lib.dll" 
		"${TARGET_HPP_FCL_PATH}/bin/hpp-fcl.dll"
		"${assimp_DIR}/../../bin/assimp-vc${MSVC_TOOLSET_VERSION}-mt.dll"
		"${assimp_DIR}/../../bin/zlib1.dll"
		"${assimp_DIR}/../../bin/pugixml.dll"
	CONFIGURATIONS
		Release
	DESTINATION 
		demo/release/${sire_demo}
	)

# Copy config to install directory
if(${CONFIG_EXISTS})
	install(DIRECTORY config CONFIGURATIONS Release DESTINATION demo/release/${sire_demo})
endif()
# if(EXISTS ${PROJECT_SOURCE_DIR}/demo/${sire_demo}/resource)
# 	install(DIRECTORY resource DESTINATION demo/${sire_demo})
# 	install(DIRECTORY resource DESTINATION resource/${sire_demo})
# endif()