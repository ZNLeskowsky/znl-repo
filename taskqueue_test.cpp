//  Actor / Task queue
//
//  Copyright (C) 2018 Zoltan N. Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include "actor.hpp"
#include "logger.hpp"
#include "taskqueue.hpp"
#include "worker.hpp"
#include <cstdlib>
//#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;
using namespace znl;

void func()
{
  Log.log( "func" );
}

int main()
{
  LOG( "__cplusplus = " << __cplusplus );
  //const std::function<void()> lambda( [=] () { Log.log( "task1" ); } );
  //Task task( lambda );
  Task task1( [=] () { Log.log( "task1" ); } );
  Task task2( [=] () { Log.log( "task2" ); } );
  Task task3( [=] () { Log.log( "task3" ); } );
  Task task2dummy( [=] () { Log.log( "task2dummy" ); } );
  Task functask( func );
  Task task2copy( std::forward<Task>( task2dummy ) ); //misassignment
  task2copy = std::forward<Task>( task2 ); //reassignment test
  TaskQueue queue;
  queue.push( task1 );
  const Task *ptask = queue.pop();
  if( ptask ) {
    ( *ptask )();
  }
  queue.push( task2copy );
  ptask = queue.pop();
  if( ptask ) {
    ( *ptask )();
  }
  queue.push( task3 );
  ptask = queue.pop();
  if( ptask ) {
    ( *ptask )();
  }
  ptask = queue.pop();
  if( ptask ) {
    ( *ptask )();
  }
  queue.push( task3 );
  queue.push( functask );
  ptask = queue.pop();
  if( ptask ) {
    ( *ptask )();
    *const_cast<Task*>( ptask ) = ( [=] () { Log.log( "reuse1" ); } );
    queue.push( *ptask );
    ptask = queue.pop();
    ( *ptask )();
  }
  ptask = queue.pop();
  if( ptask ) {
    ( *ptask )();
    *const_cast<Task*>( ptask ) = std::forward<Func>( func );
    queue.push( *ptask );
    ptask = queue.pop();
    ( *ptask )();
  }
  ptask = queue.pop();
  if( ptask ) {
    ( *ptask )();
  }

  const std::function<void()> func1( [=] () { Log.log( "func1" ); } );
  const std::function<void()> func2( [=] () { Log.log( "func2" ); } );
  const std::function<void()> func3( [=] () { Log.log( "func3" ); } );
  const std::function<void()> func4( [=] () { Log.log( "func4" ); } );
  FuncQueue fqueue;
  fqueue.push( func1 );
  fqueue.push( func2 );
  std::function<void()> ofunc;
  if( fqueue.pop( ofunc ) ) {
     ofunc();
  }
  fqueue.push( func3 );
  if( fqueue.pop( ofunc ) ) {
     ofunc();
  }
  if( fqueue.pop( ofunc ) ) {
     ofunc();
  }
  if( fqueue.pop( ofunc ) ) {
     ofunc();
  }
  fqueue.push( func3 );
  if( fqueue.pop( ofunc ) ) {
     ofunc();
  }
  fqueue.push( func4 );
  if( fqueue.pop( ofunc ) ) {
     ofunc();
  }

  if( 0 )
  {
  Log.log( "Worker test" );
  Worker worker( "WorkerA" );
  worker.start( [] ( std::thread& ) -> int {
                  Log.log( "TODO: set thread affinity" );
                  return 0;
                } );
  LOG( "Worker " << worker.name() << " started" );
  worker.send( task1 );
  worker.send( task2 );
  worker.send( task3 );
  LOG( "Worker " << worker.name() << " sending stop" );
  worker.send_stop();
  //LOG( "Waiting until worker " << worker.name() << " stopped ..." );
  //worker.wait_until_stopped();
  //LOG( "Worker " << worker.name() << " stopped" );
  }
  //sleep( 1 );
  bool use_actors = true;
  if( use_actors )
  {
  const int nactors = 4;
  Actor actors[nactors];
  Log.log( "Actors created" );
  int ai, ti;
  std::vector<Task> tasks;
  //const int ntasks = 16;
  const int ntasks = 256;
  //const int ntasks = 1024;
  //const int ntasks = 4096;
  //const int ntasks = 16384;
  tasks.reserve( ntasks );
  for( ti = 0; ti < ntasks; ++ti ) {
    tasks.emplace_back( Task( [ti] () {
      LOG( "Running task " << ( ti + 1 ) );
      sleep( 1 );
      LOG( "Ran task " << ( ti + 1 ) );
    } ) );
  }
  Log.log( "Tasks created" );
  for( ti = 0; ti < ntasks/nactors; ++ti ) {
    for( ai = 0; ai < nactors; ++ai ) {
      int tiai = ( ti * nactors ) + ai;
      LOG( "Sending task" << ( tiai + 1 ) << " to actor" << ( ai + 1 ) );
      actors[ai].send( tasks[tiai] );
    }
  }
  //sleep( 180 );
  }
  else
  {
  const int nactors = 4;
  Worker actors[nactors];
  LOG( nactors << " workers created" );
  int ai, ti;
  for( ai = 0; ai < nactors; ++ai ) {
    std::stringstream ss;
    ss << "Worker" << ( ai + 1 );
    actors[ai].set_name( ss.str() );
    LOG( "Starting worker " << actors[ai].name() );
    actors[ai].start();
    LOG( actors[ai].name() << " started" );
  }
  LOG( nactors << " workers started" );
  std::vector<Task> tasks;
  const int ntasks = 16;
  //const int ntasks = 256;
  //const int ntasks = 1024;
  //const int ntasks = 4096;
  //const int ntasks = 16384;
  tasks.reserve( ntasks );
  LOG( "Creating " << ntasks << " tasks" );
  for( ti = 0; ti < ntasks; ++ti ) {
    LOG( "Creating task " << ( ti + 1 ) );
    tasks.push_back( Task( [ti] () {
      LOG( "Running task " << ( ti + 1 ) );
      sleep( 1 );
      LOG( "Ran task " << ( ti + 1 ) );
    } ) );
    LOG( "Task " << ( ti + 1 ) << " created" );
  }
  LOG( ntasks << " tasks created" );
  for( ti = 0; ti < ntasks/nactors; ++ti ) {
    for( ai = 0; ai < nactors; ++ai ) {
      int tiai = ( ti * nactors ) + ai;
      LOG( "Sending task" << ( tiai + 1 ) << " to actor" << ( ai + 1 ) );
      actors[ai].send( tasks[tiai] );
    }
  }
  //sleep( 180 );
  }
  //sleep( 1 );
  //sleep( 1 );
  return 0;
}
