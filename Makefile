SRC=.
OBJ=.
CPPFLAGS=-std=c++11
#CPPFLAGS=-std=c++11 -Wc++1z-extensions

${OBJ}/mpscqueue_test: ${OBJ}/mpscqueue.o ${SRC}/mpscqueue_test.cpp ${SRC}/mpscqueue.hpp
	c++ ${CPPFLAGS} -pthread ${OBJ}/mpscqueue.o -o ${OBJ}/mpscqueue_test ${SRC}/mpscqueue_test.cpp

${OBJ}/mpscqueue.o: ${SRC}/mpscqueue.cpp ${SRC}/mpscqueue.hpp
	c++ ${CPPFLAGS} -c ${SRC}/mpscqueue.cpp -o ${OBJ}/mpscqueue.o

clean_mpsc:
	rm -f ${OBJ}/mpscqueue.o ${OBJ}/mpscqueue_test

${OBJ}/taskqueue_test: ${OBJ}/actor.o ${OBJ}/worker.o ${OBJ}/logger.o ${OBJ}/mpscqueue.o ${SRC}/taskqueue_test.cpp ${SRC}/taskqueue.hpp
	c++ ${CPPFLAGS} -pthread ${OBJ}/actor.o ${OBJ}/worker.o ${OBJ}/logger.o ${OBJ}/mpscqueue.o -o ${OBJ}/taskqueue_test ${SRC}/taskqueue_test.cpp

${OBJ}/actor.o: ${SRC}/actor.cpp ${SRC}/actor.hpp ${SRC}/atomiclock.hpp ${SRC}/taskqueue.hpp
	c++ ${CPPFLAGS} -c ${SRC}/actor.cpp -o ${OBJ}/actor.o

${OBJ}/worker.o: ${SRC}/worker.cpp ${SRC}/worker.hpp ${SRC}/atomiclock.hpp ${SRC}/logger.hpp ${SRC}/taskqueue.hpp
	c++ ${CPPFLAGS} -c ${SRC}/worker.cpp -o ${OBJ}/worker.o

${OBJ}/logger.o: ${SRC}/logger.cpp ${SRC}/logger.hpp ${SRC}/atomiclock.hpp ${SRC}/actor.hpp ${SRC}/worker.hpp
	c++ ${CPPFLAGS} -c ${SRC}/logger.cpp -o ${OBJ}/logger.o

clean_task:
	rm -f ${OBJ}/actor.o ${OBJ}/worker.o ${OBJ}/logger.o ${OBJ}/mpscqueue.o ${OBJ}/taskqueue_test

clean: clean_mpsc clean_task

