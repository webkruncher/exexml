

set(CMAKE_CXX_FLAGS "  -pthread " )

get_filename_component(INCLUDES_PATH ../include/ REALPATH) 

add_executable(
	exeut
		testunit.cpp
)


target_include_directories(
    exeut
	    PRIVATE
		${INCLUDE_PATHS}
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
		$<INSTALL_INTERFACE:include>
)

target_link_directories(
	exeut
		PUBLIC
			${LIBRARY_PATHS}
)

target_link_libraries(
    exeut
	    PUBLIC
			${LIBRARIES}
)

install(
    TARGETS
	    exeut
		    EXPORT exports
		    RUNTIME DESTINATION bin
) 

