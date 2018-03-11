//  Atomic lock utilities
//
//  Copyright (C) 2018 Zoltan N. Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZNL_ATOMICLOCK_HPP_INCLUDED
#define ZNL_ATOMICLOCK_HPP_INCLUDED

#include <atomic>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif


#if defined(_MSC_VER)
#endif


namespace znl {

struct AtomicLockGuard
{
  //TODO: memory_order?
  AtomicLockGuard( std::atomic_flag& lock_ ) : _lock( lock_ ) {
    while( _lock.test_and_set( std::memory_order_acquire ) ) ; //spin
  }
  ~AtomicLockGuard() { _lock.clear(std::memory_order_release); }
  std::atomic_flag& _lock;
};

} //namespace znl

#endif //ZNL_ATOMICLOCK_HPP_INCLUDED
