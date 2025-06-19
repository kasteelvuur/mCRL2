// Author(s): Richard Farla
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file bdd_io.cpp


#include "bdd_io.h"


namespace oxidd
{

// Public functions

bdd_function read_bdd_from_string(const std::string& s)
{
  std::vector<std::string> variables;
  // @TODO: Extract variables from string
  return read_bdd_from_string(s, variables);
}

bdd_function read_bdd_from_string(const std::string& s, const std::vector<std::string>& variables)
{
  std::stringstream ss(s);
  bdd_function bdd;
  text_bdd_istream(ss, variables).get(bdd);
  return bdd;
}

text_bdd_istream::text_bdd_istream(std::istream& is, const std::vector<std::string>& variables)
  : m_stream(is)
{
  character = next_char();

  // Map the variables to their corresponding BDDs
  bdd_manager mgr(5 * std::pow(2, variables.size() / 2), 1024, 1);
  for (std::string variable : variables)
  {
    m_variables[variable] = mgr.new_var();
  }
}

void text_bdd_istream::get(bdd_function& bdd)
{
  try
  {
    if (character != EOF)
    {
      bdd = parse(character);
    }
  }
  catch (std::runtime_error& e)
  {
    throw std::runtime_error(e.what() + std::string("\n") + print_parse_error_position());
  }

  assert(character == EOF);

  // Reset the parsing error buffers.
  m_column = 0;
  m_line = 0;
  m_history.clear();

  return;
}

// Private functions

bdd_function text_bdd_istream::parse(int& character)
{
  // Start parsing at the lowest level of binding
  return parse_biconditional(character);
}

bdd_function text_bdd_istream::parse_biconditional(int& character)
{
  // Continue parsing on the left side at the next level of binding
  bdd_function c = parse_implication(character);

  // Apply biconditionals between the left and right side while present
  while (character == '<')
  {
    // Allow both <=> and <-> notation
    character = next_char(false, true);
    if (character != '=' && character != '-') throw unexpected_character_error(character);
    character = next_char(false, true);
    if (character != '>') throw unexpected_character_error(character);

    // Continue parsing on the right side at the next level of binding
    character = next_char(true, true);
    c = c.equiv(parse_implication(character));
  }

  return c;
}

bdd_function text_bdd_istream::parse_implication(int& character)
{
  // Continue parsing on the left side at the next level of binding
  bdd_function c = parse_disjunction(character);

  // Apply implications between the left and right side while present
  while (character == '=' || character == '-')
  {
    // Allow both => and -> notation
    character = next_char(false, true);
    if (character != '>') throw unexpected_character_error(character);

    // Continue parsing on the right side at the next level of binding
    character = next_char(true, true);
    c = c.imp(parse_disjunction(character));
  }

  return c;
}

bdd_function text_bdd_istream::parse_disjunction(int& character)
{
  // Continue parsing on the left side at the next level of binding
  bdd_function c = parse_conjunction(character);

  // Apply disjunctions between the left and right side while present
  while (character == '|')
  {
    // Allow only || notation
    character = next_char(false, true);
    if (character != '|') throw unexpected_character_error(character);

    // Continue parsing on the right side at the next level of binding
    character = next_char(true, true);
    c |= parse_conjunction(character);
  }

  return c;
}

bdd_function text_bdd_istream::parse_conjunction(int& character)
{
  // Continue parsing on the left side at the next level of binding
  bdd_function c = parse_negation(character);

  // Apply conjunctions between the left and right side while present
  while (character == '&')
  {
    // Allow only && notation
    character = next_char(false, true);
    if (character != '&') throw unexpected_character_error(character);

    // Continue parsing on the right side at the next level of binding
    character = next_char(true, true);
    c &= parse_negation(character);
  }

  return c;
}

bdd_function text_bdd_istream::parse_negation(int& character)
{
  // Allow only ! notation
  if (character == '!')
  {
    // Continue parsing on the right at the same level of binding to allow double negations and negate the result
    character = next_char(true, true);
    return ~parse_negation(character);
  }

  // Continue parsing at the next level of binding
  return parse_primary(character);
}

bdd_function text_bdd_istream::parse_primary(int& character)
{
  if (character == '(')
  {
    // Parse the proposition formula within the parentheses
    character = next_char(true, true);
    const bdd_function& c = parse(character);
    if (character != ')')
    {
      throw std::runtime_error("Missing closing paranthesis ')' while parsing a CFLOBDD term");
    }
    character = next_char();
    return c;
  }

  // Parse a variable starting with an alphabetic character and containing only alphanumeric characters
  if (std::isalpha(character)) {
    // Get the name of the proposition variable
    std::string name;
    while (std::isalnum(character)) {
        name.push_back(character);
        character = next_char(false);
    }

    // Ensure that the next character is not a space
    if (character == ' ')
    {
      character = next_char();
    }

    // Get the BDD representing the proposition variable
    std::unordered_map<std::string, bdd_function>::const_iterator variable = m_variables.find(name);
    if (variable == m_variables.end())
    {
      throw std::runtime_error("Unknown variable '" + name + "' while parsing a CFLOBDD term");
    }
    return variable->second;
  }

  throw unexpected_character_error(character);
}

std::string text_bdd_istream::print_parse_error_position()
{
  std::stringstream s;
  s << "Error occurred at line " << m_line << ", col " << m_column << " near: ";
  for(const auto& element : m_history)
  {
    s << element;
  }
  return s.str();
}

int text_bdd_istream::next_char(bool skip_whitespace, bool required)
{
  character = EOF;

  do
  {
    try
    {
      // In liblts_lts the exception bit is set, so we need to use exception to handle EOF.
      character = m_stream.get();
    }
    catch (std::ios::failure&)
    {
      return EOF;
    }

    if (character != EOF)
    {
      if (character == '\n')
      {
        m_line++;
        m_column = 0;
      }
      else
      {
        m_column++;
      }

      if (m_history.size() >= m_history_limit)
      {
        // If the history is full the first element must be removed.
        m_history.erase(m_history.begin());
      }

      m_history.emplace_back(character);
    }
    else if (required)
    {
      throw std::runtime_error("Premature end of file while parsing.");
    }
  }
  while (isspace(character) && skip_whitespace);

  // The stream also returns a newline for the last symbol.
  return character == '\n' ? EOF : character;
}

} // namespace oxidd
