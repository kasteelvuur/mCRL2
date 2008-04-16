// Author(s): Jeroen van der Wulp
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file build_system.h
/// \brief Add your file description here.

#ifndef BUILD_SYSTEM_H__
#define BUILD_SYSTEM_H__

#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

#include "tipi/detail/utility/generic_visitor.hpp"

#include "tipi/tool.hpp"
#include "tipi/mime_type.hpp"

namespace squadt {
  class settings_manager;
  class tool_manager;
  class tool_manager_impl;
  class type_registry;
  class tool;

  namespace execution {
    class executor;
    class executor_impl;
  }

  using execution::executor;
  using execution::executor_impl;

  /**
   * \brief Container class for main components
   **/
  class build_system : public utility::visitable, private boost::noncopyable {
    template < typename R, typename S >
    friend class utility::visitor;

    private:

      /** \brief Central storage component for configuration settings (paths & constants) */
      std::auto_ptr < settings_manager > m_settings_manager;

      /** \brief Central tool manager component */
      std::auto_ptr < tool_manager >     m_tool_manager;

      /** \brief Central executor component */
      std::auto_ptr < executor >         m_executor;

      /** \brief Central type registry for runtime mime-type/format/command information */
      std::auto_ptr < type_registry >    m_type_registry;

    private:

      /** \brief Constructor with complete initialisation */
      build_system(std::auto_ptr < settings_manager >,
                   std::auto_ptr < tool_manager >,
                   std::auto_ptr < executor >,
                   std::auto_ptr < type_registry >);

      /** \brief Gets tool manager implementation object */
      tool_manager_impl const* get_tool_manager_impl() const;

      /** \brief Gets tool manager implementation object */
      tool_manager_impl* get_tool_manager_impl();

      /** \brief Gets executor implementation object */
      executor_impl const* get_executor_impl() const;

      /** \brief Gets executor implementation object */
      executor_impl* get_executor_impl();

      /** \brief Resets tool manager such that it uses the default tool collection */
      void default_tool_collection(tool_manager&) const;

    public:

      /** \brief Follows the syntax defined in RFC 2045 */
      typedef tipi::mime_type         mime_type;

      /** \brief Convenience type alias */
      typedef tipi::mime_type         storage_format;

      /** \brief Convenience type alias */
      typedef tipi::tool::category    tool_category;

    public:

      /** \brief Default constructor */
      build_system();

      /** \brief Constructor with complete initialisation */
      void initialise(std::auto_ptr < settings_manager >,
                      std::auto_ptr < tool_manager >,
                      std::auto_ptr < executor >,
                      std::auto_ptr < type_registry >);

      /** \brief Factory function for object creation */
      static std::auto_ptr < build_system > create(std::auto_ptr < settings_manager > s,
                                                      std::auto_ptr < tool_manager > t,
                                                      std::auto_ptr < executor > e,
                                                      std::auto_ptr < type_registry > r);


      /** \brief Gets settings manager object */
      settings_manager const& get_settings_manager() const;

      /** \brief Gets settings manager object */
      settings_manager& get_settings_manager();

      /** \brief Sets settings manager object */
      void set_settings_manager(std::auto_ptr < settings_manager >);

      /** \brief Gets tool manager object */
      tool_manager const& get_tool_manager() const;

      /** \brief Gets tool manager object */
      tool_manager& get_tool_manager();

      /** \brief Sets tool manager object */
      void set_tool_manager(std::auto_ptr< tool_manager >);

      /** \brief Gets tool manager object */
      executor const& get_executor() const;

      /** \brief Gets tool manager object */
      executor& get_executor();

      /** \brief Sets tool manager object */
      void set_executor(std::auto_ptr< executor >);

      /** \brief Gets type registry object */
      type_registry const& get_type_registry() const;

      /** \brief Gets type registry object */
      type_registry& get_type_registry();

      /** \brief Sets type registry object */
      void set_type_registry(std::auto_ptr< type_registry >);

      /** \brief Restores the state of this object from file (and those contained) */
      void restore();

      /** \brief Stores the state of this object (and those contained) */
      void store();

      /** \brief Stores user preferences and terminates running tools */
      void shutdown();
  };

  extern build_system global_build_system;
}

#endif

