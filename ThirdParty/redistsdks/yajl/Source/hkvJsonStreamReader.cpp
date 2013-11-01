#include "hkvJsonStreamReader.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "yajl_alloc.h"
#include "yajl_parser.h"

static void yajl_Free(void * ctx, void * ptr)
{
  assert(ptr != NULL);
  hkvJsonMemoryHandler* pMemHandler = (hkvJsonMemoryHandler*)ctx;
  assert(pMemHandler);
  pMemHandler->Free(ptr);
}

static void* yajl_Malloc(void * ctx, size_t sz)
{
  assert(sz != 0);
  hkvJsonMemoryHandler* pMemHandler = (hkvJsonMemoryHandler*)ctx;
  assert(pMemHandler);
  return pMemHandler->Malloc(sz);
}

static void* yajl_Realloc(void * ctx, void * ptr, size_t sz)
{
  if (ptr == NULL)
    assert(sz != 0);

  hkvJsonMemoryHandler* pMemHandler = (hkvJsonMemoryHandler*)ctx;
  assert(pMemHandler);
  return pMemHandler->Realloc(ptr, sz);
}

static int yajl_null(void *ctx)
{
  hkvJsonStreamReadHandler* pReadHandler = (hkvJsonStreamReadHandler*)ctx;
  assert(pReadHandler);
  return (int)pReadHandler->ParseNull();
}

static int yajl_boolean(void * ctx, int boolVal)
{
  hkvJsonStreamReadHandler* pReadHandler = (hkvJsonStreamReadHandler*)ctx;
  assert(pReadHandler);
  return (int)pReadHandler->ParseBool(boolVal ? true : false);
}

static int yajl_integer(void *ctx, long long integerVal)
{
  hkvJsonStreamReadHandler* pReadHandler = (hkvJsonStreamReadHandler*)ctx;
  assert(pReadHandler);
  return (int)pReadHandler->ParseInt(integerVal);
}

static int yajl_double(void *ctx, double doubleVal)
{
  hkvJsonStreamReadHandler* pReadHandler = (hkvJsonStreamReadHandler*)ctx;
  assert(pReadHandler);
  return (int)pReadHandler->ParseDouble(doubleVal);
}

static int yajl_string(void *ctx, const unsigned char * stringVal, size_t stringLen)
{
  hkvJsonStreamReadHandler* pReadHandler = (hkvJsonStreamReadHandler*)ctx;
  assert(pReadHandler);
  return (int)pReadHandler->ParseString(stringVal, stringLen);
}

static int yajl_map_key(void *ctx, const unsigned char * stringVal, size_t stringLen)
{
  hkvJsonStreamReadHandler* pReadHandler = (hkvJsonStreamReadHandler*)ctx;
  assert(pReadHandler);
  return (int)pReadHandler->ParseMapKey(stringVal, stringLen);
}

static int yajl_start_map(void *ctx)
{
  hkvJsonStreamReadHandler* pReadHandler = (hkvJsonStreamReadHandler*)ctx;
  assert(pReadHandler);
  return (int)pReadHandler->ParseStartMap();
}


static int yajl_end_map(void *ctx)
{
  hkvJsonStreamReadHandler* pReadHandler = (hkvJsonStreamReadHandler*)ctx;
  assert(pReadHandler);
  return (int)pReadHandler->ParseEndMap();
}

static int yajl_start_array(void *ctx)
{
  hkvJsonStreamReadHandler* pReadHandler = (hkvJsonStreamReadHandler*)ctx;
  assert(pReadHandler);
  return (int)pReadHandler->ParseStartArray();
}

static int yajl_end_array(void *ctx)
{
  hkvJsonStreamReadHandler* pReadHandler = (hkvJsonStreamReadHandler*)ctx;
  assert(pReadHandler);
  return (int)pReadHandler->ParseEndArray();
}

static yajl_callbacks callbacks = {
  yajl_null,
  yajl_boolean,
  yajl_integer,
  yajl_double,
  NULL,
  yajl_string,
  yajl_start_map,
  yajl_map_key,
  yajl_end_map,
  yajl_start_array,
  yajl_end_array
};

hkvJsonStreamReader::hkvJsonStreamReader(hkvJsonFileHandler* pFileHandler, hkvJsonStreamReadHandler* pReadHandler, int iParserFlags) :
  m_pHandle(NULL),
  m_pMemHandler(NULL),
  m_pFileHandler(pFileHandler),
  m_pReadHandler(pReadHandler)
{
  assert(m_pFileHandler);
  assert(m_pReadHandler);

  InitHandle(NULL);
  InitConfig(iParserFlags);
}

hkvJsonStreamReader::hkvJsonStreamReader(hkvJsonMemoryHandler* pMemHandler, hkvJsonFileHandler* pFileHandler, hkvJsonStreamReadHandler* pReadHandler, int iParserFlags) :
  m_pHandle(NULL),
  m_pMemHandler(pMemHandler),
  m_pFileHandler(pFileHandler),
  m_pReadHandler(pReadHandler)
{
  assert(m_pMemHandler);
  assert(m_pFileHandler);
  assert(m_pReadHandler);

  // Allocate a structure which holds allocation routines
  yajl_alloc_funcs allocFuncs = {
    yajl_Malloc,
    yajl_Realloc,
    yajl_Free,
    m_pMemHandler
  };

  InitHandle(&allocFuncs);
  InitConfig(iParserFlags);
}

hkvJsonStreamReader::~hkvJsonStreamReader()
{
  yajl_free(m_pHandle);
}

bool hkvJsonStreamReader::Parse(const char* pszFileName, unsigned int uiChunkSize)
{
  yajl_status stat;
  void* pFile;
  size_t rd;

  // Allocate read buffer
  unsigned char* pFileData = (unsigned char*)(YA_MALLOC(&(m_pHandle->alloc), uiChunkSize));
  assert(pFileData);

  // Open file
  pFile = m_pFileHandler->Open(pszFileName, hkvJsonFileHandler::JFM_READ);
  if (pFile == NULL)
    return false;

  for (;;)
  {
    rd = m_pFileHandler->Read((void*)pFileData, uiChunkSize - 1, pFile);

    if (rd == 0)
    {
      if (!m_pFileHandler->IsEOF(pFile))
        fprintf(stderr, "Error encountered on file read.\n");
      break;
    }
    pFileData[rd] = 0;

    // Pass to parser
    stat = yajl_parse(m_pHandle, pFileData, rd);

    if (stat != yajl_status_ok)
      break;
  }

  // Parse any remaining buffered data
  stat = yajl_complete_parse(m_pHandle);

  if (stat != yajl_status_ok)
  {
    unsigned char * str = yajl_get_error(m_pHandle, 1, pFileData, rd);
    fprintf(stderr, "%s", (const char *) str);
    yajl_free_error(m_pHandle, str);

    assert(false);
    return false;
  }

  // Close the file
  m_pFileHandler->Close(pFile);

  // Free read buffer
  YA_FREE(&(m_pHandle->alloc), pFileData);

  return true;
}

void hkvJsonStreamReader::InitHandle(yajl_alloc_funcs * afs)
{
  assert(m_pReadHandler);
  m_pHandle = yajl_alloc(&callbacks, afs, m_pReadHandler);
  assert(m_pHandle);
}

void hkvJsonStreamReader::InitConfig(int iParserFlags)
{
  assert(m_pHandle);
  yajl_config(m_pHandle, yajl_allow_comments, (iParserFlags & ALLOW_COMMENTS));
  yajl_config(m_pHandle, yajl_dont_validate_strings, (iParserFlags & DISABLE_UTF8_VALIDATION));
  yajl_config(m_pHandle, yajl_allow_trailing_garbage, (iParserFlags & ALLOW_TRAILING_GARBAGE));
  yajl_config(m_pHandle, yajl_allow_multiple_values, (iParserFlags & ALLOW_MULTIPLE_VALUES));
}
