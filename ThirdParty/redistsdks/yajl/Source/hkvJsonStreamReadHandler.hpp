#ifndef VJSONSTREAMREADHANDLER_HPP_INCLUDED
#define VJSONSTREAMREADHANDLER_HPP_INCLUDED

/// \brief
///   Interface to process data parsed by hkvJsonStreamReader
class hkvJsonStreamReadHandler
{
public:
  enum ParsingResult
  {
    BREAK_PARSING = 0,
    CONTINUE_PARSING
  };

  hkvJsonStreamReadHandler () {}
  virtual ~hkvJsonStreamReadHandler () {}

public: // Interface
  virtual ParsingResult ParseNull() = 0;
  virtual ParsingResult ParseBool(bool bValue) = 0;
  virtual ParsingResult ParseInt(long long iValue) = 0;
  virtual ParsingResult ParseDouble(double dValue) = 0;
  virtual ParsingResult ParseString(const unsigned char* pszValue, size_t len) = 0;
  virtual ParsingResult ParseMapKey(const unsigned char* pszValue, size_t len) = 0;
  virtual ParsingResult ParseStartMap() = 0;
  virtual ParsingResult ParseEndMap() = 0;
  virtual ParsingResult ParseStartArray() = 0;
  virtual ParsingResult ParseEndArray() = 0;
};

#endif