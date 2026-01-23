#!/bin/bash

MAG_FIELD=1.5
RUN_NUM=2
NSUBRUNS=100

# Submit a job to the grid
BASE_APP_DIR="/exp/dune/app/users/wyjang"
BASE_DATA_DIR="/pnfs/dune/scratch/users/wyjang/dune/mirage/dipole"
EXE_FILE="$BASE_APP_DIR/bin/mirage"
MACRO_FILE="$BASE_APP_DIR/share/dune/mirage/dipole/macros/POT_100k.mac"
OUTPUT_DATA_DIR="$BASE_DATA_DIR/run${RUN_NUM}"

ifdh ls $OUTPUT_DATA_DIR >/dev/null 2>&1 || ifdh mkdir_p $OUTPUT_DATA_DIR
OUTPUT_FILE="result_${RUN_NUM}_${SEED}_${MAG_FIELD}.root"

echo "Preparing job assets ... "
ASSET_FILE_NAME="scripts_$(date +%Y%m%d_%H%M).tar.gz"
echo "Target: ${ASSET_FILE_NAME}"
echo "📦 Packing your scripts into a suitcase ($ASSET_FILE_NAME)..."
tar -cvzf $ASSET_FILE_NAME -C $BASE_APP_DIR/bin mirage -C $BASE_APP_DIR/share/dune/mirage/dipole/macros POT_100k.mac

if [ $? -ne 0 ]; then
    echo "Error: Failed to create tarball."
    exit 1
fi

# 3. Compression result
ORIG_SIZE_BYTES=$(stat -c%s "$EXE_FILE" "$MACRO_FILE" | awk '{s+=$1} END {print s}')
ORIG_SIZE=$((ORIG_SIZE_BYTES / 1024))
COMP_SIZE=$(du -k "$ASSET_FILE_NAME" | awk '{print $1}')

# 4. 결과 출력 및 비교
echo "-------------------------------------------------------"
echo "✅ Tarball creation successful!"
echo "📊 Original size: ${ORIG_SIZE} KB"
echo "📊 Compressed size: ${COMP_SIZE} KB"
echo "📉 Compression ratio: $(awk "BEGIN {printf \"%.2f\", $COMP_SIZE/$ORIG_SIZE*100}")%"
echo "-------------------------------------------------------"

echo "🚀 Handing over the suitcase to the grid. -- Submitting Run $RUN_NUM with $NSUBRUNS subruns ... "
jobsub_submit -N $NSUBRUNS \
    --memory=1000MB \
    --disk=500MB \
    --expected-lifetime=8h \
    --group=dune \
    --resource-provides=usage_model=OPPORTUNISTIC,DEDICATED \
    -f dropbox://$EXE_FILE \
    -f dropbox://$MACRO_FILE \
    file:///exp/dune/app/users/wyjang/share/dune/mirage/dipole/scripts/agent.sh \
    mirage POT_100k.mac $MAG_FIELD $RUN_NUM $NSUBRUNS $BASE_DATA_DIR
