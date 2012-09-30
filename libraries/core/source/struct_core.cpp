// Author(s): Aad Mathijssen
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file struct_core.cpp

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include "mcrl2/aterm/aterm.h"
#include "mcrl2/utilities/detail/memory_utility.h"
#include "mcrl2/core/detail/struct_core.h"
#include "mcrl2/utilities/logger.h"

using namespace mcrl2::core;

namespace mcrl2
{
namespace core
{
namespace detail
{

ATermAppl gsFreshString2ATermAppl(const char* s, const ATerm &Term, bool TryNoSuffix)
{
  bool found = false;
  ATermAppl NewTerm = gsString2ATermAppl(s);
  if (TryNoSuffix)
  {
    //try "s"
    found = !gsOccurs(NewTerm, Term);
  }
  if (!found)
  {
    //find "sk" that does not occur in Term
    for (int i = 0; i < INT_MAX && !found; i++)
    {
      std::stringstream Name;
      Name << s << i;
      NewTerm = gsString2ATermAppl(Name.str().c_str());
      found = !gsOccurs(NewTerm, Term);
    }
  }
  if (found)
  {
    return NewTerm;
  }
  else
  {
    //there is no fresh ATermAppl "si", with 0 <= i < INT_MAX
    mCRL2log(log::error) << "cannot generate fresh ATermAppl with prefix " << s << std::endl;
    return ATermAppl();
  }
}

} //namespace detail
} //namespace core
} //namespace mcrl2
