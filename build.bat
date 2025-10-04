@echo off
setlocal

set CXX=g++
set CXXFLAGS=-Wall -O3 -Wextra -std=c++20 -Iinclude
set SRC_DIR=src
set OUT=bin\sweeper.exe

if not exist bin mkdir bin

%CXX% %CXXFLAGS% %SRC_DIR%\*.cpp -o %OUT%

echo Built %OUT%
