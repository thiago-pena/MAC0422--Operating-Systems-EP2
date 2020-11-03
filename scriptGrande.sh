#!/bin/bash

# script

make

for i in {1..30}
do
  ./ep2 1200 5 -benchmark $i 2> stderr.txt
done
for i in {1..30}
do
   ./ep2 1200 1200 -benchmark $i 2> stderr.txt
done
for i in {1..30}
do
   ./ep2 1200 6000 -benchmark $i 2> stderr.txt
done
