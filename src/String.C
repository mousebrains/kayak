#include "String.H"

std::string
String::trim(const std::string& str)
{
  const std::string whitespace(" \t\n\r");
  std::string::size_type i0(str.find_first_not_of(whitespace));

  if (i0 == str.npos) return std::string(); // Nothing but whitespace

  std::string::size_type i1(str.find_last_not_of(whitespace));

  if (i1 == str.npos) return str.substr(i0); // No trailing whitespace

  return str.substr(i0, i1 - i0 + 1); // leading and trailing whitespaces stripped
}

std::string
String::tolower(const std::string& str)
{
  std::string result(str);

  for (std::string::size_type i(0), e(result.size()); i < e; ++i) {
    result[i] = std::tolower(result[i]);
  }

  return result;
}

std::string
String::capitalize(const std::string& str)
{
  if (str.empty()) return str;
  std::string result(tolower(str));
  result[0] = std::toupper(result[0]);
  return result;
}
