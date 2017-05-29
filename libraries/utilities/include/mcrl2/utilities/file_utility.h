// Author(s): Sjoerd Cranen
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/utilities/file_utility.h
/// \brief add your file description here.

#ifndef MCRL2_UTILITIES_FILE_UTILITY_H
#define MCRL2_UTILITIES_FILE_UTILITY_H

#include "mcrl2/utilities/exception.h"
#include "mcrl2/utilities/logger.h"
#include <ctime>
#include <fstream>
#include <sstream>
#include <vector>

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif

namespace mcrl2
{

namespace utilities
{

/// \brief Returns true if the given filename has the extension ext. The extension does not start with a dot. 
inline
bool has_extension(const std::string& filename, const std::string& extension)
{
  assert(extension.size()>=1 && extension[0]!='.');  // The extension should not start with a dot and consist out of at least one character.
  std::string dotted_extension="."+extension;
  if (filename.size()<dotted_extension.size())
  {
    return false;
  }
  const std::string filename_extension = filename.substr(filename.size()-dotted_extension.size());
  return filename_extension==dotted_extension;
}

#ifdef WIN32
inline
void set_binary_mode(std::ios& stream)
{
  std::string name;
  FILE* handle;
  if (stream.rdbuf() == std::cin.rdbuf())
  {
    name = "cin";
    handle = stdin;
  }
  else
  if (stream.rdbuf() == std::cout.rdbuf())
  {
    name = "cout";
    handle = stdout;
    fflush(stdout);
  }
  else
  if (stream.rdbuf() == std::cerr.rdbuf())
  {
    name = "cerr";
    handle = stderr;
    fflush(stderr);
  }
  if (!name.empty())
  {
    if (_setmode(_fileno(handle), _O_BINARY) == -1)
    {
      mCRL2log(mcrl2::log::warning) << "Cannot set " << name << " to binary mode.\n";
    }
    else
    {
      mCRL2log(mcrl2::log::debug) << "Converted " << name << " to binary mode.\n";
    }
  }
}
#endif

template <class stream_type, class filestream_type>
class stream_wrapper
{
private:
  bool m_stdio;
  stream_type* m_stream;
  stream_wrapper(const stream_wrapper&) {}
public:
  stream_wrapper(stream_wrapper&& other)
    : m_stdio(other.m_stdio), m_stream(other.m_stream)
  {
    other.m_stream = nullptr;
  }

  stream_wrapper(const std::string& filename, bool text, stream_type* stdstream)
  {
    m_stdio = filename.empty() || filename == "-";
    if (m_stdio)
    {
      m_stream = stdstream;
#ifdef WIN32
      set_binary_mode(*m_stream);
#endif
    }
    else
    {
      if (text)
      {
        m_stream = new filestream_type(filename);
      }
      else
      {
        m_stream = new filestream_type(filename, std::ios_base::binary);
      }
      if (!m_stream->good())
      {
        throw mcrl2::runtime_error("Could not open file " + filename);
      }
    }
  }

  virtual ~stream_wrapper()
  {
    if (!m_stdio)
    {
      delete m_stream;
    }
  }

  stream_type& stream() const
  {
    return *m_stream;
  }
};

typedef stream_wrapper<std::ostream, std::ofstream> output_file;
typedef stream_wrapper<std::istream, std::ifstream> input_file;

class file_format
{
private:
  std::string m_shortname;
  std::string m_description;
  bool m_textformat;
  std::vector<std::string> m_extensions;
public:
  file_format(const std::string& shortname, const std::string& description, bool is_text_format)
    : m_shortname(shortname), m_description(description), m_textformat(is_text_format)
  { }

  void add_extension(const std::string& ext)
  {
    m_extensions.push_back(ext);
  }

  const std::string& shortname() const { return m_shortname; }

  const std::string& description() const { return m_description; }

  input_file open_input(const std::string& filename) const
  {
    return input_file(filename, m_textformat, &std::cin);
  }

  output_file open_output(const std::string& filename) const
  {
    return output_file(filename, m_textformat, &std::cout);
  }

  bool matches(const std::string& filename) const
  {
    for (const std::string& ext: m_extensions)
    {
      if (has_extension(filename,ext))
      {
        return true;
      }
    }
    return false;
  }

  static const file_format* unknown()
  {
    static file_format unknown("unknown", "Unknown format", false);
    return &unknown;
  }

  bool operator==(const file_format& other)
  {
    return m_shortname == other.m_shortname;
  }
};

inline
std::ostream& operator<<(std::ostream& stream, const file_format& format)
{
  return stream << format.shortname();
}

inline
std::ostream& operator<<(std::ostream& stream, const file_format* format)
{
  return stream << format->shortname();
}

inline
bool file_exists(const std::string& filename)
{
  if (FILE * file = fopen(filename.c_str(), "r"))
  {
    fclose(file);
    return true;
  }
  return false;
}

} // namespace utilities

} // namespace mcrl2

#endif // MCRL2_UTILITIES_FILE_UTILITY_H
