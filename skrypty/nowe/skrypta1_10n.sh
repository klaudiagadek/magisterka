#!/bin/bash
#skrypt
#PBS -q plgrid
#PBS -l walltime=70:00:00,nodes=1:ppn=1,mem=128mb
cd /people/plgklaudia1993/ns-3/ns-3-allinone/ns-3-dev
module add plgrid/tools/gcc/4.8.2
./waf configure
./waf --run scratch/10n802_11a2_48Mbps &> wynikia/10nodes80211a48.out
echo "TO JA"
