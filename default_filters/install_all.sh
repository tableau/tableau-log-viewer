#!/bin/bash

# find the location the filters live
BASEDIR=$(dirname "$0")
FILES=/filters/*
FILTERS=$BASEDIR$FILES

# make the filters directory if it doesn't exist
DEST=~/Library/Preferences/Tableau/TLV/filters/
mkdir -p $DEST

# go through and copy all the filters
for f in $FILTERS
do
	cp "$f" "$DEST"
	echo "$f"
done