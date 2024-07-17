#!/bin/bash
LD_LIBRARY_PATH=/usr/local/lib
echo $LD_LIBRARY_PATH
export LD_LIBRARY_PATH


LIBRARY_PATH=/usr/local/lib
echo $LIBRARY_PATH
export LIBRARY_PATH

g++ -I/usr/local/include -c mainSteelMillSlab.cpp
g++ -o SteelMillSlab  -L/usr/local/lib mainSteelMillSlab.o  -lgecodedriver -lgecodesearch -lgecodeminimodel -lgecodeint -lgecodekernel  -lgecodesupport

chmod a+x SteelMillSlab
chmod a+x script1.run
chmod a+x script2.run
chmod a+x script3.run
chmod a+x script4.run
chmod a+x script5.run
chmod a+x script6.run
chmod a+x script7.run
chmod a+x script8.run
chmod a+x script9.run
chmod a+x script10.run
chmod a+x script11.run
chmod a+x script12.run
chmod a+x script13.run
chmod a+x script14.run
chmod a+x script15.run

ssh -L 40022:143.239.81.140:22 fdesouza@143.239.81.1 -p 40022

scp -P 40022 -r SteelMillSlab/ fdesouza@localhost:/home/fdesouza/steelMillSlab/

scp -P 40022 -r  fdesouza@localhost:/home/fdesouza/steelMillSlab/output/ .

nohup ./script1.run > ../log/out240506-S1.log &
nohup ./script2.run > ../log/out240506-S2.log &
nohup ./script3.run > ../log/out240506-S3.log &
nohup ./script4.run > ../log/out240506-S4.log &
nohup ./script5.run > ../log/out240506-S5.log &
nohup ./script6.run > ../log/out240104-S6.log &
nohup ./script7.run > ../log/out240104-S7.log &
nohup ./script8.run > ../log/out240104-S8.log &
nohup ./script9.run > ../log/out240104-S9.log &
nohup ./script10.run > ../log/out240104-S10.log &
nohup ./script11.run > ../log/out240104-S11.log &
nohup ./script12.run > ../log/out240104-S12.log &
nohup ./script13.run > ../log/out240104-S13.log &
nohup ./script14.run > ../log/out240104-S14.log &
nohup ./script15.run > ../log/out240104-S15.log &




[1] 1804120
[2] 1804121
[3] 1804122
[4] 1804123
[5] 1804124




