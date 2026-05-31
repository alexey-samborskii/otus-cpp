#!/bin/sh
set -eu

BINARY_HOME=./bin
INPUT_HOME=./input
INPUT=${INPUT_HOME}/AB_NYC_2019.csv

${BINARY_HOME}/mapper_mean < ${INPUT} \
    | sort -k1,1 \
    | ${BINARY_HOME}/reducer_mean > output_mean

${BINARY_HOME}/mapper_variance < ${INPUT} \
    | sort -k1,1 \
    | ${BINARY_HOME}/reducer_variance > output_variance

cat output_mean
cat output_variance
