CC=g++
CFLAGS=-std=c++17 -O3 -march=native -Wall -Wextra -Iinclude



SRCDIR=src
TARGETDIR=bin
BUILDDIR=build
HEADERDIR=include

UFILES=$(HEADERDIR)/usuffix.cpp $(HEADERDIR)/usuffix.h

all: main suffixalgo brute

main: $(SRCDIR)/main.cpp $(UFILES)
	$(CC) $(CFLAGS)  $(SRCDIR)/main.cpp -o $(TARGETDIR)/main

suffixalgo: $(SRCDIR)/substring/suffixalgo.cpp $(UFILES)
	$(CC) $(CFLAGS)  $(SRCDIR)/substring/suffixalgo.cpp -o $(TARGETDIR)/substring/suffixalgo

brute: $(SRCDIR)/substring/brute.cpp
	$(CC) $(CFLAGS)  $(SRCDIR)/substring/brute.cpp -o $(TARGETDIR)/substring/brute


