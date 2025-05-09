if(MCRL2_ENABLE_OXIDD)

  mcrl2_add_tool(bdd
    SOURCES
      test.cpp
      bdd_io.cpp
    DEPENDS
      oxidd
  )

endif()