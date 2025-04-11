// Author(s): Richard Farla
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aterm_cflobdd.h


#ifndef MCRL2_ATERMPP_ATERM_CFLOBBD_H
#define MCRL2_ATERMPP_ATERM_CFLOBBD_H


#include "aterm_proto_cflobdd.h"


namespace atermpp
{

/// \brief Get the CFLOBDD function symbol.
/// \return The CFLOBDD function symbol
inline
const function_symbol& function_symbol_cflobdd()
{
  static function_symbol function_symbol_cflobdd = function_symbol("CFLOBDD", 2);
  return function_symbol_cflobdd;
}

/// \brief A CFLOBDD term encodes a boolean function.
class aterm_cflobdd: public aterm
{
public:
  /// \brief Construct a CFLOBDD term from an aterm.
  /// \param term The aterm to cast
  aterm_cflobdd(const aterm& term)
    : aterm(term)
  {
    assert(is_cflobdd());
  }

  /// \brief Construct a CFLOBDD term from a proto-CFLOBDD and result mapping.
  /// \param c The proto-CFLOBDD
  /// \param f The result mapping
  aterm_cflobdd(const aterm_proto_cflobdd& c, const aterm_list& vs)
    : aterm(function_symbol_cflobdd(), c, vs)
  {
    assert(is_cflobdd());
  }

  /// \brief Check if this term is a CFLOBDD.
  /// \return Whether this term is a CFLOBDD
  size_t is_cflobdd() const noexcept
  {
    // The function symbol must match the CFLOBDD function symbol
    if (this->function() != function_symbol_cflobdd()) return 0;

    // The proto-CFLOBDD must also be validated
    // This is already done in debug mode via the downcast and the corresponding assert
    // Consider adding an explicit call if this function is ever used outside asserts
    const aterm_proto_cflobdd& c = down_cast<aterm_proto_cflobdd>((*this)[0]);
    const aterm_list& vs = down_cast<aterm_list>((*this)[1]);

    // The out degree must match the amount of result mapping values
    if (c.out_degree() != vs.size()) return 0;

    // All result mapping values must be either 0 or 1, for the booleans False and True respectively
    for (const aterm& v : vs)
    {
      const size_t& value = down_cast<aterm_int>(v).value();
      if (value != 0 && value != 1) return 0;
    }

    return 1;
  }

  /// \brief Check if this CFLOBDD is reduced.
  /// \return Whether this CFLOBDD is reduced
  size_t is_reduced() const noexcept
  {
    // The result mapping may not contain any duplicates
    const aterm_list& vs = down_cast<aterm_list>((*this)[1]);
    if (vs.size() != as_set(vs).size()) return 0;

    // The proto-CFLOBDD must also be reduced
    const aterm_proto_cflobdd& c = down_cast<aterm_proto_cflobdd>((*this)[0]);
    if (!c.is_reduced()) return 0;

    return 1;
  }
};

} // namespace atermpp

#endif // MCRL2_ATERMPP_ATERM_CFLOBBD_H
