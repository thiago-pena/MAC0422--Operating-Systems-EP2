#!/bin/bash

# script

make

for i in {1..30}
do
  ./ep2 250 5 -benchmark $i command >output.txt  2>stderr.txt
  rm output.txt
  rm stderr.txt
done
for i in {1..30}
do
   ./ep2 250 250 -benchmark $i command >output.txt  2>stderr.txt
   rm output.txt
   rm stderr.txt
done
for i in {1..30}
do
   ./ep2 250 1250 -benchmark $i command >output.txt  2>stderr.txt
   rm output.txt
   rm stderr.txt
done
