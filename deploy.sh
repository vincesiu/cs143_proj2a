#!/bin/bash -xu

INPUT=${1:-""}
BRUIN_ID=904280752


function cleanup {
    rm -rf ./${BRUIN_ID}
    echo "Finished deploy script"
}

trap cleanup EXIT

REQUIRED_FILES=( \
        readme.txt \
        team.txt \
        bruinbase/* \
        )
#        bruinbase/Makefile \
#        bruinbase/*.{cc,c,h,y,l,tbl} \
#        )

if [[ $INPUT = "--submit" ]]; then
    mkdir ./${BRUIN_ID}
    for FILE in ${REQUIRED_FILES[@]}; do
        cp $FILE ./${BRUIN_ID}/$(basename $FILE)
    done
    zip -r P2.zip ./${BRUIN_ID}/
    bash <(curl -sL http://oak.cs.ucla.edu/classes/cs143/project/p2_test) ${BRUIN_ID}
    mv P2.zip ~/www/P2.zip
fi
