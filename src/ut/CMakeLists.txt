
Banner()

#set(CMAKE_CXX_FLAGS " -pthread " )

get_filename_component(INCLUDES_PATH ../include/ REALPATH) 


Headers(
	krunchercore
)


LibraryPaths(
	krunchercore::global/tracer/
	krunchercore::global/
)


Libraries(
	krunchertrace
	krunchercore
)


add_executable(
	textslicer
		textslicer.cpp
)

target_include_directories(
    textslicer
	    PRIVATE
		${INCLUDE_PATHS}
		../include/
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
		$<INSTALL_INTERFACE:include>
)
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
		../include
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

