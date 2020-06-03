#!/usr/bin/env bash

# Usage
# ...: {input file 1}...{last input file} {signature dir}


# The last element is the output directory
OUTDIR="${@:$#}"

mkdir -p "$OUTDIR"

for file in "${@:1:$#-1}"; do
    BASENAME=$(basename "$file")
    SIGNATURE_FILE="$OUTDIR/$BASENAME.sig"

    echo "Processing $BASENAME signature"

    if [ ! -f "$SIGNATURE_FILE" ]; then
        ffmpeg -i "$file" -hide_banner -filter_complex \
            signature=detectmode=off:filename="$SIGNATURE_FILE":nb_inputs=1 \
            -f null -

        echo "Added $BASENAME signature"
    fi
done
