if(MCRL2_ENABLE_OXIDD)

  mcrl2_add_tool(bdd_test
    SOURCES
      test.cpp
      bdd_io.cpp
    DEPENDS
      oxidd
  )

  mcrl2_add_tool(lps2bdd
    SOURCES
      lps2bdd.cpp
      bdd_io.cpp
    DEPENDS
      mcrl2_lps
      oxidd
  )

endif()