#!/bin/bash

g++ shower.cc -o shower.exe `pythia8-config --cxxflags --libs` `root-config --cflags --glibs` `fastjet-config --cxxflags --libs` -lHepMC
