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

/// \brief Get the pair term function symbol.
/// \return The pair term function symbol
inline
const function_symbol& function_symbol_pair()
{
  static function_symbol function_symbol_pair = function_symbol("pair", 2);
  return function_symbol_pair;
}

/// \brief A pair term stores two terms. It carries these as arguments.
class aterm_pair : public aterm
{
public:
  /// \brief Constructs a pair term from two terms.
  /// \param first The first term of the pair.
  /// \param second The second term of the pair.
  explicit aterm_pair(const aterm& first, const aterm& second)
    : aterm(function_symbol_pair(), first, second)
  {}

  /// \brief Constructs a pair term from an aterm.
  /// \param t The aterm to cast
  explicit aterm_pair(const aterm& t)
    : aterm(t)
  {
    assert(t.function() == function_symbol_pair() || !defined());
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

#endif // MCRL2_ATERMPP_ATERM_INT_H
