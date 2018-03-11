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
  SLinkable( const SLinkable& ) = delete;
  SLinkable( SLinkable&& ) = delete;
  SLinkable& operator=( const SLinkable& ) = delete;
  SLinkable& operator=( SLinkable&& ) = delete;
protected:
  SLinkable() = default;
  friend class MPSCIntrQueueBase;
  const std::atomic<SLinkable*>& next() const { return _next; }
private:
  friend class MPSCQueueBase;
  std::atomic<SLinkable*>& mutable_next() const { return _next; }
private:
  mutable std::atomic<SLinkable*> _next;
};

template<typename T> class MPSCQueue;

template<typename T>
class MPSCNode : public SLinkable
{
public:
  MPSCNode() = default;
  explicit MPSCNode( const MPSCNode& node_ ) : _value( node_._value ) {}
  explicit MPSCNode( MPSCNode&& node_ ) : _value( std::move( node_._value ) ) {}
  explicit MPSCNode( const T& value_ ) : _value( value_ ) {}
  explicit MPSCNode( T&& value_ ) : _value( std::move( value_ ) ) {}
  void assign( const MPSCNode& node_ ) { _value = node_._value; }
  void assign( MPSCNode&& node_ ) { _value = std::move( node_._value ); }
  MPSCNode& operator=( const MPSCNode& node_ ) { assign( node_ ); return *this; }
  MPSCNode& operator=( MPSCNode&& node_ ) { assign( node_ ); return *this; }
  void set_value( const T& value_ ) { _value = value_; }
  void set_value( T&& value_ ) { _value = std::move( value_ ); }
  const T& get_value() const { return _value; }
  T& get_value() { return _value; }
  //protected:
  //T&& get_value() && { return std::move( _value ); }
  //T&& get_move_value() & { return std::move( _value ); }
private:
  friend class MPSCQueue<T>;
  MPSCNode* load_next( std::memory_order order_ ) {
    return static_cast<MPSCNode*>( SLinkable::next().load( order_ ) );
  }
private:
  T _value;
};

class MPSCQueueBase
{
protected:
  MPSCQueueBase( SLinkable& stub_ ) : _first( &stub_ ), _last( &stub_ ) {
    stub_.mutable_next().store( nullptr, std::memory_order_relaxed );
  }
  void push( SLinkable& linkable_ ) {
    linkable_.mutable_next().store( nullptr, std::memory_order_relaxed );
    SLinkable* prev = _last.exchange( &linkable_, std::memory_order_acq_rel ); // may block
    prev->mutable_next().store( &linkable_, std::memory_order_release );
  }
  void store_first( SLinkable* linkable_, std::memory_order order_ ) { _first.store( linkable_, order_ ); }
  SLinkable* load_first( std::memory_order order_ ) const { return _first.load( order_ ); }
  SLinkable* load_last( std::memory_order order_ ) const { return _last.load( order_ ); }
private:
  std::atomic<SLinkable*> _last;
  std::atomic<SLinkable*> _first;
};

// Intrusive queue

class MPSCIntrQueueBase : public MPSCQueueBase
{
protected:
  MPSCIntrQueueBase() : MPSCQueueBase( _stub ) {}
  SLinkable* pop() {
    SLinkable* first = load_first( std::memory_order_relaxed );
    SLinkable* next = first->next().load( std::memory_order_acquire );
    if( first == &_stub ) {
      if( 0 == next ) {
        return nullptr;
      }
      store_first( next, std::memory_order_acq_rel );
      first = next;
      next = first->next().load( std::memory_order_acq_rel );
    }
    if( next ) {
      store_first( next, std::memory_order_acq_rel );
      return first;
    }
    if( first == load_last( std::memory_order_acq_rel ) ) {
      push( _stub );
      if( ( next = first->next().load( std::memory_order_acq_rel ) ) ) {
        store_first( next, std::memory_order_acq_rel );
        return first;
      }
    }
    return nullptr;
  }
private:
  SLinkable _stub;
};

template<class Node>
class MPSCIntrQueue : public MPSCIntrQueueBase
{
public:
  MPSCIntrQueue() = default;
  void push( Node& node_ ) { MPSCIntrQueueBase::push( node_ ); }
  Node* pop() { return static_cast<Node*>( MPSCIntrQueueBase::pop() ); }
};

// Non-intrusive queue

template<typename T>
class MPSCQueue : public MPSCQueueBase
{
public:
  MPSCQueue() : MPSCQueueBase( *( _stub = new MPSCNode<T>() ) ) {}
  ~MPSCQueue() {
    MPSCNode<T>* first = load_first( std::memory_order_relaxed );
    MPSCNode<T>* next;
    for( MPSCNode<T>* node = first; node; node = next ) {
      next = node->load_next( std::memory_order_relaxed );
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
    MPSCNode<T>* first = load_first( std::memory_order_relaxed );
    MPSCNode<T>* next = first->load_next( std::memory_order_acquire );
    if( next ) {
      store_first( next, std::memory_order_relaxed );
      assign_or_move( value_, next->get_value() );
      delete first;
      return true;
    }
    return false;
  }
  bool waiting_pop( T& value_ ) {
    do {
      MPSCNode<T>* first = load_first( std::memory_order_relaxed );
      MPSCNode<T>* next = first->load_next( std::memory_order_acquire );
      if( next ) {
        store_first( next, std::memory_order_relaxed );
        assign_or_move( value_, next->get_value() );
        delete first;
        return true;
      }
    } while( false ); //TODO: is_valued()
    return false;
  }
private:
  inline static void assign_or_move( T& to_, const T& from_ ) { to_ = from_; }
  inline static bool is_valued( const T& val_ ) { return false; }
  MPSCNode<T>* load_first( std::memory_order order_ ) {
    return static_cast<MPSCNode<T>*>( MPSCQueueBase::load_first( order_ ) );
  }
private:
  MPSCNode<T>*  _stub;
};

} //namespace znl

#endif //ZNL_MPSC_QUEUE_HPP_INCLUDED
