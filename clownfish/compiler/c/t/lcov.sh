#!/bin/sh

set -ex

lcov --zerocounters --directory ..
t/test_cfc
lcov --capture --directory .. --rc lcov_branch_coverage=1 --output-file cfc.info
genhtml --branch-coverage --output-directory coverage cfc.info

