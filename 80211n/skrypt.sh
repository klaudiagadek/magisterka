#!/bin/bash
#skrypt
#PBS -q plgrid
#PBS -l walltime=70:00:00
cd /people/plgklaudia1993/ns-3/ns-3-allinone/ns-3-dev
module add plgrid/tools/gcc/4.8.2
./waf configure
./waf --run scratch/802_11n1 &> wynikin/10nodes80211n_default.out
echo "TO JA"
