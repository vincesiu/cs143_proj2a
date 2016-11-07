#!/bin/bash -xu

INPUT=${1:-""}
UID=904280752


function cleanup {
    rm -rf ./${UID}
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
    mkdir ./${UID}
    for FILE in ${REQUIRED_FILES[@]}; do
        cp $FILE ./${UID}/$(basename $FILE)
    done
    zip -r P2.zip ./${UID}/
    bash <(curl -sL http://oak.cs.ucla.edu/classes/cs143/project/p2_test) ${UID}
    mv P2.zip ~/www/P2.zip
fi
