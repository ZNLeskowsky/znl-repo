//  Lock-free Task queue
//
//  Copyright (C) 2018 Zoltan N. Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZNL_TASK_QUEUE_HPP_INCLUDED
#define ZNL_TASK_QUEUE_HPP_INCLUDED

#include "mpscqueue.hpp"
#include <functional>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif


#if defined(_MSC_VER)
#endif


namespace znl {
namespace detail {
} //namespace detail

typedef std::function<void()> Func;

class Task : public MPSCNode<Func>
{
public:
  Task() = default;
  Task( const Task &task_ ) //= delete;
    : MPSCNode<Func>( task_ ) {}
  Task( Task &&task_ )
    : MPSCNode<Func>( std::move( task_ ) ) {}
  Task( const Func &func_ ) //= delete;
    : MPSCNode<Func>( func_ ) {}
  Task( Func &&func_ )
    : MPSCNode<Func>( std::move( func_ ) ) {}
  Task& operator=( const Task &task_ ) { //= delete;
    MPSCNode<Func>::assign( task_ );
    return *this;
  }
  Task& operator=( Task &&task_ ) {
    MPSCNode<Func>::assign( std::move( task_ ) );
    return *this;
  }
  Task& operator=( const Func &func_ ) { //= delete;
    MPSCNode<Func>::set_value( func_ );
    return *this;
  }
  Task& operator=( Func &&func_ ) {
    MPSCNode<Func>::set_value( std::move( func_ ) );
    return *this;
  }
  void operator()() const { get_value()(); }
  operator bool() const { return static_cast<bool>( get_value() ); }
  bool operator!() const { return !get_value(); }
};
 
template<> inline
void MPSCQueue<Func>::assign_or_move( Func& to_, const Func& from_ ) { to_ = std::move( from_ ); }

using FuncQueue = MPSCQueue<Func>;
using TaskQueue = MPSCIntrQueue<Task>;

} //namespace znl

#endif //ZNL_TASK_QUEUE_HPP_INCLUDED
