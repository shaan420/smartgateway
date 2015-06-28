#!/bin/sh
aclocal;
autoconf;
automake -a;
. ./cmd2;
make;
