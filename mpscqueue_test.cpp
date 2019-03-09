//  Lock-free MPSC intrusive and non-intrusive queues based on
//  Vyukov, Dmitry,
//  http://www.1024cores.net/home/lock-free-algorithms/queues/
//
//  Copyright (C) 2018 Zoltan N. Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include "mpscqueue.hpp"
#include <atomic>
#include <cassert>
#include <cstring>
#include <future>
#include <iostream>
#include <string>
#include <thread>

using namespace std;
using namespace znl;
using namespace znl::detail;

class IntNode : public MPSCNode<int>
{
public:
  IntNode( int i_ = 0 ) : MPSCNode<int>( i_ ) {}
  operator int() const { return get_value(); }
};

using IntrIntQueue = MPSCIntrQueue<IntNode>;
using IntQueue = MPSCQueue<int>;

int main()
{
  constexpr int NTHR = 8;
  constexpr int NITEM = 32;

  //std::atomic_flag    _lock;
  //std::atomic<int>    _count;
  std::future<void>   _futures[NTHR];

  cout << "Intrusive queue tests ..." << endl;
  IntNode nnode( 1 );
  IntrIntQueue inqueue;
  inqueue.push( nnode );
  const MPSCNode<int> *pnnode = inqueue.pop();

  IntNode ins[NITEM];
  int i = NITEM;
  assert( ( NITEM <= 36 ) && ( NITEM > 0 ) );
  while( i-- ) {
    ins[i] = ( i < 10 ) ? ( '0' + i ) : ( 'a' + i - 10 );
  }

  IntrIntQueue iiqueue;
  i = 0;
  iiqueue.push( ins[0] ); ++i;
  const IntNode *pi = iiqueue.pop();
  if( pi ) {
     cout << static_cast<char>( pi->get_value() ); --i;
  }
  iiqueue.push( ins[1] ); ++i;
  pi = iiqueue.pop();
  if( pi ) {
     cout << static_cast<char>( pi->get_value() ); --i;
  }
  iiqueue.push( ins[2] ); ++i;
  pi = iiqueue.pop();
  if( pi ) {
     cout << static_cast<char>( pi->get_value() ); --i;
  }
  pi = iiqueue.pop();
  if( pi ) {
     cout << static_cast<char>( pi->get_value() ); --i;
  }
  iiqueue.push( ins[2] ); ++i;
  iiqueue.push( ins[3] ); ++i;
  pi = iiqueue.pop();
  if( pi ) {
     cout << static_cast<char>( pi->get_value() ); --i;
  }
  pi = iiqueue.pop();
  if( pi ) {
     cout << static_cast<char>( pi->get_value() ); --i;
  }
  cout << endl;
  cout << i << " items remaining" << endl;
  if( i > 0 ) {
    do {
      pi = iiqueue.pop();
      if( pi ) {
        cout << static_cast<char>( pi->get_value() ); --i;
      } else {
        cout << "N|";
      }
    } while( i );
    cout << endl;
  }

  cout << "launching " << NTHR << " threads" << endl;
  for( i = 0; i < NTHR; ++i ) {
    _futures[i] = std::move( std::async( std::launch::async,
                               [&iiqueue, &ins, i] () {
                                 for( int j = 0; j < NITEM; j += NTHR ) {
                                   iiqueue.push( ins[j+i] );
                                 }
                               } ) );
    cout << "t" << i << endl;
    //cout << "launched thread " << i << endl;
    //_futures[i].wait();
  }
  cout << "launched " << i << " threads" << endl;
  i = NITEM;
  int inullcount = 0;
  int inulli = 0;
  while( i > 0 ) {
    pi = iiqueue.pop();
    if( pi ) {
      if (inulli) {
        cout << " - " << inulli << " nulls - ";
        inullcount += inulli;
        inulli = 0;
      }
      cout << static_cast<char>( pi->get_value() ); --i;
    } else {
       ++inulli;
    }
  }
  cout << endl;
  cout << "popped " << NITEM << " items" << endl;
  for( i = 0; i < NTHR; ++i ) {
    if( _futures[i].valid() )
      _futures[i].wait();
  }
  cout << "awaited " << NTHR << " threads" << endl;

  cout << "Non-intrusive queue tests ..." << endl;
  IntQueue iqueue;
  iqueue.push( 1 );
  iqueue.push( 2 );
  int ni;
  bool popped;
  popped = iqueue.pop( ni );
  if( popped ) {
     cout << ni << endl;
  }
  iqueue.push( 3 );
  popped = iqueue.pop( ni );
  if( popped ) {
     cout << ni << endl;
  }
  cout << "here A" << std::endl << std::flush;
  popped = iqueue.pop( ni );
  if( popped ) {
     cout << ni << endl;
  }
  cout << "here B" << std::endl << std::flush;
  popped = iqueue.pop( ni );
  if( popped ) {
     cout << ni << endl;
  }
  cout << "here C" << std::endl << std::flush;
  iqueue.push( 3 );
  popped = iqueue.pop( ni );
  if( popped ) {
     cout << ni << endl;
  }
  iqueue.push( 4 );
  cout << "here D" << std::endl << std::flush;
  popped = iqueue.pop( ni );
  if( popped ) {
     cout << ni << endl;
  }

  int is[NITEM];
  i = NITEM;
  while( i-- ) {
    is[i] = ( i < 10 ) ? ( '0' + i ) : ( 'a' + i - 10 );
  }
  cout << "launching " << NTHR << " threads" << endl;
  for( i = 0; i < NTHR; ++i ) {
    _futures[i] = std::move( std::async( std::launch::async,
                               [&iqueue, &is, i] () {
                                 for( int j = 0; j < NITEM; j += NTHR ) {
                                   iqueue.push( is[j+i] );
                                 }
                               } ) );
    cout << "t" << i << endl;
    //cout << "launched thread " << i << endl;
    //_futures[i].wait();
  }
  cout << "launched " << i << " threads" << endl;
  i = NITEM;
  int ninullcount = 0;
  int ninulli = 0;
  while( i > 0 ) {
    popped = iqueue.pop( ni );
    if( popped ) {
      if (ninulli) {
        cout << " - " << ninulli << " nulls - ";
        ninullcount += ninulli;
        ninulli = 0;
      }
       cout << static_cast<char>( ni ); --i;
    } else {
       ++ninulli;
    }
  }
  cout << endl;
  cout << "popped " << NITEM << " items" << endl;
  for( i = 0; i < NTHR; ++i ) {
    if( _futures[i].valid() )
      _futures[i].wait();
  }
  cout << "awaited " << NTHR << " threads" << endl;
  cout << "Intrusive null-count: " << inullcount << endl;
  cout << "Non-intrusive null-count: " << ninullcount << endl;
}
