// Author(s): Richard Farla
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file lps2bdd.cpp
/// \brief This tool transforms an .lps file into a binary decision diagram.

#include <csignal>
#include <memory>
#include "bdd_io.h"
#include "mcrl2/utilities/input_tool.h"
#include "mcrl2/lps/io.h"

using namespace mcrl2;
using utilities::tools::input_tool;

class lps2bdd_tool: public input_tool
{
  typedef input_tool super;

  public:
    lps2bdd_tool()
      : super("lps2bdd",
              "Richard Farla",
              "generates a BDD from an LPS",
              "Transforms the LPS in INFILE to a BDD. "
              "If INFILE is not present or '-', stdin is used."
             )
    {}

    bool run() override
    {
      lps::specification lpsspec;
      lps::load_lps(lpsspec, input_filename());
      std::cout << lpsspec << "\n";
      return true;
    }
};

int main(int argc, char** argv)
{
  return lps2bdd_tool().execute(argc, argv);
}
