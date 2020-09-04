#!/bin/sh
# Build script for compatibility with legacy instructions
# Yes, a shell script even though it has .py extension. The build system
# used to be Python based but we don't want to add a Python dependency
# just for this legacy compatibility script.
make all
