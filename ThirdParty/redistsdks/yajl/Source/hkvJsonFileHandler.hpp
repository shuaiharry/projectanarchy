#ifndef VJSONFILEHANDLER_HPP_INCLUDED
#define VJSONFILEHANDLER_HPP_INCLUDED

/// \brief
///   Interface to enable file operation inside hkvJsonStreamReader and hkvJsonWriter
class hkvJsonFileHandler
{
public:
  enum JsonFileMode
  {
    JFM_READ=0,
    JFM_WRITE,
    JFM_COUNT
  };

  hkvJsonFileHandler () {}
  virtual ~hkvJsonFileHandler () {}

public: // Interface
  virtual void* Open(const char* pszFileName, JsonFileMode mode) = 0;
  virtual size_t Read(void* pBuffer, size_t iLen, void* pStream) = 0;
  virtual void Close(void* pStream) = 0;
  virtual size_t Write(void* pBuffer, size_t iLen, void* pStream) = 0;
  virtual bool IsEOF(void* pStream) = 0;
};

#endif