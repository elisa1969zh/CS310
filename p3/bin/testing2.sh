#!/bin/bash 
cd ..
cd serverlogs/
rm server.output server1.output server2.output server3.output 1.config 2.config 3.config 1.log 2.log 3.log
cp 1_INIT.config 1.config
cp 2_INIT.config 2.config
cp 3_INIT.config 3.config
cp 1_INIT2.log 1.log
cp 2_INIT2.log 2.log
cp 3_INIT2.log 3.log
cd ..
make
make test1
make diff1_2
make diff2_3
make diff3_1
make cat1
make cat2
make cat3
cd serverlogs/
cp server.output server1.output
cd ..
# 
make test2
make diff1_2
make diff2_3
make diff3_1
make cat1
make cat2
make cat3
cd serverlogs/
cp server.output server2.output
cd ..
# 
make test3
make diff1_2
make diff2_3
make diff3_1
make cat1
make cat2
make cat3
cd serverlogs/
cp server.output server3.output
cd ..