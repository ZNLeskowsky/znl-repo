#!/usr/bin/env sh
SRC="$( cd "$( dirname $0 )" && pwd )"
c++ -std=c++11 -pthread -o taskqueue_test $SRC/worker.cpp $SRC/actor.cpp $SRC/logger.cpp $SRC/taskqueue_test.cpp 2>&1 |tee make_taskqueue_test.out
