#!/bin/bash

for file in *.json; do
    sed 's/"concealment": 60,/"concealment": 38,/' "$file" > "tmp_$file"
    mv "tmp_$file" "$file"
done

