// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/function_symbol.h
/// \brief The class function symbol.

#ifndef MCRL2_DATA_FUNCTION_SYMBOL_H
#define MCRL2_DATA_FUNCTION_SYMBOL_H

#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/atermpp/aterm_list.h"
#include "mcrl2/atermpp/aterm_traits.h"
#include "mcrl2/atermpp/vector.h"
#include "mcrl2/core/detail/constructors.h"
#include "mcrl2/data/data_expression.h"
#include "mcrl2/data/application.h"
#include "mcrl2/data/sort_expression.h"

namespace mcrl2 {
  
  namespace data {

    /// \brief function symbol.
    ///
    class function_symbol: public data_expression
    {
      public:

        /// Constructor.
        ///
        function_symbol()
          : data_expression(core::detail::constructOpId())
        {}

        /// \internal
        function_symbol(const atermpp::aterm_appl& a)
          : data_expression(a)
        {}

        /// \brief Constructor.
        ///
        /// \param[in] d A data expression.
        /// \pre d is a function symbol.
        function_symbol(const data_expression& d)
          : data_expression(d)
        {
          assert(d.is_function_symbol());
        }

        /// \brief Constructor.
        ///
        /// \param[in] name The name of the function.
        /// \param[in] sort The sort of the function.
        function_symbol(const std::string& name,
                        const sort_expression& sort)
          : data_expression(core::detail::gsMakeOpId(atermpp::aterm_string(name), sort))
        {}

        /// \brief Constructur.
        ///
        /// \param[in] name The name of the function.
        /// \param[in] sort The sort of the function.
        function_symbol(const core::identifier_string& name,
                        const sort_expression& sort)
          : data_expression(core::detail::gsMakeOpId(name, sort))
        {}

        /// \brief Returns the application of this function symbol to an argument.
        /// \pre this->sort() is a function sort.
        /// \param[in] e The data expression to which the function symbol is applied
        application operator()(const data_expression& e) const
        {
          assert(this->sort().is_function_sort());
          return application(*this, e);
        }

        /// \brief Returns the name of the variable.
        inline
        std::string name() const
        {
          return atermpp::aterm_string(atermpp::arg1(*this));
        }

        /// \brief Returns true iff this function symbol is a number.
        inline
        bool is_number() const
        {
          return core::detail::gsIsNumericString(name().c_str());
        }

        /// \brief Returns the sort of the variable.
        inline
        sort_expression get_sort_expression() const
        {
          return sort_expression( atermpp::arg2(*this) );
        }

    }; // class function_symbol

    /// \brief list of function symbols
    ///
    typedef atermpp::vector<function_symbol> function_symbol_list;

  } // namespace data

} // namespace mcrl2

/// \cond INTERNAL_DOCS
MCRL2_ATERM_TRAITS_SPECIALIZATION(mcrl2::data::function_symbol)
/// \endcond

#endif // MCRL2_DATA_FUNCTION_SYMBOL_H

