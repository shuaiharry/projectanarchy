#include "hkvJsonWriter.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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

hkvJsonWriter::hkvJsonWriter(hkvJsonFileHandler* pFileHandler, int iWriterFlags) :
  m_pMemHandler(NULL),
  m_pFileHandler(pFileHandler),
  m_pGenerator(NULL),
  m_bIsRunning(false),
  m_pFile(NULL),
  m_uiChunkSize(0)
{
  assert(m_pFileHandler);

	m_pGenerator = yajl_gen_alloc(NULL);
  InitConfig(iWriterFlags);
}

hkvJsonWriter::hkvJsonWriter(hkvJsonMemoryHandler* pMemHandler, hkvJsonFileHandler* pFileHandler, int iWriterFlags) :
  m_pMemHandler(pMemHandler),
  m_pFileHandler(pFileHandler),
  m_pGenerator(NULL),
  m_bIsRunning(false),
  m_pFile(NULL),
  m_uiChunkSize(0)
{
  assert(m_pMemHandler);
  assert(m_pFileHandler);

  // Allocate a structure which holds allocation routines
  yajl_alloc_funcs allocFuncs = {
    yajl_Malloc,
    yajl_Realloc,
    yajl_Free,
    m_pMemHandler
  };

  m_pGenerator = yajl_gen_alloc(&allocFuncs);
  InitConfig(iWriterFlags);
}

hkvJsonWriter::~hkvJsonWriter()
{
	yajl_gen_free(m_pGenerator);
}

bool hkvJsonWriter::BeginWriting(const char* pszFileName, unsigned int uiChunkSize)
{
  assert(!m_bIsRunning);

  // Open file
  m_pFile = m_pFileHandler->Open(pszFileName, hkvJsonFileHandler::JFM_WRITE);
  if (m_pFile == NULL)
    return false;

  m_bIsRunning = true;
  m_uiChunkSize = uiChunkSize;
  return true;
}

bool hkvJsonWriter::EndWriting()
{
  assert(m_bIsRunning);

  // Write remaining stuff
  Flush(true);

  // Close the file
  m_pFileHandler->Close(m_pFile);

  m_bIsRunning = false;
  m_uiChunkSize = 0;

  return true;
}

bool hkvJsonWriter::WriteNull() const
{
  yajl_gen_status status = yajl_gen_null(m_pGenerator);
  PrintError(status);
  Flush();
  return (yajl_gen_status_ok == status);
}

bool hkvJsonWriter::WriteBool(bool bValue) const
{
  yajl_gen_status status = yajl_gen_bool(m_pGenerator, bValue);
  PrintError(status);
  Flush();
  return (yajl_gen_status_ok == status);
}

bool hkvJsonWriter::WriteInt(long long int iValue) const
{
  yajl_gen_status status = yajl_gen_integer(m_pGenerator, iValue);
  PrintError(status);
  Flush();
  return (yajl_gen_status_ok == status);
}

bool hkvJsonWriter::WriteDouble(double dValue) const
{
  yajl_gen_status status = yajl_gen_double(m_pGenerator, dValue);
  PrintError(status);
  Flush();
  return (yajl_gen_status_ok == status);
}

bool hkvJsonWriter::WriteString(const char* pszValue, size_t len) const
{
  yajl_gen_status status = yajl_gen_string(m_pGenerator, (const unsigned char*)pszValue, len);
  PrintError(status);
  Flush();
  return (yajl_gen_status_ok == status);
}

bool hkvJsonWriter::WriteMapKey(const char* pszValue, size_t len) const
{
  yajl_gen_status status = yajl_gen_string(m_pGenerator, (const unsigned char*)pszValue, len);
  PrintError(status);
  Flush();
  return (yajl_gen_status_ok == status);
}

bool hkvJsonWriter::WriteStartMap() const
{
  yajl_gen_status status = yajl_gen_map_open(m_pGenerator);
  PrintError(status);
  Flush();
  return (yajl_gen_status_ok == status);
}

bool hkvJsonWriter::WriteEndMap() const
{
  yajl_gen_status status = yajl_gen_map_close(m_pGenerator);
  PrintError(status);
  Flush();
  return (yajl_gen_status_ok == status);
}

bool hkvJsonWriter::WriteStartArray() const
{
  yajl_gen_status status = yajl_gen_array_open(m_pGenerator);
  PrintError(status);
  Flush();
  return (yajl_gen_status_ok == status);
}

bool hkvJsonWriter::WriteEndArray() const
{
  yajl_gen_status status = yajl_gen_array_close(m_pGenerator);
  PrintError(status);
  Flush();
  return (yajl_gen_status_ok == status);
}

void hkvJsonWriter::InitConfig(int iWriterFlags)
{
  assert(m_pGenerator);
  yajl_gen_config(m_pGenerator, yajl_gen_beautify, iWriterFlags & GENERATE_INDENTED_OUTPUT);
  yajl_gen_config(m_pGenerator, yajl_gen_validate_utf8, iWriterFlags & VALIDATE_UTF8);
  yajl_gen_config(m_pGenerator, yajl_gen_escape_solidus, iWriterFlags & ESCAPE_SOLIDUS);
}

void hkvJsonWriter::Flush(bool bForce) const
{
  assert(m_pGenerator);
  assert(m_bIsRunning);

  size_t len;
  const unsigned char * buf;
  yajl_gen_status status = yajl_gen_get_buf(m_pGenerator, &buf, &len);
  assert(status == yajl_gen_status_ok);

  if ((len > m_uiChunkSize) || bForce)
  {
    m_pFileHandler->Write((void*)buf, len, m_pFile);
    yajl_gen_clear(m_pGenerator);
  }
}

void hkvJsonWriter::PrintError(yajl_gen_status status) const
{
#if _DEBUG
  if (status == yajl_gen_status_ok)
    return;

  switch (status)
  {
  case yajl_gen_keys_must_be_strings:
    fprintf(stderr, "Error: At a point where a map key is generated, a function other than WriteMapKey or WriteString was called.\n");
    break;
  case yajl_max_depth_exceeded:
    fprintf(stderr, "Error: YAJL's maximum generation depth was exceeded.\n");
    break;
  case yajl_gen_in_error_state:
    fprintf(stderr, "Error: A generator function was called while in an error state.\n");
    break;
  case yajl_gen_invalid_number:
    fprintf(stderr, "Error: Invalid floating point value (infinity or NaN).\n");
    break;
  case yajl_gen_invalid_string:
    fprintf(stderr, "Error: UTF8 validation failed.\n");
    break;
  default:
    fprintf(stderr, "Error: Unknown error.\n");
    break;
  }
#endif
}
