#!/bin/bash

whereis -b codespell | grep "/codespell" >> /dev/null
if [ $? -ne 0 ]; then
	echo "codespell needs to be installed"
	exit 1
fi
codespell -w ../src/*
codespell -w ../include/*
codespell -w ../*.md
codespell -w *.md
exit 0
