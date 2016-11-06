#!/bin/bash -xu

INPUT=${1:-""}


function cleanup {
    rm -rf ./904280752
    echo "Finished deploy script"
}

trap cleanup EXIT

REQUIRED_FILES=( \
        readme.txt \
        team.txt \
        bruinbase/Makefile \
        bruinbase/*.{cc,c,h,y,l} \
        )

if [[ $INPUT = "--submit" ]]; then
    mkdir ./904280752
    for FILE in ${REQUIRED_FILES[@]}; do
        cp $FILE ./904280752/$(basename $FILE)
    done
    zip -r P2.zip ./904280752/
    bash <(curl -sL http://oak.cs.ucla.edu/classes/cs143/project/p2_test) 904280752
    mv P2.zip ~/www/P2.zip
fi
