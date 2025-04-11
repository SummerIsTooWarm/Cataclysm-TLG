#!/bin/bash

for file in *.json; do
    awk '{
        print
        if ($0 ~ /"coverage": *[0-9]+,/) {
            match($0, /"coverage": *([0-9]+),/, m)
            print "  \"concealment\": " m[1] ","
        }
    }' "$file" > "tmp_$file"
    mv "tmp_$file" "$file"
done

