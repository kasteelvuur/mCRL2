// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file action_test.cpp
/// \brief Add your file description here.

#include <iostream>
#include <string>
#include <set>
#include <boost/test/minimal.hpp>
#include "mcrl2/atermpp/atermpp.h"
#include "mcrl2/new_data/function_symbol.h"
#include "mcrl2/new_data/function_symbol.h"
#include "mcrl2/new_data/detail/utility.h"
#include "mcrl2/lps/action.h"

using namespace atermpp;
using namespace mcrl2::core;
using namespace mcrl2::new_data;
using namespace mcrl2::lps;

int test_main(int argc, char** argv )
{
  using mcrl2::new_data::make_vector;
  using mcrl2::core::pp;

  MCRL2_ATERMPP_INIT(argc, argv)

  sort_expression X(basic_sort("X"));
  sort_expression Y(basic_sort("Y"));

  action_label aX(identifier_string("a"),  make_vector(X));
  action_label aY(identifier_string("a"),  make_vector(Y));
  action_label aXY(identifier_string("a"), make_vector(X, Y));
  action_label bX(identifier_string("b"),  make_vector(X));

  data_expression x_X = variable("x", X);
  data_expression y_X = variable("y", X);
  data_expression y_Y = variable("y", Y);

  action a1(aX, make_vector(x_X));
  action a2(aX, make_vector(y_X));
  action a3(bX, make_vector(x_X));

  BOOST_CHECK(equal_signatures(a1, a1));
  BOOST_CHECK(equal_signatures(a1, a2));
  BOOST_CHECK(!equal_signatures(a1, a3));

  action empty = action();
  std::cerr << pp(empty) << std::endl;
  std::cerr << pp(action()) << std::endl;

  return 0;
}
