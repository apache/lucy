#!/bin/sh

set -ex

lcov --zerocounters --directory .. --base-directory .
t/test_lucy
lcov --capture --directory .. --base-directory . --rc lcov_branch_coverage=1 --output-file lucy.info
genhtml --branch-coverage --output-directory coverage lucy.info

