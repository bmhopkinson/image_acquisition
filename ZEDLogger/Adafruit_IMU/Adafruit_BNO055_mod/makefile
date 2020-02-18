CPPFLAGS=-std=c++11 -g 
LDFLAGS=-g
LDLIBS=
adaimy_test: adaimu_test.o
	g++ $(LDFLAGS) -o adaimu_test adaimu_test.o $(LDLIBS)

adaimu_test.o: adaimu_test.cpp
	g++ $(CPPFLAGS) -c adaimu_test.cpp

