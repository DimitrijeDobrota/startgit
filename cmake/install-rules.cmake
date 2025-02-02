install(
    TARGETS startgit_exe
    RUNTIME COMPONENT startgit_Runtime
)

install(
    TARGETS startgit-index_exe
    RUNTIME COMPONENT startgit_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
