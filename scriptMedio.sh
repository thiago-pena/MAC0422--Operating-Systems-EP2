#!/bin/bash

# script

make

for i in {1..30}
do
  ./ep2 500 5 -benchmark $i 2> stderr.txt
done
for i in {1..30}
do
   ./ep2 500 500 -benchmark $i 2> stderr.txt
done
for i in {1..30}
do
   ./ep2 500 2500 -benchmark $i 2> stderr.txt
done
