// Author(s): Richard Farla
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file cflobdd.cpp

#include <numeric>

#include "mcrl2/atermpp/aterm.h"


using namespace atermpp;
using namespace std;


class proto_cflobdd: public aterm
{
  private:
    const uint32_t level;
    const uint32_t out_degree;

  public:
    /**
     * Constant proto-CFLOBDD I or V
     * @param out_degree The out degree of this proto-CFLOBDD
     */
    proto_cflobdd(const uint32_t& out_degree)
      : aterm(function_symbol((out_degree == 1) ? "I" : "V", 0)), level(1), out_degree(out_degree)
    {
      // Out degree must be 1 or 2 for I or V respectively
      assert(1 <= out_degree <= 2);
    }

    /**
     * Proto-CFLOBDD inductive case (L, [L_0, ..., L_{n-1}], m)
     * @param c The proto-CFLOBDD L
     * @param cvs The list of proto-CFLOBDDs [L_0, ..., L_{n-1}] and mapping m merged into a vector of pairs.
     * Each proto-CFLOBDD L_i is paired with a vector of mapping result values v_i such that
     * L_i.out_degree = v_i.size() and v_i[j] = m(i,j).
     */
    proto_cflobdd(const proto_cflobdd& c, const vector<pair<proto_cflobdd, vector<uint32_t>>>& cvs):
      aterm(function_symbol("C", 2), c, cvs),
      level(c.level + 1),
      out_degree(std::accumulate(
        cvs.begin(),
        cvs.end(),
        0,
        [](const uint32_t& max, const pair<proto_cflobdd, vector<uint32_t>>& cv) {
          return std::max(max, *std::max_element(get<1>(cv).begin(), get<1>(cv).end()));
        }
      ))
    {
      // The out degree of the first proto-CFLOBDD must match the number of remaining proto-CFLOBDDs
      assert(c.out_degree == cvs.size());

      for (pair<proto_cflobdd, vector<uint32_t>> cv : cvs) {
        // All proto-CFLOBDs must have the same level
        assert(c.level == get<0>(cv).level);

        // The out degree of each proto-CFLOBDD L_i must match the amount of corresponding return values
        assert(get<0>(cv).out_degree == get<1>(cv).size());
      }

      // TODO: Actually store c and cvs in some way. Possibly in separate class?
    }
};

class proto_cflobdd_i: public proto_cflobdd
{
  proto_cflobdd_i()
    : proto_cflobdd(1)
  {}
};

class proto_cflobdd_v: public proto_cflobdd
{
  proto_cflobdd_v()
    : proto_cflobdd(2)
  {}
};

class proto_cflobdd_c: public proto_cflobdd
{
  proto_cflobdd_c(const proto_cflobdd& c, const vector<pair<proto_cflobdd, vector<uint32_t>>>& cvs)
    : proto_cflobdd(c, cvs)
  {}
};
