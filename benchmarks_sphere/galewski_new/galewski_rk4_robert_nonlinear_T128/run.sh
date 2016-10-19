#! /bin/bash

BASEDIR="`pwd`"
rm -f ./prog_*

SPHROOT="../../../"
cd "$SPHROOT"

make clean
scons --program=swe_sph_and_rexi --gui=disable --sphere-spectral-space=enable --threading=omp --mode=release -j4

cd "$BASEDIR"

# h0=g=f=1

# Stable time step size for T64
TS=30

# unstable for explicit RK4 time stepping
#TS=0.01

OTS=$((120*20))

RES=128
#RES=16

REXI_M=0	# Deactivate REXI
REXI_H=0.2
REXI_HALF_POLES=1
REXI_EXTENDED_MODES=4

BENCH_ID=1
USE_ROBERT_FUNS=1

NONLINEAR=1
VISCOSITY=100000

SIMTIME=720000


$SPHROOT/build/swe_sph_and_rexi_libfft_dealiasing_omp_gnu_release -M $RES -p $USE_REXI -k $NONLINEAR -C -$TS -o $OTS -u $VISCOSITY -t $SIMTIME --rexi=0 --rexi-m=$REXI_M --rexi-h=$REXI_H --rexi-half=$REXI_HALF_POLES -s $BENCH_ID -l $USE_ROBERT_FUNS --rexi-ext-modes $REXI_EXTENDED_MODES


#mv prog_* "$BASEDIR"

$SPHROOT/plot_csv.py prog_h_*.csv
$SPHROOT/create_mp4.sh prog_h out_prog_h.mp4

$SPHROOT/plot_csv.py prog_eta_*.csv
$SPHROOT/create_mp4.sh prog_eta out_prog_eta.mp4
