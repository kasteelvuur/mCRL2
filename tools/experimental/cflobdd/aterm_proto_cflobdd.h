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

#include <unordered_set>


namespace atermpp
{

namespace
{
  global_function_symbol g_proto_cflobdd_i("proto-CFLOBDD-I", 0);
  global_function_symbol g_proto_cflobdd_v("proto-CFLOBDD-V", 0);
  global_function_symbol g_proto_cflobdd_c("proto-CFLOBDD-C", 2);
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

    // Renumber the values such that they coincide with the index where the projected list holds the original value
    renumbered.push_back(aterm_int(value_location - projected.begin()));

    // Project the values such that we only keep the leftmost occurrence of duplicates
    if (value_location == projected.end()) projected.push_back(value);
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
  /// \brief Default constructor.
  aterm_proto_cflobdd()
   : aterm()
  {}

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
    : aterm(g_proto_cflobdd_c, c, cvs)
  {
    assert(is_proto_cflobdd());
  }

  /// \brief Construct a no-distinction proto-CFLOBDD of a specific level.
  /// \param level The level of the no-distinction proto-CFLOBDD
  aterm_proto_cflobdd(const size_t& level)
  {
    if (!level)
    {
      // Base case - constant proto-CFLOBDD I
      *this = aterm_proto_cflobdd(g_proto_cflobdd_i);
    } 
    else
    {
      // Inductively build on lower level no-distinction proto-CFLOBDDs
      const aterm_proto_cflobdd& lower(level - 1);
      const aterm_list& values = {aterm_int(0)};
      const aterm_list& cvs = {aterm_pair(lower, values)};
      *this = aterm_proto_cflobdd(lower, cvs);
    }
  }

  /// \brief Construct a proto-CFLOBDD encoding only one proposition variable
  /// \param level The level of the proto-CFLOBDD
  /// \param variable_index The index of the proposition variable
  aterm_proto_cflobdd(const size_t& level, const size_t& variable_index)
  {
    assert(variable_index < std::pow(2, level));

    // Set the constant V for level 0
    if (!level)
    {
      *this = aterm_proto_cflobdd(g_proto_cflobdd_v);
      return;
    }

    // Determine the location of the proposition variable
    const aterm_proto_cflobdd& no_distinction(level - 1);
    const size_t& mid_index = std::pow(2, level - 1);
    if (variable_index < mid_index)
    {
      // The proposition variable is in the left split, so recurse there
      const aterm_proto_cflobdd& c = aterm_proto_cflobdd(level - 1, variable_index);
      const aterm_list& cvs = {
        aterm_pair(no_distinction, aterm_list {aterm_int(0)}),
        aterm_pair(no_distinction, aterm_list {aterm_int(1)})
      };
      *this = aterm_proto_cflobdd(c, cvs);
    }
    else
    {
      // The proposition variable is in the right split, so recurse there
      const aterm_proto_cflobdd& c = aterm_proto_cflobdd(level - 1, variable_index - mid_index);
      const aterm_list& cvs = {
        aterm_pair(c, aterm_list {aterm_int(0), aterm_int(1)})
      };
      *this = aterm_proto_cflobdd(no_distinction, cvs);
    }
  }

  /// \brief Check if this term is a proto-CFLOBDD.
  /// \return Whether this term is a proto-CFLOBDD
  bool is_proto_cflobdd() const noexcept
  {
    if (this->function() == g_proto_cflobdd_i || this->function() == g_proto_cflobdd_v)
    {
      // Constant proto-CFLOBDDs I and V have no arguments to check
      return 1;
    }
    else if (this->function() == g_proto_cflobdd_c)
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
    if (this->function() == g_proto_cflobdd_i || this->function() == g_proto_cflobdd_v)
    {
      // Constant proto-CFLOBDDs I and V are always considered reduced
      return 1;
    }
    else if (this->function() == g_proto_cflobdd_c)
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
    if (this->function() == g_proto_cflobdd_i || this->function() == g_proto_cflobdd_v)
    {
      // Constant proto-CFLOBDDs I and V have level 0
      return 0;
    }
    else if (this->function() == g_proto_cflobdd_c)
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
    if (this->function() == g_proto_cflobdd_i)
    {
      // Constant proto-CFLOBDDs I has out degree 1
      return 1;
    }
    else if (this->function() == g_proto_cflobdd_v)
    {
      // Constant proto-CFLOBDDs V has out degree 2
      return 2;
    }
    else if (this->function() == g_proto_cflobdd_c)
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

  /// \brief Get the vertex and edge count of the proto-CFLOBDD.
  /// \return A pair containing the vertex and edge count of the proto-CFLOBDD
  std::pair<size_t, size_t> count_vertices_and_edges(std::unordered_set<aterm>& counted) const noexcept
  {
    // Count each proto-CFLOBDD only once
    if (counted.find(*this) != counted.end()) return {0, 0};
    counted.insert(*this);

    if (this->function() == g_proto_cflobdd_i)
    {
      // Constant proto-CFLOBDDs I has an entry and exit vertex, with two corresponding edges
      return {2, 2};
    }
    else if (this->function() == g_proto_cflobdd_v)
    {
      // Constant proto-CFLOBDDs V has an entry vertex and two exit vertices, with two corresponding edges
      return {3, 2};
    }
    else if (this->function() == g_proto_cflobdd_c)
    {
      // Inductive proto-CFLOBDD (L, [L_0, ..., L_{n-1}], m)
      // Entry vertex has an edge to L
      size_t vertex_count = 1;
      size_t edge_count = 1;

      // Add the vertices and edges from L
      const aterm_proto_cflobdd& c = down_cast<aterm_proto_cflobdd>((*this)[0]);
      const auto& [c_vertex_count, c_edge_count]  = c.count_vertices_and_edges(counted);
      vertex_count += c_vertex_count;
      edge_count += c_edge_count;

      // The middle vertices have exactly one incoming and outgoing edge each
      const aterm_list& cvs = down_cast<aterm_list>((*this)[1]);
      const size_t& middle_vertex_count = cvs.size();
      vertex_count += middle_vertex_count;
      edge_count += 2 * middle_vertex_count;

      // Add the vertices and edges from [L_0, ..., L_{n-1}]
      for (const aterm& aterm_cv : cvs)
      {
        const aterm_pair& cv = down_cast<aterm_pair>(aterm_cv);
        const aterm_proto_cflobdd& c_i = down_cast<aterm_proto_cflobdd>(cv.first());
        const auto& [c_i_vertex_count, c_i_edge_count]  = c_i.count_vertices_and_edges(counted);
        vertex_count += c_i_vertex_count;
        edge_count += c_i_edge_count;

        // The number of return edges per proto-CFLOBDD is equal to its out degree
        edge_count += c_i.out_degree();
      }

      // The number of exit vertices is the out degree
      vertex_count += this->out_degree();

      return {vertex_count, edge_count};
    }
    else
    {
      // This case should never happen
      assert(false);
      return {0, 0};
    }
  }

  /// \brief Evaluate this proto-CFLOBDD.
  /// \param sigma Proposition letter assignments in order
  /// \return The result of this proto-CFLOBDD given the proposition letter assignments
  size_t evaluate(const std::vector<bool>& sigma) const noexcept
  {
    // The amount of proposition letters must be equal to two to the power of the proto-CFLOBDD's level
    assert(sigma.size() == std::pow(2, this->level()));

    if (this->function() == g_proto_cflobdd_i)
    {
      // Constant proto-CFLOBDDs I always evaluates to 0
      return 0;
    }
    else if (this->function() == g_proto_cflobdd_v)
    {
      // Constant proto-CFLOBDDs V evaluates to 0 or 1 for the assignment False or True respectively
      return sigma[0] ? 1 : 0;
    }
    else if (this->function() == g_proto_cflobdd_c)
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
    // Check if the pair product has already been evaluated
    static std::unordered_map<aterm_pair, aterm_pair> cache;
    std::unordered_map<aterm_pair, aterm_pair>::const_iterator cached_product = cache.find(aterm_pair(*this, other));
    if (cached_product != cache.end()) return cached_product->second;

    aterm_list values;
    const size_t& this_out_degree = this->out_degree();
    const size_t& other_out_degree = other.out_degree();

    // Return the other proto-CFLOBDD, pairing its outputs with zero, if this is a no-distinction proto-CFLOBDD
    if (this_out_degree == 1)
    {
      const aterm_int& zero = aterm_int(0);
      for (size_t i = other_out_degree; i > 0; i--)
      {
        values.push_front(aterm_pair(zero, aterm_int(i - 1)));
      }
      return aterm_pair(other, values);
    }

    // Return this proto-CFLOBDD, pairing its outputs with zero, if the other is a no-distinction proto-CFLOBDD
    if (other_out_degree == 1)
    {
      const aterm_int& zero = aterm_int(0);
      for (size_t i = this_out_degree; i > 0; i--)
      {
        values.push_front(aterm_pair(aterm_int(i - 1), zero));
      }
      return aterm_pair(*this, values);
    }

    // Return V, with the same result for either proto-CFLOBDD, if both proto-CFLOBDDs are V
    const aterm_proto_cflobdd& v = aterm_proto_cflobdd(g_proto_cflobdd_v);
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

    // Construct, cache, and return the new proto-CFLOBDD and value pairs
    const aterm_pair& pair_product = aterm_pair(
      aterm_proto_cflobdd(entree_proto_cflobdd, aterm_list(cvs.begin(), cvs.end())),
      aterm_list(value_pairs.begin(), value_pairs.end())
    );
    cache[aterm_pair(*this, other)] = pair_product;
    return pair_product;
  }

  /// \brief Reduce this proto-CFLOBDD according to the new return values.
  /// \param values The new return values
  /// \return The reduced proto-CFLOBDD
  aterm_proto_cflobdd reduce(const aterm_list& values) const noexcept
  {
    // Check if the reduction has already been evaluated
    static std::unordered_map<aterm_pair, aterm_proto_cflobdd> cache;
    std::unordered_map<aterm_pair, aterm_proto_cflobdd>::const_iterator cached_reduction = cache.find(aterm_pair(*this, values));
    if (cached_reduction != cache.end()) return cached_reduction->second;

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
    if (as_set(values).size() == 1) return aterm_proto_cflobdd(this->level());

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
    cache[aterm_pair(*this, values)] = new_proto_cflobdd;
    assert(new_proto_cflobdd.is_reduced());
    return new_proto_cflobdd;
  }

  /// \brief Fix a proposition letter assignment.
  /// \param index The index of the proposition letter
  /// \param value The fixed value
  /// \return The new proto-CFLOBDD with a list of old exit values at their new indices
  aterm_pair fix(const size_t& index, const aterm_int& value) const noexcept
  {
    assert(index < std::pow(2, this->level()));

    // Fixing a proposition letter assignment has no effect on no-distinction proto-CFLOBDDs
    if (this->out_degree() == 1)
    {
      return aterm_pair(*this, aterm_list {aterm_int(0)});
    }

    // Fix the proposition letter assignment if we have reached a constant proto-CFLOBDD V
    if (*this == aterm_proto_cflobdd(g_proto_cflobdd_v))
    {
      return aterm_pair(aterm_proto_cflobdd(g_proto_cflobdd_i), aterm_list {value});
    }

    // Inductive proto-CFLOBDD (L, [L_0, ..., L_{n-1}], m) remains
    const aterm_proto_cflobdd& c = down_cast<aterm_proto_cflobdd>((*this)[0]);
    const aterm_list& cvs = down_cast<aterm_list>((*this)[1]);

    aterm_proto_cflobdd new_c = c;
    std::vector<aterm_pair> new_cvs_vec;
    std::vector<aterm_int> new_values_vec;

    // Determine the location of the proposition letter
    const size_t& mid_index = std::pow(2, this->level() - 1);
    if (index < mid_index)
    {
      // The proposition letter is in the left split, so recurse there
      const aterm_pair& fixed_pair = c.fix(index, value);
      new_c = down_cast<aterm_proto_cflobdd>(fixed_pair.first());

      // Calculate the new value mapping accordingly
      const std::vector<aterm>& cvs_vec = as_vector(down_cast<aterm_list>((*this)[1]));
      const aterm_list& fixed_values = down_cast<aterm_list>(fixed_pair.second());
      for (const aterm& fixed_value : fixed_values)
      {
        // Get the proto-CFLOBDD and result mapping corresponding to the fixed value
        const size_t& fixed_index = down_cast<aterm_int>(fixed_value).value();
        const aterm_pair& fixed_cv = down_cast<aterm_pair>(cvs_vec[fixed_index]);
        const aterm_list& fixed_vs = down_cast<aterm_list>(fixed_cv.second());

        // Add the result mapping value if it is not included yet
        // Set the return value for the proto-CFLOBDD to the index of the value
        std::vector<aterm_int> fixed_v_vec;
        for (const aterm& fixed_v : fixed_vs)
        {
          const aterm_int& v = down_cast<aterm_int>(fixed_v);
          const std::vector<aterm_int>::iterator& value_location = std::find(new_values_vec.begin(), new_values_vec.end(), v);
          const size_t& value_index = value_location - new_values_vec.begin();
          if (value_location == new_values_vec.end()) new_values_vec.push_back(v);
          fixed_v_vec.push_back(aterm_int(value_index));
        }
        new_cvs_vec.push_back(aterm_pair(
          down_cast<aterm_proto_cflobdd>(fixed_cv.first()),
          aterm_list(fixed_v_vec.begin(), fixed_v_vec.end())
        ));
      }
    }
    else
    {
      // The proposition letter is in the right split, so recurse there
      std::vector<aterm_int> reduce_values_vec;
      const size_t& new_index = index - mid_index;
      for (const aterm& cv : cvs)
      {
        // Cast the current data
        const aterm_pair& cv_pair = down_cast<aterm_pair>(cv);
        const aterm_proto_cflobdd& old_c = down_cast<aterm_proto_cflobdd>(cv_pair.first());
        const aterm_list& old_vs = down_cast<aterm_list>(cv_pair.second());
        const std::vector<aterm>& old_vs_vec = as_vector(old_vs);

        // Fix the proposition letter assignment in the old proto-CFLOBDD
        const aterm_pair& fixed_pair = old_c.fix(new_index, value);
        const aterm_list& fixed_values = down_cast<aterm_list>(fixed_pair.second());

        // Calculate the corresponding result mapping 
        std::vector<aterm_int> fixed_v_vec;
        for (const aterm& fixed_value : fixed_values)
        {
          const size_t& fixed_value_index = down_cast<aterm_int>(fixed_value).value();
          const aterm_int& value_mapped = down_cast<aterm_int>(old_vs_vec[fixed_value_index]);
          const std::vector<aterm_int>::iterator& value_location = std::find(new_values_vec.begin(), new_values_vec.end(), value_mapped);
          const size_t& value_index = value_location - new_values_vec.begin();
          if (value_location == new_values_vec.end()) new_values_vec.push_back(value_mapped);
          fixed_v_vec.push_back(aterm_int(value_index));
        }

        // Add the proto-CFLOBDD and result mapping pair if it is not included yet
        const aterm_pair& new_cv = aterm_pair(
          down_cast<aterm_proto_cflobdd>(fixed_pair.first()),
          aterm_list(fixed_v_vec.begin(), fixed_v_vec.end())
        );
        const std::vector<aterm_pair>::iterator& cv_location = std::find(new_cvs_vec.begin(), new_cvs_vec.end(), new_cv);
        const size_t& cv_index = cv_location - new_cvs_vec.begin();
        if (std::find(new_cvs_vec.begin(), new_cvs_vec.end(), new_cv) == new_cvs_vec.end()) new_cvs_vec.push_back(new_cv);
        reduce_values_vec.push_back(aterm_int(cv_index));
      }

      // Reduce the old c
      new_c = new_c.reduce(aterm_list(reduce_values_vec.begin(), reduce_values_vec.end()));
    }

    return aterm_pair(
      aterm_proto_cflobdd(new_c, aterm_list(new_cvs_vec.begin(), new_cvs_vec.end())),
      aterm_list(new_values_vec.begin(), new_values_vec.end())
    );
  }
};

/// \brief Constant proto-CFLOBDD I.
class aterm_proto_cflobdd_i: public aterm_proto_cflobdd
{
public:
  aterm_proto_cflobdd_i()
    : aterm_proto_cflobdd(g_proto_cflobdd_i)
  {}
};

/// \brief Constant proto-CFLOBDD V.
class aterm_proto_cflobdd_v: public aterm_proto_cflobdd
{
public:
  aterm_proto_cflobdd_v()
    : aterm_proto_cflobdd(g_proto_cflobdd_v)
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

/// \brief Calculate the product on a list of proto-CFLOBDDs.
/// \param c_list The list of proto-CFLOBDDs
/// \return The product
aterm_pair product(const aterm_list& c_list) noexcept
{
  assert(c_list.size());

  // Check if the product has already been evaluated
  static std::unordered_map<aterm_list, aterm_pair> cache;
  std::unordered_map<aterm_list, aterm_pair>::const_iterator cached_product = cache.find(c_list);
  if (cached_product != cache.end()) return cached_product->second;

  const size_t& level = down_cast<aterm_proto_cflobdd>(c_list[0]).level();
  const aterm_proto_cflobdd& no_distinction = aterm_proto_cflobdd(level);
  const aterm_proto_cflobdd& v = aterm_proto_cflobdd_v();
  bool all_no_distinction = true;

  for (const aterm& c : c_list)
  {
    if (c == v)
    {
      // Return V if at least one proto-CFLOBDD in the list is V
      aterm_list values_1;
      aterm_list values_2;

      for (reverse_term_list_iterator i = c_list.rbegin(); i != c_list.rend(); ++i)
      {
        // Out degree is either 0 for I or 1 for V
        const size_t& out_degree = down_cast<aterm_proto_cflobdd>(*i).out_degree();
        values_1.push_front(aterm_int(0));
        values_2.push_front(aterm_int(out_degree));
      }

      const aterm_pair& product = aterm_pair(v, aterm_list({values_1, values_2}));
      cache[c_list] = product;
      return product;
    }

    if (c != no_distinction)
    {
      // We encountered something else than a no-distinction proto-CFLOBDD, so the corresponding case no longer applies
      // The V case is also guaranteed to not apply, since the current proto-CFLOBDD would be V if this is reached on level 0
      all_no_distinction = false;
      break;
    }
  }

  if (all_no_distinction)
  {
    // Return the no-distinction proto-CFLOBDD if all are no-distinction
    aterm_list values;
    for (const aterm& term : c_list)
    {
      values.push_front(aterm_int(0));
    }

    const aterm_pair& product = aterm_pair(v, aterm_list({values}));
    cache[c_list] = product;
    return product;
  }

  // @TODO: continue here - just finished base cases

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

  // Construct, cache, and return the new proto-CFLOBDD and value pairs
  const aterm_pair& pair_product = aterm_pair(
    aterm_proto_cflobdd(entree_proto_cflobdd, aterm_list(cvs.begin(), cvs.end())),
    aterm_list(value_pairs.begin(), value_pairs.end())
  );
  cache[aterm_pair(*this, other)] = pair_product;
  return pair_product;
}

} // namespace atermpp

#endif // MCRL2_ATERMPP_ATERM_PROTO_CFLOBBD_H
