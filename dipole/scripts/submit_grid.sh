#!/bin/bash

# Submit a job to the grid

BASE_APP_DIR="/exp/dune/app/users/wyjang"
#BASE_DATA_DIR="/exp/dune/data/users/wyjang"
BASE_DATA_DIR="/pnfs/dune/scratch/users/wyjang/dune_mixedpol/dipole"

EXE_PATH="$BASE_APP_DIR/bin/dipole_defocus"
MACRO_FILE="$BASE_APP_DIR/share/dune_mixedpol/dipole/macros/run1.mac"
OUTPUT_DATA_DIR="$BASE_DATA_DIR/dune_mixedpol/dipole"
MAG_FIELD=3.0

ifdh mkdir_p $OUTPUT_DATA_DIR
for RUN_NUM in {1..100}
do
    SEED=$((RUN_NUM))
    OUTPUT_FILE="result_${RUN_NUM}_${SEED}_${MAG_FIELD}.root"

    echo "Submitting Run $RUN_NUM with seed $SEED ..."

    jobsub_submit -N 1 \
    --memory=2000MB \
    --disk=1000MB \
    --expected-lifetime=8h \
    --group=dune \
    --resource-provides=usage_model=OPPORTUNISTIC,DEDICATED \
    -f dropbox://$EXE_PATH \
    -f dropbox://$MACRO_FILE \
    file:///exp/dune/app/users/wyjang/share/dune_mixedpol/dipole/scripts/agent.sh \
    $MAG_FIELD $SEED $OUTPUT_FILE $OUTPUT_DATA_DIR

done