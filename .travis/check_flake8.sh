#!/bin/sh

# Check that the source conforms to the PEP8 style

# Find flake8 executable
FLAKE8_EXE=flake8
if [ -z "$(which $FLAKE8_EXE)" ]; then
  echo "Cannot find ${FLAKE8_EXE} executable"
  exit 1
else
  ${FLAKE8_EXE} --version
fi

# Check formatting (exclude the examples directory)
DIRTY=$(find python -type f -name '*.py' | xargs ${FLAKE8_EXE})

if [ -z "${DIRTY}" ]; then
  echo "Flake8 [ OK ]"
  exit 0
else
  echo "Flake8 FAILED with the following errors:";
  echo "${DIRTY}"
  exit 1
fi
