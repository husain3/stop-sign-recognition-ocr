all: g++ `pkg-config --cflags --libs opencv` stopSignRecognition.cc  -o opencv
