#!/bin/bash

PWR=$(pwd)

#
# threadsafe mode is serialized
#
SQLITE_THREADSAFE=2 ./configure --prefix=$PWD/sql3_install --exec-prefix=$PWD/sql3_install --disable-shared
