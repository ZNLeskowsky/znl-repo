//  Actor
//
//  Copyright (C) 2018 Zoltan N. Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <cassert>
#include <iostream>

#include "atomiclock.hpp"

#include "actor.hpp"

namespace znl {

void Actor::_run()
{
  assert( _count );
  for( ;; ) {
#ifdef ZNL_ACTOR_INTRUSIVE
    Task *task = _actionqueue.pop();
    if(  task ) {
      ( *task )();
      delete task;
#else
    Func func;
    if( _actionqueue.pop( func ) ) {
      func();
#endif
      if( 0 == --_count ) {
        AtomicLockGuard guard( _lock );
        if( 0 == _count ) {
          _running = false;
          break;
        }
      }
    }
  }
}

void Actor::send( std::function<void()>&& func_ )
{
#ifdef ZNL_ACTOR_INTRUSIVE
  _actionqueue.push( *new Task( std::move( func_ ) ) );
#else
  _actionqueue.push( func_ );
#endif
  if( 0 == _count++ ) {
    AtomicLockGuard guard( _lock );
    if( !_running ) { //if( 1 == _count )
      if( _future.valid() ) {
        _future.wait();
      }
      _running = true;
      //_future.std::future<void>::~future();
      _future = std::move( std::async( std::launch::async,
                           [this] () { _run(); } ) );
    }
  }
}

} //namespace znl
