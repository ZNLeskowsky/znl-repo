//  Logger
//
//  Copyright (C) 2018 Zoltan N. Leskowsky
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZNL_LOGGER_HPP_INCLUDED
#define ZNL_LOGGER_HPP_INCLUDED

#define ACTOR_LOGGER 1
#if ACTOR_LOGGER
#include "actor.hpp"
#else
#include "worker.hpp"
#endif
#include <iostream>
#include <sstream>
#include <string>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif


#if defined(_MSC_VER)
#endif


namespace znl {

#if ACTOR_LOGGER
typedef Actor LoggerBase;
#else
typedef Worker LoggerBase;
#endif

//template<typename LoggerBase = Actor>
class Logger : public LoggerBase
{
public:
  Logger( const std::string& name_, bool start_ = true ) : LoggerBase( name_ ) {
    if( start_ ) {
      start();
    }
  }
  void start() {
      log( std::string( "Logger " ) + name() + " logging" );
#if !ACTOR_LOGGER
      LoggerBase::start();
#endif
  }
  void log( const std::string& msg_ ) {
      //send( *new Task( [=] () { std::cout << msg_ << std::endl << std::flush; } ) );
      send( Func( [=] () { std::cout << msg_ << std::endl << std::flush; } ) );
  }
  Logger& operator<<( const std::string& msg_ ) {
      log( msg_ ); return *this;
  }
};

extern Logger Log;

} //namespace znl

#define LOG( MSG ) { std::stringstream ss; ss << MSG; znl::Log.log( ss.str() ); }

#endif //ZNL_LOGGER_HPP_INCLUDED
