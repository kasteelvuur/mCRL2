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

/// \brief Collapse a list of values.
/// \param values The values to collapse
/// \return A pair containing the projected and renumbered values
inline aterm_pair collapse_classes_leftmost(const aterm_list& values) noexcept
{
  std::vector<aterm_int> projected;
  std::vector<aterm_int> renumbered;

  for (const aterm& v : values)
  {
    // Get the location of the value in the projected list
    const aterm_int& value = down_cast<aterm_int>(v);
    const std::vector<aterm_int>::iterator& value_location = std::find(projected.begin(), projected.end(), value);

    // Project the values such that we only keep the leftmost occurrence of duplicates
    if (value_location == projected.end()) projected.push_back(value);

    // Renumber the values such that they coincide with the index where the projected list holds the original value
    renumbered.push_back(aterm_int(value_location - projected.begin()));
  }

  return aterm_pair(
    aterm_list(projected.begin(), projected.end()),
    aterm_list(renumbered.begin(), renumbered.end())
  );
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

  /// \brief Evaluate this proto-CFLOBDD.
  /// \param sigma Proposition letter assignments in order
  /// \return The result of this proto-CFLOBDD given the proposition letter assignments
  size_t evaluate(const std::vector<bool>& sigma) const noexcept
  {
    // The amount of proposition letters must be equal to two to the power of the proto-CFLOBDD's level
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

  /// \brief Calculate the pair product between this and another proto-CFLOBDD.
  /// \param other The other proto-CFLOBDD
  /// \return The pair product
  aterm_pair pair_product(const aterm_proto_cflobdd& other) const noexcept
  {
    aterm_list values;

    // Return the other proto-CFLOBDD, pairing its outputs with zero, if this is a no-distinction proto-CFLOBDD
    size_t out_degree = this->out_degree();
    if (out_degree == 1)
    {
      const aterm_int& zero = aterm_int(0);
      for (size_t i = out_degree; i > 0; i--)
      {
        values.push_front(aterm_pair(zero, aterm_int(i)));
      }
      return aterm_pair(other, values);
    }

    // Return this proto-CFLOBDD, pairing its outputs with zero, if the other is a no-distinction proto-CFLOBDD
    out_degree = other.out_degree();
    if (out_degree == 1)
    {
      const aterm_int& zero = aterm_int(0);
      for (size_t i = out_degree; i > 0; i--)
      {
        values.push_front(aterm_pair(aterm_int(i), zero));
      }
      return aterm_pair(*this, values);
    }

    // Return V, with the same result for either proto-CFLOBDD, if both proto-CFLOBDDs are V
    const aterm_proto_cflobdd& v = aterm_proto_cflobdd(function_symbol_v());
    if (*this == v && other == v)
    {
      const aterm_int& zero = aterm_int(0);
      const aterm_int& one = aterm_int(1);
      values.push_front(aterm_pair(one, one));
      values.push_front(aterm_pair(zero, zero));
      return aterm_pair(v, values);
    }

    // Recursively calculate the pair product of the entree proto-CFLOBDDs
    const aterm_proto_cflobdd& this_entree = down_cast<aterm_proto_cflobdd>((*this)[0]);
    const aterm_proto_cflobdd& other_entree = down_cast<aterm_proto_cflobdd>(other[0]);
    const aterm_pair& entree_pair_product = this_entree.pair_product(other_entree);
    const aterm_proto_cflobdd& entree_proto_cflobdd = down_cast<aterm_proto_cflobdd>(entree_pair_product.first());
    const aterm_list& entree_results = down_cast<aterm_list>(entree_pair_product.second());

    // Recursively calculate the pair product of the proto-CFLOBDDs found by following the entree product results
    const std::vector<aterm> this_cvs = as_vector(down_cast<aterm_list>((*this)[1]));
    const std::vector<aterm> other_cvs = as_vector(down_cast<aterm_list>(other[1]));
    std::vector<aterm_pair> cvs;
    std::vector<aterm_pair> value_pairs;
    for (const aterm& entree_result : entree_results)
    {
      const aterm_pair& entree_result_pair = down_cast<aterm_pair>(entree_result);
      const aterm_int& this_i = down_cast<aterm_int>(entree_result_pair.first());
      const aterm_int& other_i = down_cast<aterm_int>(entree_result_pair.second());
      const aterm_pair& this_cv = down_cast<aterm_pair>(this_cvs[this_i.value()]);
      const aterm_pair& other_cv = down_cast<aterm_pair>(other_cvs[other_i.value()]);

      // Calculate the pair product between the two proto-CFLOBDDs
      const aterm_proto_cflobdd& this_c = down_cast<aterm_proto_cflobdd>(this_cv.first());
      const aterm_proto_cflobdd& other_c = down_cast<aterm_proto_cflobdd>(other_cv.first());
      const aterm_pair& pair_product = this_c.pair_product(other_c);
      const aterm_proto_cflobdd& proto_cflobdd = down_cast<aterm_proto_cflobdd>(pair_product.first());
      const aterm_list& product_result = down_cast<aterm_list>(pair_product.second());

      // Calculate the corresponding result mapping values
      const std::vector<aterm> this_values = as_vector(down_cast<aterm_list>(this_cv.second()));
      const std::vector<aterm> other_values = as_vector(down_cast<aterm_list>(other_cv.second()));
      std::vector<aterm_int> return_values;
      for (const aterm& product_result : product_result)
      {
        // Get the return values index j for both proto-CFLOBDDs
        const aterm_pair& product_result_pair = down_cast<aterm_pair>(product_result);
        const aterm_int& this_j = down_cast<aterm_int>(product_result_pair.first());
        const aterm_int& other_j = down_cast<aterm_int>(product_result_pair.second());

        // Get the value corresponding to m(i,j) for both proto-CFLOBDDs
        const aterm_int& this_value = down_cast<aterm_int>(this_values[this_j.value()]);
        const aterm_int& other_value = down_cast<aterm_int>(other_values[other_j.value()]);
        const aterm_pair& value_pair = aterm_pair(this_value, other_value);

        // Add the pair of values if it is not included yet
        // Set the return value m(i,j) for the new proto-CFLOBDD to the index of the pair
        const std::vector<aterm_pair>::iterator& value_location = std::find(value_pairs.begin(), value_pairs.end(), value_pair);
        const size_t& index = value_location - value_pairs.begin();
        if (value_location == value_pairs.end()) value_pairs.push_back(value_pair);
        return_values.push_back(aterm_int(index));
      }
      cvs.push_back(aterm_pair(proto_cflobdd, aterm_list(return_values.begin(), return_values.end())));
    }

    // Construct and return the new proto-CFLOBDD and value pairs
    return aterm_pair(
      aterm_proto_cflobdd(entree_proto_cflobdd, aterm_list(cvs.begin(), cvs.end())),
      aterm_list(value_pairs.begin(), value_pairs.end())
    );
  }

  /// \brief Reduce this proto-CFLOBDD according to the new return values.
  /// \param values The new return values
  /// \return The reduces proto-CFLOBDD
  aterm_proto_cflobdd reduce(const aterm_list& values) const noexcept
  {
    // This proto-CFLOBDD cannot be reduced according to values that read [0, ..., |values| - 1]
    const size_t& size = values.size();
    size_t next = 0;
    for (const aterm& v : values)
    {
      const size_t& value = down_cast<aterm_int>(v).value();
      if (value != next) break;
      else if (value == size - 1) return *this;
      else next++;
    }

    // Reduce to a no-distinction proto-CFLOBDD if there is only one unique value
    if (as_set(values).size() == 1) return no_distinction(this->level());

    // Recursively propogate the new return mapping values to all included proto-CFLOBDDs 
    const std::vector<aterm> values_vec = as_vector(values);
    const aterm_list& this_cvs = down_cast<aterm_list>((*this)[1]);
    std::vector<aterm_pair> new_cvs_vec;
    std::vector<aterm_int> entree_values_vec;
    for (const aterm& this_cv : this_cvs)
    {
      // Get the new return mapping values for this proto-CFLOBDD
      const aterm_pair& this_cv_pair = down_cast<aterm_pair>(this_cv);
      const aterm_proto_cflobdd& this_proto_cflobdd = down_cast<aterm_proto_cflobdd>(this_cv_pair.first());
      const aterm_list& this_values = down_cast<aterm_list>(this_cv_pair.second());
      aterm_list new_values;
      for (reverse_term_list_iterator i = this_values.rbegin(); i != this_values.rend(); ++i)
      {
        const size_t& index = down_cast<aterm_int>(*i).value(); 
        new_values.push_front(values_vec[index]);
      }

      // Propagate the new return mapping values
      const aterm_pair& collapsed_values = collapse_classes_leftmost(new_values);
      const aterm_list& projected_values = down_cast<aterm_list>(collapsed_values.first());
      const aterm_list& renumbered_values = down_cast<aterm_list>(collapsed_values.second());
      const aterm_proto_cflobdd& reduced_proto_cflobdd = this_proto_cflobdd.reduce(renumbered_values);
      const aterm_pair& new_cv = aterm_pair(reduced_proto_cflobdd, projected_values);

      // Add the pair of reduced proto-CFLOBDD and result mapping values if it is not included yet
      // Set the renumbered return value for the entree proto-CFLOBDD to the index of the pair
      const std::vector<aterm_pair>::iterator& cvs_location = std::find(new_cvs_vec.begin(), new_cvs_vec.end(), new_cv);
      const size_t& index = cvs_location - new_cvs_vec.begin();
      if (cvs_location == new_cvs_vec.end()) new_cvs_vec.push_back(new_cv);
      entree_values_vec.push_back(aterm_int(index));
    }
    const aterm_list& new_cvs = aterm_list(new_cvs_vec.begin(), new_cvs_vec.end());

    // Reduce the entree proto-CFLOBDD accordingly
    const aterm_list& entree_values = aterm_list(entree_values_vec.begin(), entree_values_vec.end());
    const aterm_proto_cflobdd& this_c = down_cast<aterm_proto_cflobdd>((*this)[0]);
    const aterm_proto_cflobdd& new_c = this_c.reduce(entree_values);

    const aterm_proto_cflobdd& new_proto_cflobdd = aterm_proto_cflobdd(new_c, new_cvs);
    assert(new_proto_cflobdd.is_reduced());
    return new_proto_cflobdd;
  }

  /// \brief Construct a no-distinction proto-CFLOBDD of a specific level.
  /// \param level The level of the no-distinction proto-CFLOBDD
  /// \return A no-distinction proto-CFLOBDD of the provided level
  aterm_proto_cflobdd no_distinction(const size_t& level) const noexcept
  {
    // Base case I
    if (level == 0) return aterm_proto_cflobdd(function_symbol_i());

    // Inductively build on lower level no-distinction proto-CFLOBDDs
    const aterm_proto_cflobdd& lower = no_distinction(level - 1);
    const aterm_list& values = {aterm_int(0)};
    const aterm_list& cvs = {aterm_pair(lower, values)};
    return aterm_proto_cflobdd(lower, cvs);
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
