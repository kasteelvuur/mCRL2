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
  bool is_cflobdd() const noexcept
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
  bool is_reduced() const noexcept
  {
    // The result mapping may not contain any duplicates
    const aterm_list& vs = down_cast<aterm_list>((*this)[1]);
    if (vs.size() != as_set(vs).size()) return 0;

    // The proto-CFLOBDD must also be reduced
    const aterm_proto_cflobdd& c = down_cast<aterm_proto_cflobdd>((*this)[0]);
    if (!c.is_reduced()) return 0;

    return 1;
  }

  /// \brief Evaluate this CFLOBDD.
  /// \param sigma Proposition letter assignments in order
  /// \return The result of this CFLOBDD given the proposition letter assignments
  size_t evaluate(const std::vector<bool>& sigma) const noexcept
  {
    // Evaluate the proto-CFLOBDD to determine which result mapping value should be used
    const aterm_proto_cflobdd& c = down_cast<aterm_proto_cflobdd>((*this)[0]);
    const size_t& x = c.evaluate(sigma);

    // Return the corresponding result mapping value
    const aterm_list& vs = down_cast<aterm_list>((*this)[1]);
    const aterm_int v = down_cast<aterm_int>(as_vector(vs)[x]);
    return v.value();
  }

  /// \brief Negate this CFLOBDD.
  /// \return The new CFLOBDD
  aterm_cflobdd negate() const noexcept
  {
    const aterm_list& f = down_cast<aterm_list>((*this)[1]);
    aterm_list g;

    // Reversely iterate over the result mapping such that we push in the correct order
    for (reverse_term_list_iterator i = f.rbegin(); i != f.rend(); ++i)
    {
      const size_t& value = down_cast<aterm_int>(*i).value();
      if (value == 0)
      {
        // Flip the result mapping from 0 to 1
        g.push_front(aterm_int(1));
      }
      else if (value == 1)
      {
        // Flip the result mapping from 1 to 0
        g.push_front(aterm_int(0));
      }
      else
      {
        // This case should never happen
        assert(false);
        g.push_front(*i);
      }
    }

    return aterm_cflobdd((*this)[0], g);
  }

  /// \brief Combine this CFLOBDD with another by applying a binary operator, ensuring that the evaluation
  ///   for any assignment equals combining the separate evaluations according to the binary operator.
  /// \param other The CFLOBDD to combine with
  /// \param func The binary operator
  /// \return The new CFLOBDD
  aterm_cflobdd apply_and_reduce(
    const aterm_cflobdd& other,
    aterm_int(*func)(const aterm_int&, const aterm_int&)
  ) const noexcept
  {
    // assert(this->is_reduced() && other.is_reduced());
    const aterm_proto_cflobdd& c_1 = down_cast<aterm_proto_cflobdd>((*this)[0]);
    const aterm_proto_cflobdd& c_2 = down_cast<aterm_proto_cflobdd>(other[0]);
    assert(c_1.level() == c_2.level());

    // Calculate the pair product
    const aterm_pair& pair_product = c_1.pair_product(c_2);
    const aterm_proto_cflobdd& product_proto_cflobdd = down_cast<aterm_proto_cflobdd>(pair_product.first());
    const aterm_list& product_results = down_cast<aterm_list>(pair_product.second());

    // Calculate new result mapping values
    // Reversely iterate over the result mapping pairs such that we push in the correct order
    aterm_list combined_results;
    for (reverse_term_list_iterator i = product_results.rbegin(); i != product_results.rend(); ++i)
    {
      const aterm_pair& result_pair = down_cast<aterm_pair>(*i);
      const aterm_int& value_1 = down_cast<aterm_int>(result_pair.first());
      const aterm_int& value_2 = down_cast<aterm_int>(result_pair.second());
      combined_results.push_front(func(value_1, value_2));
    }

    // Collapse the new result mapping values to get rid of duplicates and reduced mapping violations
    const aterm_pair& collapsed_results = collapse_classes_leftmost(combined_results);
    const aterm_list& projected_results = down_cast<aterm_list>(collapsed_results.first());
    const aterm_list& renumbered_results = down_cast<aterm_list>(collapsed_results.second());

    // Reduce the pair product proto-CFLOBDD according to the renumbered results
    const aterm_proto_cflobdd& reduced_proto_cflobdd = product_proto_cflobdd.reduce(renumbered_results);
    assert(reduced_proto_cflobdd.is_reduced());

    return aterm_cflobdd(reduced_proto_cflobdd, projected_results);
  }

  /// \brief Calculate the conjunction of two CFLOBDDs.
  /// \param other The CFLOBDD to combine with
  /// \return The new CFLOBDD
  aterm_cflobdd operator&&(const aterm_cflobdd& other) const noexcept
  {
    return this->apply_and_reduce(
      other,
      [](const aterm_int& i, const aterm_int& j) -> aterm_int
      {
        return aterm_int(i.value() && j.value());
      }
    );
  }
};

} // namespace atermpp

#endif // MCRL2_ATERMPP_ATERM_CFLOBBD_H
