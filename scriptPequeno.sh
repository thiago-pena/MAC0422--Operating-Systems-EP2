#!/bin/bash

# script

make

for i in {1..30}
do
  ./ep2 250 5 -benchmark $i 2> stderr.txt
done
for i in {1..30}
do
   ./ep2 250 250 -benchmark $i 2> stderr.txt
done
for i in {1..30}
do
   ./ep2 250 1250 -benchmark $i 2> stderr.txt
done
