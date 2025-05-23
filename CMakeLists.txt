cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    startgit
    VERSION 0.2.1
    DESCRIPTION "Static page generator for git repositories"
    HOMEPAGE_URL "https://git.dimitrijedobrota.com/stargit.git"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare dependencies ----

find_package(git2wrap CONFIG REQUIRED)
find_package(hemplate 0.2.2 CONFIG REQUIRED)
find_package(md4c CONFIG REQUIRED)
find_package(poafloc 1.2 CONFIG REQUIRED)

# ---- Declare library ----

add_library(
    startgit_lib OBJECT
    source/arguments.cpp
    source/common.cpp
    source/branch.cpp
    source/commit.cpp
    source/diff.cpp
    source/file.cpp
    source/html.cpp
    source/repository.cpp
    source/tag.cpp
    source/utils.cpp
)

target_link_libraries(startgit_lib PUBLIC git2wrap::git2wrap hemplate::hemplate poafloc::poafloc md4c::md4c-html)

target_include_directories(
    startgit_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(startgit_lib PUBLIC cxx_std_20)

# ---- Declare executable ----

add_executable(startgit_exe source/startgit.cpp)
add_executable(startgit::exe ALIAS startgit_exe)

set_property(TARGET startgit_exe PROPERTY OUTPUT_NAME startgit)

target_compile_features(startgit_exe PRIVATE cxx_std_20)

target_link_libraries(startgit_exe PRIVATE git2wrap startgit_lib)


add_executable(startgit-index_exe source/startgit-index.cpp)
add_executable(startgit-index::exe ALIAS startgit-index_exe)

set_property(TARGET startgit-index_exe PROPERTY OUTPUT_NAME startgit-index)

target_compile_features(startgit-index_exe PRIVATE cxx_std_20)

target_link_libraries(startgit-index_exe PRIVATE git2wrap startgit_lib)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT startgit_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of startgit"
  )
endif()

include(cmake/dev-mode.cmake)
