#!/bin/bash

cp out_web/* ../boxfight-ts/client/build/
rm ../boxfight-ts/client/build/index.html
mv ../boxfight-ts/client/build/main.html ../boxfight-ts/client/build/index.html