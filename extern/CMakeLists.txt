cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
include(FetchContent)

set(FETCHCONTENT_QUIET ON)

set(target_name Extern)

# ------------------------------------------------------------------------------
# A multi-platform library for OpenGL, OpenGL ES, Vulkan, window and
# input http://www.glfw.org/
message("-- Fetching External Project: glfw")
FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
)
FetchContent_MakeAvailable(glfw)

# GLM
message("-- Fetching External Project: glm")
FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
)
FetchContent_MakeAvailable(glm)

# Simply to have some color in the console, and this library is windows only
if(WIN32)
	message("-- Fetching External Project: color-console")

	FetchContent_Declare(color-console
	  GIT_REPOSITORY https://github.com/imfl/color-console
	  CONFIGURE_COMMAND ""
	  BUILD_COMMAND ""
	)

	FetchContent_GetProperties(color-console)
	if (NOT color-console_POPULATED)
	    FetchContent_Populate(color-console)
	endif()

	add_library(color-console INTERFACE)
	target_include_directories(color-console INTERFACE ${color-console_SOURCE_DIR}/include)
endif()