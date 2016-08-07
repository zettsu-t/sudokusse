#!/usr/bin/sh

echo -e "\e[96mTesting with $1\e[0m"
# Pass an exit code of the executable $1 to Makefile
"$1"
