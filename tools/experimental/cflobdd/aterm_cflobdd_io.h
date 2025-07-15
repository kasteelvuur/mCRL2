// Author(s): Richard Farla
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aterm_cflobdd_io.h


#ifndef MCRL2_ATERMPP_ATERM_CFLOBDD_IO_H
#define MCRL2_ATERMPP_ATERM_CFLOBDD_IO_H


#include "aterm_cflobdd.h"
#include "mcrl2/atermpp/aterm_io.h"


namespace atermpp
{

/// \brief Reads an aterm_cflobdd from a string. The string can be in either binary or text format.
///   Assumes alphanumeric proposition variables and operator precedence from high to low as: !, &&, ||, =>, <=>.
/// @param s The string to read from.
/// \return The term corresponding to the string.
aterm_cflobdd read_cflobdd_from_string(const std::string& s);

/// \brief Reads an aterm_cflobdd from a string. The string can be in either binary or text format.
///   Assumes alphanumeric proposition variables and operator precedence from high to low as: !, &&, ||, =>, <=>.
/// \param s The string to read from.
/// \param variables The variables of the cflobdd in order.
/// \return The term corresponding to the string.
aterm_cflobdd read_cflobdd_from_string(const std::string& s, const std::vector<std::string>& variables);

/// \brief Reads an aterm_cflobdd from a string. The string can be in either binary or text format.
///   Assumes alphanumeric proposition variables and operator precedence from high to low as: !, &&, ||, =>, <=>.
/// \param s The string to read from.
/// \param variables The variables names mapped to the corresponding cflobdds.
/// \return The term corresponding to the string.
aterm_cflobdd read_cflobdd_from_string(const std::string& s, const std::unordered_map<std::string, aterm_cflobdd>& variables);

/// \brief Reads CFLOBDD terms in textual format from an input stream.
class text_aterm_cflobdd_istream final : public aterm_istream
{

/// \brief Shortcut runtime error for unexpected character
class unexpected_character_error : public std::runtime_error {
public:
  unexpected_character_error(char character)
    : std::runtime_error(std::string("Unexpected character '") + character + "' while parsing a CFLOBDD term")
  {}
};

public:
  text_aterm_cflobdd_istream(std::istream& os, const std::vector<std::string>& variables);
  text_aterm_cflobdd_istream(std::istream& os, const std::unordered_map<std::string, aterm_cflobdd>& variables);

  void get(aterm& t) override;

private:
  /// \brief Parses a proposition formula as an aterm_cflobdd.
  aterm_cflobdd parse(int& character);

  /// \brief Parses a proposition formula centered around a biconditional as an aterm_cflobdd.
  aterm_cflobdd parse_biconditional(int& character);

  /// \brief Parses a proposition formula centered around an implication as an aterm_cflobdd.
  aterm_cflobdd parse_implication(int& character);

  /// \brief Parses a proposition formula centered around a disjunction as an aterm_cflobdd.
  aterm_cflobdd parse_disjunction(int& character);

  /// \brief Parses a proposition formula centered around a conjunction as an aterm_cflobdd.
  aterm_cflobdd parse_conjunction(int& character);

  /// \brief Parses a proposition formula centered around a negation as an aterm_cflobdd.
  aterm_cflobdd parse_negation(int& character);

  /// \brief Parses a singular proposition letter or proposition formula within parentheses as an aterm_cflobdd.
  aterm_cflobdd parse_primary(int& character);

  /// \returns A string indicating the parse error position.
  std::string print_parse_error_position();

  /// \returns The first character that is not whitespace or end-of-file (EOF).
  /// \param skip_whitespace, returns the next non space character.
  /// \param required Throw error when the next character is EOL.
  int next_char(bool skip_whitespace = true, bool required = false);

  std::istream& m_stream;
  std::unordered_map<std::string, aterm_cflobdd> m_variables;
  const std::size_t m_level;

  std::size_t m_line = 0; ///< The line number of the current character.
  std::size_t m_column = 0; ///< The column of the current character.

  std::size_t m_history_limit = 64; ///< Determines the maximum number of characters that are stored.
  std::deque<char> m_history; ///< Stores the characters that have been read so-far.

  int character; ///< The last character that was read.
};

} // namespace atermpp

#endif // MCRL2_ATERMPP_ATERM_CFLOBDD_IO_H
