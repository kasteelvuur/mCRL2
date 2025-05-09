// Author(s): Richard Farla
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aterm_pair.h
/// \brief Term containing two terms.


#ifndef MCRL2_ATERMPP_ATERM_PAIR_H
#define MCRL2_ATERMPP_ATERM_PAIR_H


#include "mcrl2/atermpp/aterm.h"


namespace atermpp
{

namespace
{
  global_function_symbol g_pair("pair", 2);
}

/// \brief A pair term stores two terms. It carries these as arguments.
class aterm_pair : public aterm
{
public:
  /// \brief Default constructor.
  aterm_pair()
   : aterm()
  {}

  /// \brief Constructs a pair term from two terms.
  /// \param first The first term of the pair.
  /// \param second The second term of the pair.
  explicit aterm_pair(const aterm& first, const aterm& second)
    : aterm(g_pair, first, second)
  {}

  /// \brief Constructs a pair term from an aterm.
  /// \param t The aterm to cast
  explicit aterm_pair(const aterm& t)
    : aterm(t)
  {
    assert(t.function() == g_pair || !defined());
  }

  /// \brief Provide the first term stored in the pair. 
  /// \return The first term.
  aterm first() const noexcept
  {
    return (*this)[0];
  }

  /// \brief Provide the second term stored in the pair. 
  /// \return The second term.
  aterm second() const noexcept
  {
    return (*this)[1];
  }
};

} // namespace atermpp

namespace std
{

/// \brief Standard hash function.
template<> struct hash<atermpp::aterm_pair>
{
  size_t operator()(const atermpp::aterm_pair& t) const
  {
    return hash<atermpp::aterm>()(t);
  }
};

} // namespace std

#endif // MCRL2_ATERMPP_ATERM_INT_H
