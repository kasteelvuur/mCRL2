// Author(s): Richard Farla
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file cflobdd.cpp


#include "mcrl2/atermpp/aterm.h"


using namespace atermpp; 


class cflobdd: public aterm
{
  public:
    cflobdd(const function_symbol& f)
      : aterm(f)
    {}

    cflobdd(const function_symbol& f, const cflobdd& c1, const cflobdd& c2)
      : aterm(f,c1,c2)
    {}
};

class cflobdd_i: public cflobdd
{
  // Constructor
  cflobdd_i()
    : cflobdd(function_symbol("i", 0))
  {}
};

class cflobdd_v: public cflobdd
{
  // Constructor
  cflobdd_v()
    : cflobdd(function_symbol("v", 0))
  {}
};

class cflobdd_c: public cflobdd
{
  // Constructor
  cflobdd_c(const cflobdd& c1, const cflobdd& c2)
    : cflobdd(function_symbol("c", 2), c1, c2)
  {}
};
