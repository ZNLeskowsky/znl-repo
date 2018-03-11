//  Worker
//
//  Copyright (C) 2018 Zoltan N. Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <unistd.h>
#include "worker.hpp"
#include "logger.hpp"

namespace znl {

//#define DEBUG( MSG ) std::cerr << "DEBUG: " << MSG << std::endl;
#define DEBUG( MSG ) LOG( "DEBUG: " << MSG );

static void renew_promise( std::promise<void>& p_ )
{
  p_.~promise<void>();
  ( void ) new ( &p_ ) std::promise<void>();
}

Worker::~Worker()
{
  if( is_running() ) {
    stop();
  }
}

void Worker::start()
{
  _status = 0;
  DEBUG( "Worker " << name() << " starting" );
  _thread = std::thread( [this] () {
              DEBUG( "Worker " << name() << " running" );
              _run();
              DEBUG( "Worker " << name() << " ran" );
            } );
  DEBUG( "Worker " << name() << " started" );
}

void Worker::start( std::function<int( std::thread& )>&& prepare_ )
{
  _status = 0;
  _thread = std::thread( 
              [this] () {
                _ready.get_future().wait();
#ifdef ZNL_MULTIUSE_FUTURE
                renew_promise( _ready );
#endif
                if( _status == 0 ) {
                  _run();
                }
              }
            );
  _status = prepare_( _thread );
  _ready.set_value();
}

void Worker::_run()
{
  DEBUG( "Worker " << name() << " _run()" );
  for( ;; ) {
    DEBUG( "Worker " << name() << " popping" );
    Task& task = _pop();
    DEBUG( "Worker " << name() << " popped" );
    if( &task == &_stop ) {
      DEBUG( "Worker " << name() << " stopping" );
      break;
    }
    task();
  }
  DEBUG( "Worker " << name() << " stopped" );
}

#ifdef ZNL_MULTIUSE_FUTURE
void Worker::send( Task& task_ )
{
  _taskqueue.push( task_ );
  if( 0 == _count++ ) {
    AtomicLockGuard lk( _lock );
    if( _waiting ) {
      _ready.set_value();
      _waiting = false;
    }
  }
}

#else //!ZNL_MULTIUSE_FUTURE
void Worker::send( Task& task_ )
{
  _taskqueue.push( task_ );
  if( 0 == _count++ ) {
    {
      std::lock_guard<std::mutex> lk( _mutex ); // needed?
      if( !_waiting ) {
        return;
      }
    }
    _condvar.notify_one();
    _waiting = false;
  }
}
#endif

#ifdef ZNL_MULTIUSE_FUTURE
Task& Worker::_pop()
{
  Task *ptask;
  int count = _count--; //TODO
  DEBUG( "Worker " << name() << count << " tasks queued" );
  while( ( ptask = _taskqueue.pop() ) == nullptr ) {
    DEBUG( "Worker " << name() << " popped null task" );
    if( 0 == count ) {
      ++_count; //TODO
      bool wait;
      {
        AtomicLockGuard lk( _lock );
        _waiting = wait = ( 0 == _count.load() );
      }
      if( wait ) {
        DEBUG( "Worker " << name() << " waiting for push" );
        _ready.get_future().wait();
        renew_promise( _ready );
      }
      count = _count--; //TODO
    }
  }
  return *ptask;
}

#else //!ZNL_MULTIUSE_FUTURE
Task& Worker::_pop()
{
  DEBUG( "Worker " << name() << " _pop()" );
  Task *ptask;
  int count = _count--; //TODO
  while( ( ptask = _taskqueue.pop() ) == nullptr ) {
    DEBUG( "Worker " << name() << " _pop() popped null task" );
    if( 0 == count ) {
      ++_count; //TODO
      std::unique_lock<std::mutex> lk( _mutex );
      _waiting = true;
      if( 0 == _count.load() ) {
        //DEBUG( "Worker " << name() << " waiting for condvar" );
        _condvar.wait( lk, [&ptask, this] () {
                             return ( ptask = _taskqueue.pop() );
                           } );
        break;
      }
      _waiting = false;
    }
  }
  DEBUG( "Worker " << name() << " _pop() returning task" );
  return *ptask;
}
#endif

} //namespace znl
