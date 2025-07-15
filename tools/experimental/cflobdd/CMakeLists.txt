mcrl2_add_tool(cflobdd_test
  SOURCES
    test.cpp
    aterm_cflobdd_io.cpp
  DEPENDS
    mcrl2_atermpp
)

mcrl2_add_tool(lps2cflobdd
  SOURCES
    lps2cflobdd.cpp
    aterm_cflobdd_io.cpp
  DEPENDS
    mcrl2_lps
    mcrl2_atermpp
)
