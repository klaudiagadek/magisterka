#!/bin/bash
#skrypt
#PBS -q plgrid
#PBS -l walltime=70:00:00,nodes=1:ppn=1,mem=256mb
cd /people/plgklaudia1993/ns-3/ns-3-allinone/ns-3-dev
module add plgrid/tools/gcc/4.8.2
./waf configure
./waf --run scratch/802_11n_6 &> wynikin50/10nodes80211n_6b.out
echo "TO JA"
