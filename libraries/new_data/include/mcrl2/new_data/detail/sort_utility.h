// Author(s): Jeroen Keiren, Jan Friso Groote
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/new_data/detail/sort_utility.h
/// \brief Utility functions for sorts.

#ifndef MCRL2_DATA_SORT_UTILITY_H
#define MCRL2_DATA_SORT_UTILITY_H

#include <vector>
#include <cassert>
#include "mcrl2/new_data/data.h"
#include "mcrl2/new_data/data_specification.h"

namespace mcrl2 {

  namespace new_data {
    namespace detail {

      /// Test is a term is a sort, and if it is equal to s
      struct compare_sort
      {
        sort_expression s;

        compare_sort(sort_expression s_)
          : s(s_)
        {}

        bool operator()(atermpp::aterm_appl t) const
        {
          return s == t;
        }
      };

      ///\pre l is a list type of some sort (e.g. sort_expression_list)
      ///\return sort s occurs in l.
      template <typename list_type>
      bool occurs_in(list_type l, sort_expression s)
      {
        return atermpp::find_if(l, compare_sort(s)) != atermpp::aterm();
      }

      ///\return the list of all functions f of sort s in fl
      inline function_symbol_list get_constructors(const function_symbol_list& fl, const sort_expression& s)
      {
        function_symbol_list result;
        for(function_symbol_list::iterator i = fl.begin(); i != fl.end(); ++i)
        {
          if (result_sort(i->sort()) == s)
          {
            result = push_front(result, *i);
          }
        }
        atermpp::reverse(result);
        return result;
      }

      ///\return true if f has 1 or more arguments, false otherwise
      inline bool has_arguments(const function_symbol& f)
      {
        return !core::detail::gsIsSortId(atermpp::aterm_appl(f.argument(1)));
      }

      ///\return true if all sorts in sl are finite, false otherwise
      ///Note that when a constructor sort is in visited we hold the sort as infinite because loops are created!
      inline bool is_finite(const function_symbol_list& fl, const mcrl2::new_data::sort_expression_list& sl, const mcrl2::new_data::sort_expression_list visited = mcrl2::new_data::sort_expression_list())
      {
        bool result = true;

        // A list of sorts is finite if all sorts in the list are finite
        // If a sort is in "visited" that means that we have already seen the sort
        // during our calculation. We now get loops of the sort D = d1(E), sort E=e1(D),
        // this makes our sort infinite.
        for (sort_expression_list::iterator i = sl.begin(); i != sl.end(); ++i)
        {
          if (!occurs_in(visited, *i))
          {
            result = result && is_finite(fl, *i, visited);
          }
          else
          {
            result = false;
          }
        }
        return result;
      }

      ///\pre fl is a list of constructors
      ///\return sort s is finite
      inline bool is_finite(const function_symbol_list& fl, const sort_expression& s, const mcrl2::new_data::sort_expression_list visited = mcrl2::new_data::sort_expression_list())
      {
        bool result = true;
        function_symbol_list cl = get_constructors(fl, s);

        //If a sort has not got any constructors it is infinite
        if (cl.empty())
        {
          result = false;
        }

        //Otherwise a sort is finite if all its constructors are finite;
        //i.e. the constructors have no arguments, or their arguments are finite.
        //In the recursive call pass s add s to the visited sorts, so that we know
        //it may not occur in a constructor anymore.
        for (function_symbol_list::iterator i = cl.begin(); i != cl.end(); ++i)
        {
          result = result && (!(has_arguments(*i)) || is_finite(fl, domain_sorts(i->sort()), push_front(visited, s)));
        }

        return result;
      }
            ///\pre s is a sort that occurs in data_specfication data
      ///\ret true iff there exists a constructor function with s as target sort
      inline bool is_constructorsort(const sort_expression &s,const data_specification &data)
      { // This function is added by Jan Friso Groote on 8/7/2007.
        // cl contains all constructors with target sort s.

        if (s.is_arrow())
        { return false;
        }

        assert(occurs_in(data.sorts(),s));
        return (!data.constructors(s).empty());
      }

      /// Undocumented function.
      inline
      new_data::sort_expression_list get_sorts(new_data::variable_list v)
      {
        new_data::sort_expression_list result;
        for (new_data::variable_list::iterator i = v.begin(); i != v.end(); i++)
        {
                result = push_back(result, i->sort());
        }
        return reverse(result);
      }

      /// Undocumented function.
      inline
      new_data::sort_expression_list get_sorts(new_data::data_expression_list v)
      {
        new_data::sort_expression_list result;
        for (new_data::data_expression_list::iterator i = v.begin(); i != v.end(); i++)
        {
                result = push_back(result, i->sort());
        }
        return reverse(result);
      }

      /// Undocumented function.
      inline
      bool check_finite(new_data::function_symbol_list fl, new_data::sort_expression s)
      {
        return is_finite(fl, s);
      }

      /// Undocumented function.
      inline
      bool check_finite_list(new_data::function_symbol_list fl, new_data::sort_expression_list s)
      {
        return is_finite(fl, s);
      }

      /// Undocumented function.
      inline
      new_data::data_expression_list create_data_expression_list(new_data::function_symbol f, std::vector< new_data::data_expression_list > dess)
      {
        // Result list
        new_data::data_expression_list result;
        // At first put function f in result
        result = push_front(result, new_data::data_expression(f));
        for (std::vector< new_data::data_expression_list >::iterator i = dess.begin(); i != dess.end(); i++)
        {
                //*i is a list of constructor expressions that should be applied to the elements of result
                new_data::data_expression_list tmp;
                for (new_data::data_expression_list::iterator k = i->begin(); k != i->end(); k++)
                //*k is a constructor expression that should be applied to the elements of result
                {
                        for (new_data::data_expression_list::iterator j = result.begin(); j != result.end(); j++)
                        {
                                // *j is a data expression
                                //  apply *j to *k
                                //  One of the constructor values is applied to f
                                      new_data::data_expression_list l;
                                      l = push_front(l, *k);
                                tmp = push_front(tmp, new_data::data_expression(data_application(*j, l)));
                        }
                }
                // Next iteration replace all values which are created in tmp
                result = tmp;
        }
        return result;
      }

      /// Undocumented function.
      inline
      new_data::data_expression_list enumerate_constructors(new_data::function_symbol_list fl, new_data::sort_expression s)
      {
        // All datasorts which are taken into account must be finite. Normally this is the case, because a check on finiteness is done in create_bes
        assert(is_finite(fl, s));
        // The resulting new_data::data_expression_list.
        new_data::data_expression_list ces;
        // Get all constructors of sort s
        new_data::function_symbol_list cl = get_constructors(fl,s);
        // For each constructor of sort s...
        for (new_data::function_symbol_list::iterator i = cl.begin(); i != cl.end(); i++)
        {
                // Get all domains of the constructor
                new_data::sort_expression_list domains = domain_sorts(i->sort());
                // Vector for all enumerated constructors
                std::vector< new_data::data_expression_list > argumentss;
                // For each sort of the constructor...
                for (new_data::sort_expression_list::iterator j = domains.begin(); j != domains.end(); j++)
                {
                        // Put all values which the sort can have in a list
                        argumentss.push_back(enumerate_constructors(fl,*j));
                }
                // Create data_expression_list out of the values which a sort can have
                new_data::data_expression_list temp = create_data_expression_list(*i, argumentss);
                //concatenate ces and temp
                for (new_data::data_expression_list::iterator k = temp.begin(); k != temp.end(); k++)
                {
                        ces = push_front(ces, *k);
                }
        }
        // Put ces in the correct order
        ces = reverse(ces);
        return ces;
      }
   }

  } // namespace new_data

} // namespace mcrl2

#endif // MCRL2_DATA_SORT_UTILITY_H
