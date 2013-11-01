#ifndef VJSONMEMHANDLER_HPP_INCLUDED
#define VJSONMEMHANDLER_HPP_INCLUDED

/// \brief
///   Interface to replace build-in memory management of yajl
class hkvJsonMemoryHandler
{
public:
  hkvJsonMemoryHandler () {}
  virtual ~hkvJsonMemoryHandler () {}

public: // Interface
  virtual void Free(void* ptr) = 0;
  virtual void* Malloc(size_t sz) = 0;
  virtual void* Realloc(void* ptr, size_t sz) = 0;
};

#endif