//  Actor
//
//  Copyright (C) 2018 Zoltan N. Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZNL_ACTOR_HPP_INCLUDED
#define ZNL_ACTOR_HPP_INCLUDED

#include <atomic>
#include <future>
#include <string>

#include "taskqueue.hpp"

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif


#if defined(_MSC_VER)
#endif


//#define ZNL_ACTOR_INTRUSIVE

namespace znl {
namespace detail {
} //namespace detail

#ifdef ZNL_ACTOR_INTRUSIVE
  using ActionQueue = TaskQueue;
#else
  using ActionQueue = FuncQueue;
#endif

class Actor
{
public:
  Actor() : _running( false ), _lock( ATOMIC_FLAG_INIT ), _count( 0 ) {}
  Actor( const std::string& name_ )
    : _name( name_ ), _running( false ),
      _lock( ATOMIC_FLAG_INIT ), _count( 0 ) {}
  ~Actor() { if( _future.valid() ) _future.wait(); }
  const std::string& name() const { return _name; }
  void send( Func&& func_ );
  //void send( Task&& task_ ) { send( std::move( task_.get_func() ) ); }
  int active() const { return _count; }
private:
  void _run();
private:
  std::string         _name;
  ActionQueue         _actionqueue;
  bool                _running;
  std::atomic_flag    _lock;
  std::atomic<int>    _count;
  std::future<void>   _future;
};

} //namespace znl

#endif //ZNL_ACTOR_HPP_INCLUDED
