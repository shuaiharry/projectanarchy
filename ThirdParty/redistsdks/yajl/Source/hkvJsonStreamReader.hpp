#ifndef VJSONSTREAMREADER_HPP_INCLUDED
#define VJSONSTREAMREADER_HPP_INCLUDED

#include "api/yajl_parse.h"
#include "hkvJsonMemoryHandler.hpp"
#include "hkvJsonFileHandler.hpp"
#include "hkvJsonStreamReadHandler.hpp"

/// \brief
///   Class for stream based reading a JSON file
class hkvJsonStreamReader
{
public:

  /// \brief
  ///   Parser flags
  enum ParserFlags
  {
    ALLOW_COMMENTS          = 0x01, ///< Javascript style comments are ignored
    DISABLE_UTF8_VALIDATION = 0x02, ///< Disable UTF8 string validation
    ALLOW_TRAILING_GARBAGE  = 0x04, ///< By default the parser will ensure the entire input text was consumed and will raise an error otherwise.
                                    ///  Enabling this flag will cause yajl to disable this check.
    ALLOW_MULTIPLE_VALUES   = 0x08  ///< Allow multiple values to be parsed by a single handle. This flag will change the behavior of the parser,
                                    ///  and cause it continue parsing after a value is parsed, rather than transitioning into a complete state.
                                    ///  This option can be useful when parsing multiple values from an input stream.
  };

	/// \brief
  ///   Default constructor without memory management
	hkvJsonStreamReader(hkvJsonFileHandler* pFileHandler, hkvJsonStreamReadHandler* pReadHandler, int iParserFlags = ALLOW_COMMENTS);

  /// \brief
  ///   Constructor including memory management
  hkvJsonStreamReader(hkvJsonMemoryHandler* pMemHandler, hkvJsonFileHandler* pFileHandler, hkvJsonStreamReadHandler* pReadHandler, int iParserFlags = ALLOW_COMMENTS);

  /// \brief
  ///   Destructor
	~hkvJsonStreamReader();

  /// \brief
  ///   Parses a file with the given filename
  bool Parse(const char* pszFileName, unsigned int uiChunkSize = 128);

private:
  void InitHandle(yajl_alloc_funcs * afs);
  void InitConfig(int iParserFlags);

  hkvJsonMemoryHandler* m_pMemHandler;
  hkvJsonFileHandler* m_pFileHandler;
  hkvJsonStreamReadHandler* m_pReadHandler;

  /// Handle to access yajl functionality
	yajl_handle m_pHandle;
};

#pragma once

#endif
