#!/usr/bin/env sh
c++ -std=c++11 -pthread -o taskqueue_test src/worker.cpp src/actor.cpp src/logger.cpp src/taskqueue_test.cpp 2>&1 |tee make_taskqueue_test.out
