#!/bin/bash

# script

make

for i in {1..30}
do
   ./ep2 1200 1200 -benchmark $i command >output$i.txt  2>stderr$i.txt
   rm output.txt
   rm stderr.txt
done
for i in {1..30}
do
  ./ep2 1200 5 -benchmark $i command >output.txt  2>stderr.txt
  rm output.txt
  rm stderr.txt
done
for i in {1..30}
do
   ./ep2 1200 6000 -benchmark $i command >output.txt  2>stderr.txt
   rm output.txt
   rm stderr.txt
done
