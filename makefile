CPPFLAGS = -O3 -std=c++17
CXX = g++

.PHONY: test
test: test.out
	./test.out

test.out: test.cpp wav_logger.hpp wav_logger.o
	$(CXX) -o test.out test.cpp wav_logger.o $(CPPFLAGS)

wav_logger.o: wav_logger.hpp wav_logger.cpp
	$(CXX) -c wav_logger.cpp "-DMODULATE_MAX_LOG_FILE_LENGTH=(1<<18)" $(CPPFLAGS)

.PHONY: clean
clean:
	rm -rf *~ .*~ *.o *.out *.wav logs async_logs threaded_async_logs *.dSYM
