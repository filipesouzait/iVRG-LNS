#!/bin/bash
LD_LIBRARY_PATH=/usr/local/lib
echo $LD_LIBRARY_PATH
export LD_LIBRARY_PATH
g++ -I/usr/local/include -c mainSteelMillSlab.cpp
g++ -o SteelMillSlab  -L/usr/local/lib mainSteelMillSlab.o  -lgecodedriver -lgecodesearch -lgecodeminimodel -lgecodeint -lgecodekernel  -lgecodesupport
./SteelMillSlab instance=2_0 selection=0 runTime=2
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