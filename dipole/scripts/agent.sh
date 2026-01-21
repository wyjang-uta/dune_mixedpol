#!/bin/bash

# parse arguments from jobsub
MAG_FIELD=$1
SEED=$2
OUTPUT_FILE=$3
OUTPUT_DIR=$4

source /cvmfs/dune.opensciencegrid.org/products/dune/setup_dune.sh
source /cvmfs/larsoft.opensciencegrid.org/spack-packages/setup-env.sh
spack load expat@2.5.0
spack load geant4@10.6.1
spack load ifdhc@2.8.0
export G4DIR=$(spack location -i geant4@10.6.1)
export Geant4_DIR=$G4DIR/lib64/Geant4-10.6.1
export CMAKE_PREFIX_PATH=$G4DIR:$(spack location -i expat@2.5.0):$CMAKE_PREFIX_PATH
source $G4DIR/bin/geant4.sh

cd ${CONDOR_DIR_INPUT}
chmod +x ./dipole_defocus
./dipole_defocus ./run1.mac $MAG_FIELD $SEED $HOME/$OUTPUT_FILE

if [ -f "$HOME/$OUTPUT_FILE" ]; then
    echo "Transferring $OUTPUT_FILE to $OUTPUT_DIR..."
    ifdh cp $HOME/$OUTPUT_FILE ${OUTPUT_DIR}/${OUTPUT_FILE}
else
    echo "Error: Output file $HOME/$OUTPUT_FILE not found!"
    exit 1
fi