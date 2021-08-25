// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef GOOGLE_PROTOBUF_STUBS_ONCE_H__
#define GOOGLE_PROTOBUF_STUBS_ONCE_H__

#include <google/protobuf/stubs/system_mutex.h>

#include <utility>
#include <functional>

#include <google/protobuf/port_def.inc>

namespace google {
namespace protobuf {
namespace internal {

// Flag structure which keeps track of invocations status
class once_flag {
 template<typename Callable, typename... Args>
 friend void call_once(once_flag& flag, Callable&& f, Args&&... args);

public:
  /**
   * \brief OnceFlag's constructor
   */

  constexpr once_flag() : mutex_{}, done_{} {}


  /// internal mutex used by callOnce()
  SystemMutex mutex_;

  /// tells whether any function was already called for this object (true) or
  /// not (false)
  volatile bool done_;
};

// Extract object pointer and use to call a member function
template <typename Callable, typename Arg1, typename... Args>
void call_member(Callable &&f, Arg1 arg1,
                      Args &&...args) {
  (std::forward<Arg1>(arg1).*f)(std::forward<Arg1>(arg1), std::forward<Args>(args)...);
}

// In case of a basic function
template <typename Callable, typename... Args>
typename std::enable_if<!(std::is_member_pointer<typename std::decay<Callable>::type>::value), void>::type
call(Callable &&f, Args &&...args){
    std::forward<Callable>(f)(std::forward<Args>(args)...);
 }

// In case a function is a method (member of a class)
template <typename Callable, typename... Args>
typename std::enable_if<std::is_member_pointer<typename std::decay<Callable>::type>::value, void>::type
call(Callable &&f, Args &&...args){
    call_member(std::forward<Callable>(f), std::forward<Args>(args)...);
}

// Handles calling the function once among all invocations
template <typename Callable, typename... Args>
void call_once(once_flag &flag, Callable &&f, Args &&...args) {

  if (flag.done_ == true)
    return;

  const simply_lock_guard<SystemMutex> lockGuard{flag.mutex_};

  if (flag.done_ == true)
    return;

  call(std::forward<Callable>(f), std::forward<Args>(args)...);
  flag.done_ = true;
}

}  // namespace internal

using ProtobufOnceType = internal::once_flag;

inline void GoogleOnceInit(ProtobufOnceType* once, void (*init_func)()) {
  internal::call_once(*once, init_func);
}

template <typename Arg>
inline void GoogleOnceInitArg(ProtobufOnceType* once, void (*init_func)(Arg*),
                              Arg* arg) {
  internal::call_once(*once, init_func, arg);
}

class GoogleOnceDynamic {
 public:
  // If this->Init() has not been called before by any thread,
  // execute (*func_with_arg)(arg) then return.
  // Otherwise, wait until that prior invocation has finished
  // executing its function, then return.
  template<typename T>
  void Init(void (*func_with_arg)(T*), T* arg) {
    GoogleOnceInitArg<T>(&this->state_, func_with_arg, arg);
  }
 private:
  ProtobufOnceType state_;
};

#define GOOGLE_PROTOBUF_ONCE_TYPE ::google::protobuf::ProtobufOnceType
#define GOOGLE_PROTOBUF_DECLARE_ONCE(NAME) \
  ::google::protobuf::ProtobufOnceType NAME

}  // namespace protobuf
}  // namespace google

#include <google/protobuf/port_undef.inc>

#endif  // GOOGLE_PROTOBUF_STUBS_ONCE_H__
