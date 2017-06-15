#!/bin/bash
#skrypt
#PBS -q plgrid
#PBS -l walltime=70:00:00,nodes=1:ppn=1,mem=128mb
cd /people/plgklaudia1993/ns-3/ns-3-allinone/ns-3-dev
module add plgrid/tools/gcc/4.8.2
./waf configure
./waf --run scratch/50n802_11a1_54Mbps &> wynikia50/50nodes80211a54.out
echo "TO JA"
