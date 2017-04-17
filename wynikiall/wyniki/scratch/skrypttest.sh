#!/bin/bash
#skrypt
#PBS -q plgrid-testing
#PBS -l walltime=90:00:00
cd /people/plgklaudia1993/ns-3/ns-3-allinone/ns-3-dev
module add plgrid/tools/gcc/4.8.2
./waf configure
./waf --run scratch/802_11btest &> wyniki/100nodes80211b_defaulttest.out
echo "TO JA"
