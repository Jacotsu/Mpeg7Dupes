#!/usr/bin/env bash

# Usage
# ...: {input file 1}...{last input file} {signature dir}


# The last element is the output directory
OUTDIR="${@:$#}"

mkdir -p "$OUTDIR"

# $1 file
# $2 out dir
generate_signature () {
    BASENAME=$(basename "$1")
    SIGNATURE_FILE="$2/$BASENAME.sig"

    echo "Processing $BASENAME signature"

    if [ ! -f "$SIGNATURE_FILE" ]; then
        ffmpeg -i "$1" -hide_banner -filter_complex \
            signature=detectmode=off:filename="$SIGNATURE_FILE":nb_inputs=1 \
            -f null -

        echo "Added $BASENAME signature"
    fi
}

#export -f generate_signature
#parallel generate_signature ::: "${@:1:$#-1}" ::: "$OUTDIR"

for file in "${@:1:$#-1}"; do
    generate_signature "$file" "$OUTDIR"
done
