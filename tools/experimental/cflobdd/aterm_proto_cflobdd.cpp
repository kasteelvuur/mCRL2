// Author(s): Richard Farla
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aterm_proto_cflobdd.cpp


#include "aterm_pair.h"

#include "mcrl2/atermpp/aterm.h"
#include "mcrl2/atermpp/aterm_int.h"
#include "mcrl2/atermpp/aterm_list.h"


using namespace atermpp;


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

/// \brief A proto-CFLOBDD term encodes boolean functions.
class aterm_proto_cflobdd: public aterm
{
  public:
    /// \brief Construct a proto-CFLOBDD term from an aterm.
    /// \param term The aterm to cast
    aterm_proto_cflobdd(const aterm& term)
      : aterm(term)
    {
      assert(is_proto_cflobdd(term));
    }

    /// \brief Constant proto-CFLOBDD I or V.
    /// \param function_symbol The function symbol for I or V
    aterm_proto_cflobdd(const function_symbol& function_symbol)
      : aterm(function_symbol)
    {
      // Function symbol must be one of I or V
      assert(function_symbol == function_symbol_i() || function_symbol == function_symbol_v());
    }

    /// \brief Proto-CFLOBDD inductive case (L, [L_0, ..., L_{n-1}], m).
    /// \param c The proto-CFLOBDD L
    /// \param cvs The list of proto-CFLOBDDs [L_0, ..., L_{n-1}] and mapping m merged into a vector of pairs.
    ///   Each proto-CFLOBDD L_i is paired with a vector of mapping result values v_i such that
    ///   L_i.out_degree = v_i.size() and v_i[j] = m(i,j).
    aterm_proto_cflobdd(const aterm_proto_cflobdd& c, const aterm_list& cvs):
      aterm(function_symbol_c(), c, cvs)/**,
      level(aterm_int(c.level.value() + 1)),
      out_degree(std::accumulate(
        vss.begin(),
        vss.end(),
        0,
        [](const aterm& max, const aterm_list& vs) {
          return std::max(max, *std::max_element(vs.begin(), vs.end()));
        }
      ))*/
    {
      // The out degree of the first proto-CFLOBDD must match the number of remaining proto-CFLOBDDs
      // assert(c.out_degree.value() == cs.size() == vss.size());

      for (const aterm& cv : cvs) {
        const aterm_pair& x = down_cast<aterm_pair>(cv);
        // if (x.function() == function_symbol("C", 3)) {
        //   assert(x.level > 1);
        // }
        // assert(typeid(x) == typeid(aterm_proto_cflobdd));
        // // All proto-CFLOBDs must have the same level
        // assert(c.level == x.level);

        // // The out degree of each proto-CFLOBDD L_i must match the amount of corresponding return values
        // assert(get<0>(cv).out_degree.value() == get<1>(cv).size());
      }

      // TODO: Actually store c and cvs in some way. Possibly in separate class?
    }

    /// \brief Check if a term is a proto-CFLOBDD.
    /// \param term The term to check
    /// \return Whether the term is a proto-CFLOBDD
    size_t is_proto_cflobdd(const aterm& term) const noexcept
    {
      // TODO: Strict check on arguments as well?
      return term.function() == function_symbol_i()
        || term.function() == function_symbol_v()
        || term.function() == function_symbol_c();
    }

    /// \brief Get the level of the proto-CFLOBDD.
    /// \details The constant proto-CFLOBDDs I and V have level 1.
    ///   The inductive proto-CFLOBDD (L, [L_0, ..., L_{n-1}], m) is one level higher than its children.
    /// \return The level of the proto-CFLOBDD
    size_t level() const noexcept
    {
      if (this->function() == function_symbol_i() || this->function() == function_symbol_v())
      {
        // Constant proto-CFLOBDDs I and V have level 1
        return 1;
      }
      else if (this->function() == function_symbol_c())
      {
        // Inductive proto-CFLOBDD (L, [L_0, ..., L_{n-1}], m) is one level higher than its children
        const aterm_proto_cflobdd& l = down_cast<aterm_proto_cflobdd>((*this)[0]);
        return l.level() + 1;
      }
      else
      {
        // This case should never happen
        assert(false);
        return 0;
      }
    }

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
          const aterm_list& values = down_cast<aterm_list>(pair.right());
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
};

class aterm_proto_cflobdd_i: public aterm_proto_cflobdd
{
  public:
    aterm_proto_cflobdd_i()
      : aterm_proto_cflobdd(function_symbol_i())
    {}
};

class aterm_proto_cflobdd_v: public aterm_proto_cflobdd
{
  public:
    aterm_proto_cflobdd_v()
      : aterm_proto_cflobdd(function_symbol_v())
    {}
};

class aterm_proto_cflobdd_c: public aterm_proto_cflobdd
{
  public:
    aterm_proto_cflobdd_c(const aterm_proto_cflobdd& c, const aterm_list& cvs)
      : aterm_proto_cflobdd(c, cvs)
    {}
};

int main()
{
  return 0;
}
