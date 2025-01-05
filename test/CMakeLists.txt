# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(startgitTests LANGUAGES CXX)

# ---- Tests ----

add_executable(startgit_test source/startgit_test.cpp)
target_link_libraries(startgit_test PRIVATE startgit_lib)
target_compile_features(startgit_test PRIVATE cxx_std_20)

add_test(NAME startgit_test COMMAND startgit_test)

# ---- End-of-file commands ----

add_folders(Test)
