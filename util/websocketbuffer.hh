#pragma once

#include <rapidjson/rapidjson.h>
#include <rapidjson/internal/stack.h>
#include <libwebsockets.h>

namespace bold
{
  template <typename Allocator = rapidjson::CrtAllocator>
  struct GenericWebSocketBuffer
  {
    typedef unsigned char Ch;

    GenericWebSocketBuffer(Allocator* allocator = nullptr, size_t capacity = kDefaultCapacity)
      : stack_(allocator, capacity)
    {
      Push(LWS_SEND_BUFFER_PRE_PADDING);
    }

    void Put(Ch c) { *stack_.template Push<Ch>() = c; }
    void Flush() {}
    void Clear() { stack_.Clear(); }
    void ShrinkToFit() { stack_.ShrinkToFit(); }
    Ch* Push(size_t count) { return stack_.template Push<Ch>(count); }
    void Pop(size_t count) { stack_.template Pop<Ch>(count); }
    Ch* GetBuffer() { return stack_.template Bottom<Ch>(); }
    size_t GetSize() const { return stack_.GetSize(); }

    static const size_t kDefaultCapacity = 256;
    rapidjson::internal::Stack<Allocator> stack_;
  };

  typedef GenericWebSocketBuffer<> WebSocketBuffer;

  /// Specialized version of PutN() with memset() for better performance.
  inline void PutN(WebSocketBuffer& stream, char c, size_t n)
  {
    std::memset(stream.stack_.Push<char>(n), c, n * sizeof(c));
  }
}
