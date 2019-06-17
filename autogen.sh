#!/bin/bash

# Used to build the suite completely
# Aimed at developers, users should just use configure and make

autoreconf -ivf
./configure --prefix=$PWD
