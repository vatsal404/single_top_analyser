pname=$1
if [ -z "$1" ]
then
    echo "No Input Argument"
else
    syst=norm
    cd ${pname}
    mkdir -p ${syst}
    cd ${syst}
    for i in 18 16post 16pre 17
    do
        echo Setup ${i} samples
        mkdir -p ${i}/tmp
        mv *${i}*${syst}*.root ${i}
        cd ${i}
        mv Run* tmp
        rm -rf tmp/Run${i}_${syst}.root
        hadd Run${i}_${syst}.root tmp/Run${i}*_${syst}.root
        #hadd TTToSemiLeptonic_${i}_${syst}.root TTToSemi*.root
        #mv tmp/Run20${i}_${syst}.root tmp/TTToSemiLeptonic_${i}_${syst}.root ./
        cd ../
    done

    #cd ${pname}
    #for i in 16pre 16post 17
    #do
    #    cp 18/LFV* ${i}/
    #    cd ${i}
    #    rename _18_ _${i}_ *_18_*
    #    cd ../
    #done
    rm -rf Run2_${syst}.root
    hadd Run2_${syst}.root 16pre/Run16pre_${syst}.root 16post/Run16post_${syst}.root 17/Run17_${syst}.root 18/Run18_${syst}.root
    rm -rf *.out
    cd ../../
fi
