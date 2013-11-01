#ifndef VJSONWRITER_HPP_INCLUDED
#define VJSONWRITER_HPP_INCLUDED

#include "api/yajl_gen.h"
#include "hkvJsonMemoryHandler.hpp"
#include "hkvJsonFileHandler.hpp"

/// \brief
///   Class for writing a JSON file
class hkvJsonWriter
{
public:

  /// \brief
  ///   Writer flags
  enum WriterFlags
  {
    GENERATE_INDENTED_OUTPUT = 0x01,  ///< Generate indented (readable) output
    VALIDATE_UTF8 = 0x08,             ///< Enable UTF8 validation of strings passed to WriteString
    ESCAPE_SOLIDUS  = 0x10            ///< The forward solidus (slash or '/' in human) is not required to be escaped in json text.
                                      ///  By default, YAJL will not escape it in the iterest of saving bytes.
                                      ///  Setting this flag will cause YAJL to always escape '/' in generated JSON strings.
  };

  /// \brief
  ///   Default constructor without memory management
	hkvJsonWriter(hkvJsonFileHandler* pFileHandler, int iWriterFlags = (GENERATE_INDENTED_OUTPUT | VALIDATE_UTF8));

  /// \brief
  ///   Constructor including memory management
  hkvJsonWriter(hkvJsonMemoryHandler* pMemHandler, hkvJsonFileHandler* pFileHandler, int iWriterFlags = (GENERATE_INDENTED_OUTPUT | VALIDATE_UTF8));
	
  /// \brief
  ///   Destructor
	~hkvJsonWriter();

  /// \brief
  ///   Begins writing
  bool BeginWriting(const char* pszFileName, unsigned int uiChunkSize = 128);

  /// \brief
  ///   Ends writing
  bool EndWriting();

  bool WriteNull() const;
  bool WriteBool(bool bValue) const;
  bool WriteInt(long long int iValue) const;
  bool WriteDouble(double dValue) const;
  bool WriteString(const char* pszValue, size_t len) const;
  bool WriteMapKey(const char* pszValue, size_t len) const;
  bool WriteStartMap() const;
  bool WriteEndMap() const;
  bool WriteStartArray() const;
  bool WriteEndArray() const;

private:
  void InitConfig(int iWriterFlags);
  void Flush(bool bForce = false) const;
  void PrintError(yajl_gen_status status) const;

  hkvJsonMemoryHandler* m_pMemHandler;
  hkvJsonFileHandler* m_pFileHandler;

  // Generator used to create JSON stuff
	yajl_gen m_pGenerator;

  bool m_bIsRunning;
  void* m_pFile;
  unsigned int m_uiChunkSize;
};

#pragma once

#endif
