#!/bin/sh
set -eu

PROJECT_NAME=MapReduce

if [ -d "./bin" ] && [ -f "./input/AB_NYC_2019.csv" ]; then
    BINARY_HOME=./bin
    INPUT=./input/AB_NYC_2019.csv
else
    BINARY_HOME=/usr/bin
    INPUT=/usr/share/${PROJECT_NAME}/input/AB_NYC_2019.csv
fi

${BINARY_HOME}/mapper_mean < ${INPUT} \
    | sort -k1,1 \
    | ${BINARY_HOME}/reducer_mean > output_mean

${BINARY_HOME}/mapper_variance < ${INPUT} \
    | sort -k1,1 \
    | ${BINARY_HOME}/reducer_variance > output_variance

cat output_mean
cat output_variance