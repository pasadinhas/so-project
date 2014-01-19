#!/bin/bash

TESTS="test_kos_multi_threaded_all_getAll_fromFile test_kos_single_threaded_put_get test_kos_single_threaded_put_remove_get test_kos_single_threaded_put_get_put_get test_kos_single_threaded_put_get_remove_put_get test_kos_single_threaded_put_dump test_kos_single_threaded_put_get_remove_put_dump  test_kos_multi_threaded_put_get test_kos_multi_threaded_put_get_remove_get   test_kos_multi_threaded_all test_kos_multi_threaded_all_getAll test_kos_multi_threaded_all_shared"

cd ..
make clean >/dev/null
make >/dev/null
cd tests

c=0
for i in $TESTS
do
	(( c++ ))
	echo #########################################################
	echo Executing test $c: $i
	echo #########################################################
	rm f*
	time ./${i}
	echo ---------------------------------------------------------
	echo press a key to continue
	read a
done
