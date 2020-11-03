#!/bin/bash

# script

make

for i in {1..30}
do
    echo $i
  ./ep2 250 5 -benchmark $i command > output-1-$i.txt 2> stderr-1-$i.txt
  rm output.txt
  rm stderr.txt
done
for i in {1..30}
do
    echo $i
   ./ep2 250 250 -benchmark $i command >output-2-$i.txt  2> stderr-2-$i.txt
   rm output.txt
   rm stderr.txt
done
for i in {1..30}
do
    echo $i
   ./ep2 250 1250 -benchmark $i command >output-3-$i.txt  2> stderr-3-$i.txt
   rm output.txt
   rm stderr.txt
done
