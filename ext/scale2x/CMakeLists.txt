# scale2x

include(Find-Restrict-Keyword)

add_library(scale2x STATIC
	scale2x.cpp
	scale2x.h
	scale3x.cpp
	scale3x.h
	scalebit.cpp
	scalebit.h
)
target_compile_definitions(scale2x PRIVATE
	USE_SCALE_RANDOMWRITE
	restrict=${RESTRICT_KEYWORD})
target_include_directories(scale2x PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(OpenJazz scale2x)
