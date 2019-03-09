//  Worker
//
//  Copyright (C) 2018 Zoltan N. Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZNL_WORKER_HPP_INCLUDED
#define ZNL_WORKER_HPP_INCLUDED

#define ZNL_MULTIUSE_FUTURE

#include <atomic>
#include <future>
#ifndef ZNL_MULTIUSE_FUTURE
#include <condition_variable>
#include <mutex>
#endif
#include <thread>

#ifdef ZNL_MULTIUSE_FUTURE
#include "atomiclock.hpp"
#endif

#include "taskqueue.hpp"

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif


#if defined(_MSC_VER)
#endif


namespace znl {

class Worker
{
public:
  Worker() :
    _waiting( false ), _count( 0 ), _status( 0 ), _lock( ATOMIC_FLAG_INIT ) {}
  Worker( const std::string& name_ ) : _name( name_ ),
    _waiting( false ), _count( 0 ), _status( 0 ), _lock( ATOMIC_FLAG_INIT ) {}
  ~Worker();
  const std::string& name() const { return _name; }
  void set_name( const std::string& name_ ) { _name = name_; }
  void start();
  void start( std::function<int( std::thread& )>&& prepare_ );
  bool is_running() { return _thread.joinable(); }
  void send_stop() { send( _stop ); }
  void wait_until_stopped() { if( _thread.joinable() ) { _thread.join(); } }
  void stop() { send_stop(); wait_until_stopped(); }
  void send( const Task& task_ );
  void set_status( int status_ ) { _status = status_; }
  int get_status() const { return _status; }
private:
  void _run();
  const Task& _pop();
protected:
  std::thread::native_handle_type native_handle() {
    return _thread.native_handle();
  }
private:
  std::string             _name;
  TaskQueue               _taskqueue;
  std::atomic<bool>       _waiting;
  std::atomic<int>        _count;
  std::atomic<int>        _status;
  std::atomic_flag        _lock;
#ifndef ZNL_MULTIUSE_FUTURE
  std::condition_variable _condvar;
  std::mutex              _mutex;
#endif
  std::promise<void>      _ready;
  std::thread             _thread;
  Task                    _stop;
};

} //namespace znl

#endif //ZNL_WORKER_HPP_INCLUDED
