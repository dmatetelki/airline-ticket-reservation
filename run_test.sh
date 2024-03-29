#!/bin/bash

# Usage:
#   ./run_test.sh <TEST_BINARY>


function yesno()
{
  while true; do
      read -p "$* (y/n)[y]" yn
      if [ "$yn" = '' ]; then yn="y"; fi
      case "$yn" in
          [Yy]* ) return 0;;
          [Nn]* ) return 1;;
          * ) echo "Please answer y/n.";;
      esac
  done
}

pre="\E[00;33m"
fail="\E[00;31m"
post="\E[00;00m"

ulimit -c unlimited

test=$1

if [ ! -e $test ]; then
  echo -e "The parameter binary: $test does not exists"
  exit 1
fi

echo -e "${pre}Reset & remove lcov, $test.out and core files${post}"
# coverage
lcov --directory . -z
rm -f ./lcov.info
# cores
rm -f ./leak.log.core.*
# cxxtest output
rm -f $test.out


echo -e "${pre}Run tests${post}"

valgrind \
      --log-file=leak.log \
      --leak-check=full \
      --show-reachable=yes \
      --show-below-main=no \
      --track-origins=yes \
      --num-callers=30 \
      --malloc-fill=0xaa \
      --free-fill=0xdd \
      --suppressions=./qt47.supp \
      $test | tee $test.out; retval=$PIPESTATUS

# retval is 0 on success
# or the number of failed cases
# or 137 if segfault happens
if [ $retval -ne 0 ]; then
      echo -e "${fail}The executed binary: $test failed.${post}"

      if [ $retval -ne 137 ]; then
        echo -e "${pre}Failed checks:${post}"
        # TODO print the previous line too
        cat $test.out | grep "Error:" | awk -F"/test/" '{ print $2  }'
      fi

      cores=$(ls leak.log.core.* 2>/dev/null)
      if [ "$cores" != "" ]; then
        echo -e "${pre}Core file generated: ${post}"
        echo $cores

        # NOTE no need to bt full
        gdb $test $cores -ex "set width 1000" -ex "thread apply all bt" -ex q > gdb.out
        ./gdb_out_parser.pl gdb.out

        echo ""
        if yesno "run 'gdb $test $cores' ?"; then
          gdb $test $cores
        fi
      fi
      exit -1
fi


echo -e "${pre}Capture coverage info${post}"
lcov --directory . --capture -o lcov.info
# some classes are not used in the lib yet
lcov --directory . --capture -o lcov2.info
cat lcov2.info >> lcov.info

echo -e "${pre}Filtering coverage tracefile${post}"
lcov -r lcov.info "g++-v*" -o lcov.info
lcov -r lcov.info "qt4*" -o lcov.info
lcov -r lcov.info "*build*" -o lcov.info
lcov -r lcov.info "*algorithmtests.*" -o lcov.info


echo -e "${pre}Generating coverage HTML${post}"
rm -rf ./cov
mkdir cov
genhtml --frames --legend -o ./cov lcov.info

echo -e "${pre}Checking the coverage results${post}"
./cov_check.pl 90 cov/index.html

echo -e "${pre}Checking leak results${post}"
./leak_check.pl leak.log
if [ $? == 1 ]; then
  if yesno "run 'vim leak.log' ?"; then
    vim leak.log
  fi
fi
