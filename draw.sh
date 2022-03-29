#cd jun03_TT
if [ -z "$1" ]
then
    echo "No Input Argument"
else
    mkdir -p plot_TTLFV
    rm -rf plot_TTLFV/*
    cd $1
    for i in run2 16pre 16post 17 18
    #for i in run2
    do
        python ../drawhists.py -Y $i
        python ../drawhists.py -Y $i -L
    #    python ../drawhists.py -Y $i -S
    #    python ../drawhists.py -Y $i -L -S
    done
    mv plot* ../plot_TTLFV
    cd ../
fi
