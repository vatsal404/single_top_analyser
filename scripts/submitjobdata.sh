version="skim_LFV_4_1_0"
skimmed_data="/data1/common/skimmed_NanoAOD/"${version}"/data"

./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_RunA_V19 --json=data/Cert_314472-325175_13TeV_17SeptEarlyReReco2018ABC_PromptEraD_Collisions18_JSON.txt ${skimmed_data}/SingleMuon2018A/000 Run2018A1.root &> Run2018A1.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_RunA_V19 --json=data/Cert_314472-325175_13TeV_17SeptEarlyReReco2018ABC_PromptEraD_Collisions18_JSON.txt ${skimmed_data}/SingleMuon2018A/050 Run2018A2.root &> Run2018A2.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_RunA_V19 --json=data/Cert_314472-325175_13TeV_17SeptEarlyReReco2018ABC_PromptEraD_Collisions18_JSON.txt ${skimmed_data}/SingleMuon2018A/100 Run2018A3.root &> Run2018A3.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_RunA_V19 --json=data/Cert_314472-325175_13TeV_17SeptEarlyReReco2018ABC_PromptEraD_Collisions18_JSON.txt ${skimmed_data}/SingleMuon2018A/150 Run2018A4.root &> Run2018A4.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_RunA_V19 --json=data/Cert_314472-325175_13TeV_17SeptEarlyReReco2018ABC_PromptEraD_Collisions18_JSON.txt ${skimmed_data}/SingleMuon2018A/200 Run2018A5.root &> Run2018A5.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_RunB_V19 --json=data/Cert_314472-325175_13TeV_17SeptEarlyReReco2018ABC_PromptEraD_Collisions18_JSON.txt ${skimmed_data}/SingleMuon2018B Run2018B.root &> Run2018B.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_RunC_V19 --json=data/Cert_314472-325175_13TeV_17SeptEarlyReReco2018ABC_PromptEraD_Collisions18_JSON.txt ${skimmed_data}/SingleMuon2018C Run2018C.root &> Run2018C.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_RunD_V19 --json=data/Cert_314472-325175_13TeV_17SeptEarlyReReco2018ABC_PromptEraD_Collisions18_JSON.txt ${skimmed_data}/SingleMuon2018D/000 Run2018D1.root &> Run2018D1.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_RunD_V19 --json=data/Cert_314472-325175_13TeV_17SeptEarlyReReco2018ABC_PromptEraD_Collisions18_JSON.txt ${skimmed_data}/SingleMuon2018D/050 Run2018D2.root &> Run2018D2.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_RunD_V19 --json=data/Cert_314472-325175_13TeV_17SeptEarlyReReco2018ABC_PromptEraD_Collisions18_JSON.txt ${skimmed_data}/SingleMuon2018D/100 Run2018D3.root &> Run2018D3.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_RunD_V19 --json=data/Cert_314472-325175_13TeV_17SeptEarlyReReco2018ABC_PromptEraD_Collisions18_JSON.txt ${skimmed_data}/SingleMuon2018D/150 Run2018D4.root &> Run2018D4.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_RunD_V19 --json=data/Cert_314472-325175_13TeV_17SeptEarlyReReco2018ABC_PromptEraD_Collisions18_JSON.txt ${skimmed_data}/SingleMuon2018D/200 Run2018D5.root &> Run2018D5.out &
./processnanoaod.py --allinone -Y 2018 --globaltag Autumn18_RunD_V19 --json=data/Cert_314472-325175_13TeV_17SeptEarlyReReco2018ABC_PromptEraD_Collisions18_JSON.txt ${skimmed_data}/SingleMuon2018D/250 Run2018D6.root &> Run2018D6.out &
sleep 1h
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017BCD_V11 --json=data/Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016B_ver2 Run2016B_ver2.root &> Run2016B_ver2.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017BCD_V11 --json=data/Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016C Run2016C.root &> Run2016C.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017BCD_V11 --json=data/Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016D Run2016D.root &> Run2016D.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017EF_V11 --json=data/Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016E Run2016E.root &> Run2016E.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017EF_V11 --json=data/Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016F Run2016F.root &> Run2016F.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017GH_V11 --json=data/Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016G Run2016G.root &> Run2016G.out &
./processnanoaod.py --allinone -Y 2016 --globaltag Summer16_07Aug2017GH_V11 --json=data/Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt ${skimmed_data}/SingleMuon2016H Run2016H.root &> Run2016H.out &

./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017B_V32 --json=data/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt ${skimmed_data}/SingleMuon2017B Run2017B.root &> Run2017B.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017C_V32 --json=data/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt ${skimmed_data}/SingleMuon2017C Run2017C.root &> Run2017C.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017DE_V32 --json=data/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt ${skimmed_data}/SingleMuon2017D Run2017D.root &> Run2017D.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017DE_V32 --json=data/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt ${skimmed_data}/SingleMuon2017E Run2017E.root &> Run2017E.out &
./processnanoaod.py --allinone -Y 2017 --globaltag Fall17_17Nov2017F_V32 --json=data/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt ${skimmed_data}/SingleMuon2017F Run2017F.root &> Run2017F.out &
