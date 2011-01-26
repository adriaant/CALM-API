#! /bin/sh/

# invoke with sh run.sh
k=14
while [ $k -ne 101 ]
do
    echo $k

    calm -r 1 -e 10 -i 100 -d simulations/multi -b king -v 0 -p 1 >& simulations/multi/logs/growing-$k.txt
    k=`expr $k + 1`
    
done

