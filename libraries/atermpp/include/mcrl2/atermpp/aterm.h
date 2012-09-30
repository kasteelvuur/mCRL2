// Author(s): Wieger Wesselink, Jan Friso Groote. Based on the ATerm library by Paul Klint and others.
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/atermpp/aterm.h
/// \brief The aterm class.

#ifndef MCRL2_ATERMPP_ATERM_H
#define MCRL2_ATERMPP_ATERM_H

#include <string>
#include <iostream>
#include <cassert>
#include <vector>
#include <assert.h>

#include "boost/type_traits/is_base_of.hpp"
#include "boost/type_traits/is_convertible.hpp"
#include "boost/static_assert.hpp"
#include "mcrl2/atermpp/detail/function_symbol_implementation.h"
#include "mcrl2/atermpp/detail/aterm.h"

/// \brief The main namespace for the aterm++ library.
namespace atermpp
{

class aterm
{
  public:
    template < typename T >
    friend class term_appl; 

    template < typename T >
    friend class term_list; 
    
  protected:
    detail::_aterm *m_term;

    static detail::_aterm *undefined_aterm();
    static detail::_aterm *empty_aterm_list();
 
    void free_term();

    void decrease_reference_count()
    {
      assert(m_term!=NULL);
      assert(m_term->reference_count()>0);
      if (0== --m_term->reference_count())
      {
        free_term();
        return;
      }
    }

    template <bool CHECK>
    static void increase_reference_count(detail::_aterm* t)
    {
      assert(t!=NULL);
      if (CHECK) assert(t->reference_count()>0);
      t->reference_count()++;
    }

    void copy_term(detail::_aterm* t)
    {
      increase_reference_count<true>(t);
      decrease_reference_count();
      m_term=t;
    }

  public: // Should be protected;
    /// \brief Constructor.
    /// \detail The function symbol must have arity 0. This function
    /// is for internal use only. Use term_appl(sym) in applications.
    /// \param sym A function symbol.
    aterm(const function_symbol &sym);
  
    // Functions below should become protected.
    detail::_aterm & operator *() const
    {
      assert(m_term!=NULL);
      assert(m_term->reference_count()>0);
      return *m_term;
    }

    detail::_aterm * operator ->() const
    {
      assert(m_term!=NULL && m_term->reference_count()>0);
      return m_term;
    }

    aterm (detail::_aterm *t):m_term(t)
    {
      // Note that reference_count can be 0, as this term can just be constructed,
      // and is now handed over to become a real aterm.
      assert(t!=NULL);
      increase_reference_count<false>(m_term);
    }

  public:

    /// \brief Default constructor
    aterm():m_term(undefined_aterm())
    {
      increase_reference_count<false>(m_term);
    }

    /// \brief Copy constructor
    aterm (const aterm &t):m_term(t.m_term)
    {
      assert(t.address()!=NULL);
      increase_reference_count<true>(m_term);
    }

    /// \brief Assignment operator.
    /// \param t a term to be assigned.
    aterm &operator=(const aterm &t)
    {
      copy_term(t.m_term);
      return *this;
    }

    /// \brief Destructor.
    ~aterm ()
    {
      decrease_reference_count();
    }

    /// \brief Returns the function symbol belonging to a term.
    /// \return The function symbol of this term.
    const function_symbol &function() const
    {
      return m_term->function();
    }

    /// \brief Returns the type of this term.
    /// Result is one of AT_APPL, AT_INT,
    /// or AT_LIST.
    /// \detail Often it is more efficient to use the utility functions
    ///      type_is_appl, type_is_int or type_is_list.
    /// \return The type of the term.
    size_t type() const
    {
      return m_term->type(); 
    }

    /// \brief Returns whether this term has type AT_APPL.
    /// \detail This function is more efficient than using
    ///   type()==AT_APPL.
    /// \return True iff term is an term_appl.
    bool type_is_appl() const
    {
      return m_term->function().number()>detail::function_adm.AS_EMPTY_LIST.number(); 
    }

    /// \brief Returns whether this term has type AT_INT
    /// \detail This function is more efficient than using
    ///   type()==AT_INT.
    /// \return True iff term is an term_int.
    bool type_is_int() const
    {
      return m_term->function().number()==detail::function_adm.AS_INT.number(); 
    }

    /// \brief Returns whether this term has type AT_LIST
    /// \detail This function is more efficient than using
    ///   type()==AT_LIST.
    /// \return True iff term is an term_list.
    bool type_is_list() const
    {
      const size_t n=m_term->function().number();
      return n==detail::function_adm.AS_LIST.number()|| n==detail::function_adm.AS_EMPTY_LIST.number(); 
    }


    /// \brief Writes the term to a string.
    /// \return A string representation of the term.
    std::string to_string() const;

    /// \brief Equality function on two aterms.
    /// \detail Terms are stored in a maximally shared way. This
    ///         means that this equality operator can be calculated
    ///         in constant time.
    /// \param t A term to which the current term is compared.
    /// \return true iff t is equal to the current term.
    bool operator ==(const aterm &t) const
    {
      assert(m_term!=NULL && m_term->reference_count()>0);
      assert(t.m_term!=NULL && t.m_term->reference_count()>0);
      return m_term==t.m_term;
    }

    /// \brief Inequality operator on two aterms.
    /// \detail See note at the == operator. This operator requires constant time.
    /// \param t A term to which the current term is compared.
    /// \return false iff t is equal to the current term.
    bool operator !=(const aterm &t) const
    {
      assert(m_term!=NULL && m_term->reference_count()>0);
      assert(t.m_term!=NULL && t.m_term->reference_count()>0);
      return m_term!=t.m_term;
    }

    /// \brief Comparison operator for two aterms.
    /// \detail This operator requires constant time. It compares
    ///         the addresses where terms are stored. That means
    ///         that the outcome of this operator is only stable
    ///         as long as aterms are not garbage collected.
    /// \param t A term to which the current term is compared.
    /// \return True iff the current term is smaller than the argument.
    bool operator <(const aterm &t) const
    {
      assert(m_term!=NULL && m_term->reference_count()>0);
      assert(t.m_term!=NULL && t.m_term->reference_count()>0);
      return m_term<t.m_term;
    }

    /// \brief Comparison operator for two aterms.
    /// \detail This operator requires constant time. See note at the operator <.
    /// \param t A term to which the current term is compared.
    /// \return True iff the current term is larger than the argument.
    bool operator >(const aterm &t) const
    {
      assert(m_term!=NULL && m_term->reference_count()>0);
      assert(t.m_term!=NULL && t.m_term->reference_count()>0);
      return m_term>t.m_term;
    }

    /// \brief Comparison operator for two aterms.
    /// \detail This operator requires constant time. See note at the operator <.
    /// \param t A term to which the current term is compared.
    /// \return True iff the current term is smaller or equal than the argument.
    bool operator <=(const aterm &t) const
    {
      assert(m_term!=NULL && m_term->reference_count()>0);
      assert(t.m_term!=NULL && t.m_term->reference_count()>0);
      return m_term<=t.m_term;
    }

    /// \brief Comparison operator for two aterms.
    /// \detail This operator requires constant time. See note at the operator <.
    /// \param t A term to which the current term is compared.
    /// \return True iff the current term is larger or equalthan the argument.
    bool operator >=(const aterm &t) const
    {
      assert(m_term!=NULL && m_term->reference_count()>0);
      assert(t.m_term!=NULL && t.m_term->reference_count()>0);
      return m_term>=t.m_term;
    }

    /// \brief Provide the current address of this aterm.
    /// \details This address will be stable as long as this aterm
    ///          exists, i.e., has a reference count larger than 0.
    /// \return A void* pointer, representing the machine address of the current aterm.
    void * address() const
    {
      assert(m_term!=NULL && m_term->reference_count()>0);
      return &* *this;
    }

    /// \brief Returns true if this term is not equal to the term assigned by
    //         the default constructor, i.e. *this!=aterm().
    /// \details This operation is more efficient than comparing the current
    ///          term with an aterm().
    /// \return A boolean indicating whether this term equals the default constructor.
    bool defined() const
    {
      assert(m_term!=NULL && m_term->reference_count()>0);
      return this->function().number()!=detail::function_adm.AS_DEFAULT.number();
    }

};

/// \brief A cheap cast from one aterm based type to another
/// \details When casting one aterm based type into another, generally
///          a new aterm is constructed, and the old one is destroyed.
///          This can cause undesired overhead, for instance due to
///          increasing and decreasing of reference counts. This
///          cast changes the type, without changing the aterm itself.
///          It can only be used if the input and output types inherit
///          from aterms, and contain no additional information than a
///          single aterm.
/// \param   A term of a type inheriting from an aterm.
/// \return  A term of type ATERM_TYPE_OUT.
template <class ATERM_TYPE_OUT>
const ATERM_TYPE_OUT &aterm_cast(const aterm &t) 
{
  BOOST_STATIC_ASSERT((boost::is_base_of<aterm, ATERM_TYPE_OUT>::value));
  BOOST_STATIC_ASSERT((sizeof(ATERM_TYPE_OUT)==sizeof(aterm)));
  return (ATERM_TYPE_OUT &)t;
}

inline
std::ostream& operator<<(std::ostream& out, const aterm& t)
{
  return out << t.to_string();
}
	
} // namespace atermpp

#endif // MCRL2_ATERMPP_ATERM_H
