#!/bin/bash

# script

make

for i in {1..30}
do
  ./ep2 500 5 -benchmark $i command >output.txt  2>stderr.txt
  rm output.txt
  rm stderr.txt
done
for i in {1..30}
do
   ./ep2 500 500 -benchmark $i command >output.txt  2>stderr.txt
   rm output.txt
   rm stderr.txt
done
for i in {1..30}
do
   ./ep2 500 2500 -benchmark $i command >output.txt  2>stderr.txt
   rm output.txt
   rm stderr.txt
done
