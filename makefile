UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)
ifeq ($(UNAME_S),Darwin)
    ifeq ($(UNAME_M),arm64)
        INC=-I/opt/homebrew/include
        LIB=-L/opt/homebrew/lib
    else
        INC=-I/usr/local/include
        LIB=-L/usr/local/lib
    endif
else
    INC=
    LIB=
endif
OBJECTS=inetds.o proto.o

help:
	@echo " make serverds -- to build the TCP server"
	@echo " make inetclient -- to build the TCP client"
	@echo " make clean -- to remove unwanted files"

serverds: server.c ${OBJECTS}
	gcc -g $(INC) server.c ${OBJECTS} $(LIB) -lpthread -lconfig -Wall -o serverds

inetds.o: inetds2.c proto.o
	gcc -g $(INC) inetds2.c -c -o inetds.o -Wall

proto.o: proto.c proto.h
	gcc -g $(INC) proto.c -c -o proto.o -Wall

inetclient: inetsample2.c proto.o
	gcc -g $(INC) inetsample2.c proto.o -Wall -o inetclient

cv_worker: main.cpp io_utils.cpp preprocess.cpp duplicate_detection.cpp merge_utils.cpp
	g++ -std=c++17 main.cpp io_utils.cpp preprocess.cpp duplicate_detection.cpp merge_utils.cpp -o cv_worker `pkg-config --cflags --libs opencv4`
clean:
	rm -f ${OBJECTS} serverds inetclient


#pentru rularea pe Ubuntu/Debian Linux, instalati urmatoarele dependente
#sudo apt update
#sudo apt install build-essential libconfig-dev libopencv-dev pkg-config