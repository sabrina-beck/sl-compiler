#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NO_COLOR='\033[0m'

buildDir="build/"
testResultDir="${buildDir}tests/"

mkdir -p $testResultDir

for testFile in tests/sl/*; do

  testNumber=$(echo $testFile | sed -e 's/[^0-9]//g')
  echo -n "Running test $testNumber"

  expectedResponsePath=$(find tests/mep -regextype posix-egrep -regex ".*$testNumber.*")
  #expectedResponsePath=$(find tests/output -regextype posix-egrep -regex ".*$testNumber.*")
  resultProgram="${testResultDir}result$testNumber.mep"
  resultFile="${testResultDir}result$testNumber.res"
  inputFile="tests/input/data$testNumber.in"

  ./build/main < $testFile > $resultProgram
  #./build/mepa/mepa.py --silent --limit 12000 --progfile $resultProgram < $inputFile > $resultFile

  DIFF=$(diff -b $resultProgram $expectedResponsePath)
#  DIFF=$(diff $resultFile $expectedResponsePath)
  if [ "$DIFF" != "" ]
  then
    echo -e " | ${RED}FAILED${NO_COLOR}"
    diff -b -y --color $resultProgram $expectedResponsePath
    #diff --color $resultFile $expectedResponsePath
  else
    echo -e " | ${GREEN}SUCCESS${NO_COLOR}"
  fi
done