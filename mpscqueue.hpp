//  Lock-free MPSC intrusive and non-intrusive queues based on
//  Vyukov, Dmitry,
//  http://www.1024cores.net/home/lock-free-algorithms/queues/
//
//  Copyright (C) 2018 Zoltan N. Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZNL_MPSC_QUEUE_HPP_INCLUDED
#define ZNL_MPSC_QUEUE_HPP_INCLUDED

#include <atomic>
#include <cstddef>
#include <utility>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif


#if defined(_MSC_VER)
#endif


namespace znl {
namespace detail {
} //namespace detail

class SLinkable
{
public:
  SLinkable() = default; // can't be made protected
  SLinkable( const SLinkable& ) : SLinkable() {} //= delete;
  SLinkable( SLinkable&& ) : SLinkable() {} //= delete;
  SLinkable& operator=( const SLinkable& ) {} //= delete;
  SLinkable& operator=( SLinkable&& ) {} //= delete;
protected:
  friend class MPSCIntrQueueBase;
  const std::atomic<const SLinkable*>* immutable_next() const { return &_next; }
private:
  friend class MPSCQueueBase;
  std::atomic<const SLinkable*>* mutable_next() const { return &_next; }
private:
  mutable std::atomic<const SLinkable*> _next;
};

template<typename T> class MPSCQueue;

template<typename T>
class MPSCNode : public SLinkable
{
public:
  MPSCNode() = default;
  MPSCNode( const MPSCNode& node_ ) : _value( node_._value ) {}
  MPSCNode( MPSCNode&& node_ ) : _value( std::move( node_._value ) ) {}
  explicit MPSCNode( const T& value_ ) : _value( value_ ) {}
  explicit MPSCNode( T&& value_ ) : _value( std::move( value_ ) ) {}
  MPSCNode& operator=( const MPSCNode& node_ ) = default;
  MPSCNode& operator=( MPSCNode&& node_ ) = default;
  void set_value( const T& value_ ) { _value = value_; }
  void set_value( T&& value_ ) { _value = std::move( value_ ); }
  const T& get_value() const { return _value; }

private:
  friend class MPSCQueueBase;
  friend class MPSCQueue<T>;
  T& get_mutable_value() { return _value; }
  T&& get_move_value() & { return std::move( _value ); }
  const MPSCNode* load_next( std::memory_order order_ ) const {
    return static_cast<const MPSCNode*>( SLinkable::immutable_next()->load( order_ ) );
  }

private:
  T _value;
};

class MPSCQueueBase
{
protected:
  MPSCQueueBase() = default;
  MPSCQueueBase( const SLinkable& stub_ );
  void init( const SLinkable& stub_ ) {
    _first.store( &stub_, std::memory_order_relaxed );
    _last.store( &stub_, std::memory_order_relaxed );
  }
  void clear_next( const SLinkable& linkable_, std::memory_order order_ = std::memory_order_relaxed ) {
    linkable_.mutable_next()->store( nullptr, order_);
  }
  const SLinkable* exchange_last( const SLinkable& linkable_, std::memory_order order_ = std::memory_order_acq_rel ) {
    return _last.exchange( &linkable_, order_ ); // may block
  }
  void store_next( const SLinkable& prev_, const SLinkable& linkable_, std::memory_order order_ = std::memory_order_relaxed ) {
    prev_.mutable_next()->store( &linkable_, order_);
  }
  void push( const SLinkable& linkable_ );
  void store_first( const SLinkable* linkable_, std::memory_order order_ ) { _first.store( linkable_, order_ ); }
  const SLinkable* load_first( std::memory_order order_ ) const { return _first.load( order_ ); }
  const SLinkable* load_last( std::memory_order order_ ) const { return _last.load( order_ ); }
  //TODO:
  bool push_in_process() const { return false; }
private:
  std::atomic<const SLinkable*> _last;
  std::atomic<const SLinkable*> _first;
};

// Intrusive queue

class MPSCIntrQueueBase : public MPSCQueueBase
{
protected:
  MPSCIntrQueueBase() : MPSCQueueBase( _stub ) {}
  const SLinkable* pop();
private:
  SLinkable _stub;
};

template<class T>
class MPSCIntrQueue : public MPSCIntrQueueBase
{
public:
  MPSCIntrQueue() = default;
  void push( const T& val_ ) { MPSCQueueBase::push( val_ ); }
  const T* pop() { return static_cast<const T*>( MPSCIntrQueueBase::pop() ); }
};

// Non-intrusive queue

template<typename T>
class MPSCQueue : public MPSCQueueBase
{
public:
  MPSCQueue() : _stub( new MPSCNode<T>() ) { MPSCQueueBase::init( *_stub ); }
  ~MPSCQueue() {
    MPSCNode<T>* first = const_cast<MPSCNode<T>*>( load_first( std::memory_order_relaxed ) );
    MPSCNode<T>* next;
    for( MPSCNode<T>* node = first; node; node = next ) {
      next = const_cast<MPSCNode<T>*>( node->load_next( std::memory_order_relaxed ) );
      delete node;
      if( next == first ) {
        break;
      }
    }
  }
  void push( const T& value_ ) {
    MPSCQueueBase::push( *new MPSCNode<T>( value_ ) );
  }
  void push( T&& value_ ) {
    MPSCQueueBase::push( *new MPSCNode<T>( std::move( value_ ) ) );
  }
  bool pop( T& value_ ) {
    MPSCNode<T>* first = const_cast<MPSCNode<T>*>( load_first( std::memory_order_relaxed ) );
    MPSCNode<T>* next = const_cast<MPSCNode<T>*>( first->load_next( std::memory_order_acquire ) );
    if( next ) {
      store_first( next, std::memory_order_relaxed );
      assign_or_move( value_, next->get_mutable_value() );
      delete first;
      return true;
    }
    return false;
  }
  bool waiting_pop( T& value_ ) {
    do {
      MPSCNode<T>* first = const_cast<MPSCNode<T>*>( load_first( std::memory_order_relaxed ) );
      MPSCNode<T>* next = const_cast<MPSCNode<T>*>( first->load_next( std::memory_order_acquire ) );
      if( next ) {
        store_first( next, std::memory_order_relaxed );
        assign_or_move( value_, next->get_mutable_value() );
        delete first;
        return true;
      }
    } while( push_in_process() );
    return false;
  }
private:
  inline static void assign_value( T& to_, T& from_ ) { to_ = from_; }
  inline static void move_value( T& to_, T& from_ ) { to_ = std::move( from_ ); }
  inline static void swap_value( T& to_, T& from_ ) { std::swap( to_, from_ ); }
  // override for particular T where more efficient:
  inline static void assign_or_move( T& to_, T& from_ ) { move_value( to_, from_ ); }
  const MPSCNode<T>* load_first( std::memory_order order_ ) const {
    return static_cast<const MPSCNode<T>*>( MPSCQueueBase::load_first( order_ ) );
  }
  //inline static bool is_valued( const T& val_ ) { return false; }
private:
  const MPSCNode<T>*  _stub;
};

} //namespace znl

#endif //ZNL_MPSC_QUEUE_HPP_INCLUDED
