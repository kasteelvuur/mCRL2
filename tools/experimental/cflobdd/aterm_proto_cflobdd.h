// Author(s): Richard Farla
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aterm_proto_cflobdd.h


#ifndef MCRL2_ATERMPP_ATERM_PROTO_CFLOBBD_H
#define MCRL2_ATERMPP_ATERM_PROTO_CFLOBBD_H


#include "aterm_pair.h"

#include "mcrl2/atermpp/aterm.h"
#include "mcrl2/atermpp/aterm_int.h"
#include "mcrl2/atermpp/aterm_list.h"


namespace atermpp
{
/// \brief Get the constant proto-CFLOBDD I function symbol.
/// \return The constant proto-CFLOBDD I function symbol
inline
const function_symbol& function_symbol_i()
{
  static function_symbol function_symbol_i = function_symbol("proto-CFLOBDD-I", 0);
  return function_symbol_i;
}

/// \brief Get the constant proto-CFLOBDD V function symbol.
/// \return The constant proto-CFLOBDD V function symbol
inline
const function_symbol& function_symbol_v()
{
  static function_symbol function_symbol_v = function_symbol("proto-CFLOBDD-V", 0);
  return function_symbol_v;
}

/// \brief Get the inductive proto-CFLOBDD (L, [L_0, ..., L_{n-1}], m) function symbol.
/// \return The inductive proto-CFLOBDD (L, [L_0, ..., L_{n-1}], m) function symbol
inline
const function_symbol& function_symbol_c()
{
  static function_symbol function_symbol_c = function_symbol("proto-CFLOBDD-C", 2);
  return function_symbol_c;
}

/// \brief A proto-CFLOBDD term forms a building block for a CFLOBDD.
class aterm_proto_cflobdd : public aterm
{
public:
  /// \brief Construct a proto-CFLOBDD term from an aterm.
  /// \param term The aterm to cast
  aterm_proto_cflobdd(const aterm& term)
    : aterm(term)
  {
    assert(is_proto_cflobdd());
  }

  /// \brief Constant proto-CFLOBDD I or V.
  /// \param function_symbol The function symbol for I or V
  aterm_proto_cflobdd(const function_symbol& function_symbol)
    : aterm(function_symbol)
  {
    assert(is_proto_cflobdd());
  }

  /// \brief Proto-CFLOBDD inductive case (L, [L_0, ..., L_{n-1}], m).
  /// \param c The proto-CFLOBDD L
  /// \param cvs The list of proto-CFLOBDDs [L_0, ..., L_{n-1}] and mapping m merged into a list of pairs.
  ///   Each proto-CFLOBDD L_i is paired with a list of mapping result values v_i such that
  ///   L_i.out_degree() = v_i.size() and v_i[j] = m(i,j).
  aterm_proto_cflobdd(const aterm_proto_cflobdd& c, const aterm_list& cvs)
    : aterm(function_symbol_c(), c, cvs)
  {
    assert(is_proto_cflobdd());
  }

  /// \brief Check if this term is a proto-CFLOBDD.
  /// \return Whether this term is a proto-CFLOBDD
  bool is_proto_cflobdd() const noexcept
  {
    if (this->function() == function_symbol_i() || this->function() == function_symbol_v())
    {
      // Constant proto-CFLOBDDs I and V have no arguments to check
      return 1;
    }
    else if (this->function() == function_symbol_c())
    {
      // Inductive proto-CFLOBDD (L, [L_0, ..., L_{n-1}], m), see constructors for L_i and m merge specification
      const aterm_proto_cflobdd& c = down_cast<aterm_proto_cflobdd>((*this)[0]);
      const aterm_list& cvs = down_cast<aterm_list>((*this)[1]);

      // The out degree of the first proto-CFLOBDD must match the number of remaining proto-CFLOBDDs
      if (c.out_degree() != cvs.size()) return 0;

      for (const aterm& cv : cvs)
      {
        const aterm_pair& pair = down_cast<aterm_pair>(cv);
        const aterm_proto_cflobdd& c_i = down_cast<aterm_proto_cflobdd>(pair.first());
        const aterm_list& values = down_cast<aterm_list>(pair.second());

        // All proto-CFLOBDDs must have the same level
        if (c.level() != c_i.level()) return 0;

        // The out degree of each proto-CFLOBDD L_i must match the amount of corresponding return values
        if (c_i.out_degree() != values.size()) return 0;
      }

      // Each proto-CFLOBDDs L and L_i must also be validated
      // This is already done recursively in debug mode via the downcasts and the corresponding assert
      // Consider adding explicit recursive calls if this function is ever used outside asserts

      return 1;
    }
    else
    {
      // The function symbol does not match any known proto-CFLOBDD
      return 0;
    }
  }

  /// \brief Get the level of the proto-CFLOBDD.
  /// \details The constant proto-CFLOBDDs I and V have level 0.
  ///   The inductive proto-CFLOBDD (L, [L_0, ..., L_{n-1}], m) is one level higher than its children.
  /// \return The level of the proto-CFLOBDD
  size_t level() const noexcept
  {
    if (this->function() == function_symbol_i() || this->function() == function_symbol_v())
    {
      // Constant proto-CFLOBDDs I and V have level 0
      return 0;
    }
    else if (this->function() == function_symbol_c())
    {
      // Inductive proto-CFLOBDD (L, [L_0, ..., L_{n-1}], m) is one level higher than its children
      const aterm_proto_cflobdd& c = down_cast<aterm_proto_cflobdd>((*this)[0]);
      return c.level() + 1;
    }
    else
    {
      // This case should never happen
      assert(false);
      return 0;
    }
  }

  /// \brief Get the out degree of the proto-CFLOBDD.
  /// \details The constant proto-CFLOBDDs I and V have out degree 1 and 2 respectively.
  ///   The inductive proto-CFLOBDD (L, [L_0, ..., L_{n-1}], m) has out degree equal to its highest value + 1.
  /// \return The out degree of the proto-CFLOBDD
  size_t out_degree() const noexcept
  {
    if (this->function() == function_symbol_i())
    {
      // Constant proto-CFLOBDDs I has out degree 1
      return 1;
    }
    else if (this->function() == function_symbol_v())
    {
      // Constant proto-CFLOBDDs V has out degree 2
      return 2;
    }
    else if (this->function() == function_symbol_c())
    {
      // Inductive proto-CFLOBDD (L, [L_0, ..., L_{n-1}], m) has out degree equal to its highest value + 1
      const aterm_list& cvs = down_cast<aterm_list>((*this)[1]);
      size_t max = 0;
      for (const aterm& cv : cvs)
      {
        const aterm_pair& pair = down_cast<aterm_pair>(cv);
        const aterm_list& values = down_cast<aterm_list>(pair.second());
        for (const aterm& value : values)
        {
          max = std::max(max, down_cast<aterm_int>(value).value());
        }
      }
      return max + 1;
    }
    else
    {
      // This case should never happen
      assert(false);
      return 0;
    }
  }

  /// \brief Check if this proto-CFLOBDD is reduced.
  /// \return Whether this proto-CFLOBDD is reduced
  bool is_reduced() const noexcept
  {
    if (this->function() == function_symbol_i() || this->function() == function_symbol_v())
    {
      // Constant proto-CFLOBDDs I and V are always considered reduced
      return 1;
    }
    else if (this->function() == function_symbol_c())
    {
      // Inductive proto-CFLOBDD (L, [L_0, ..., L_{n-1}], m)
      const aterm_proto_cflobdd& c = down_cast<aterm_proto_cflobdd>((*this)[0]);
      const aterm_list& cvs = down_cast<aterm_list>((*this)[1]);

      // The proto-CFLOBDD L must also be reduced
      if (!c.is_reduced()) return 0;

      size_t next = 0;
      for (const aterm& cv : cvs)
      {
        const aterm_pair& pair = down_cast<aterm_pair>(cv);
        const aterm_proto_cflobdd& c_i = down_cast<aterm_proto_cflobdd>(pair.first());
        const aterm_list& values = down_cast<aterm_list>(pair.second());

        // All proto-CFLOBDDs L_i must also be reduced
        if (!c_i.is_reduced()) return 0;

        // There may be no duplicates within a single list of return values
        if (values.size() != as_set(values).size()) return 0;

        // The return values must form a compact extension
        for (const aterm& v : values)
        {
          const size_t& value = down_cast<aterm_int>(v).value();
          if (value > next) return 0;
          if (value == next) next++;
        }
      }

      // There may be no duplicate pairs
      if (cvs.size() != as_set(cvs).size()) return 0;

      return 1;
    }
    else
    {
      // This case should never happen
      assert(false);
      return 0;
    }
  }

  /// \brief Evaluate this proto-CFLOBDD.
  /// \param sigma Proposition letter assignments in order
  /// \return The result of this proto-CFLOBDD given the proposition letter assignments
  size_t evaluate(const std::vector<bool>& sigma) const noexcept
  {
    // The amount of proposition letters must equal to two to the power of the proto-CFLOBDD's level
    assert(sigma.size() == std::pow(2, this->level()));

    if (this->function() == function_symbol_i())
    {
      // Constant proto-CFLOBDDs I always evaluates to 0
      return 0;
    }
    else if (this->function() == function_symbol_v())
    {
      // Constant proto-CFLOBDDs V evaluates to 0 or 1 for the assignment False or True respectively
      return sigma[0] ? 1 : 0;
    }
    else if (this->function() == function_symbol_c())
    {
      // Inductive proto-CFLOBDD (L, [L_0, ..., L_{n-1}], m) evaluates inductively
      // Each call passes half of the proposition letter assignments
      const size_t& half_size = sigma.size() / 2;
      const std::vector<bool> split_left(sigma.begin(), sigma.begin() + half_size);
      const std::vector<bool> split_right(sigma.begin() + half_size, sigma.end());

      // Evaluate proto-CFLOBDD L to determine which proto-CFLOBDD L_i should be used next
      const aterm_proto_cflobdd& c = down_cast<aterm_proto_cflobdd>((*this)[0]);
      const size_t& i = c.evaluate(split_left);

      // Evaluate proto-CFLOBDD L_i to determine j for m(i,j)
      const aterm_list& cvs = down_cast<aterm_list>((*this)[1]);
      const aterm_pair cv = down_cast<aterm_pair>(as_vector(cvs)[i]);
      const aterm_proto_cflobdd& c_i = down_cast<aterm_proto_cflobdd>(cv.first());
      const size_t& j = c_i.evaluate(split_right);

      // Return the evaluation m(i,j)
      const aterm_list& m_i = down_cast<aterm_list>(cv.second());
      const aterm_int m_ij = down_cast<aterm_int>(as_vector(m_i)[j]);
      return m_ij.value();
    }
    else
    {
      // This case should never happen
      assert(false);
      return 0;
    }
  }
};

/// \brief Constant proto-CFLOBDD I.
class aterm_proto_cflobdd_i: public aterm_proto_cflobdd
{
public:
  aterm_proto_cflobdd_i()
    : aterm_proto_cflobdd(function_symbol_i())
  {}
};

/// \brief Constant proto-CFLOBDD V.
class aterm_proto_cflobdd_v: public aterm_proto_cflobdd
{
public:
  aterm_proto_cflobdd_v()
    : aterm_proto_cflobdd(function_symbol_v())
  {}
};

/// \brief Proto-CFLOBDD inductive case (L, [L_0, ..., L_{n-1}], m).
class aterm_proto_cflobdd_c: public aterm_proto_cflobdd
{
public:
  aterm_proto_cflobdd_c(const aterm_proto_cflobdd& c, const aterm_list& cvs)
    : aterm_proto_cflobdd(c, cvs)
  {}
};

} // namespace atermpp

#endif // MCRL2_ATERMPP_ATERM_PROTO_CFLOBBD_H
