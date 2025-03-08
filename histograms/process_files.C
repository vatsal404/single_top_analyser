#include <iostream>
#include <vector>
#include <string>

void process_files() {
    std::string directory = "/eos/uscms/store/user/vsinha/bg_estimation/";

      std::map<std::string, std::tuple<double, double>> file_info = {
        {directory + "PROC_DYJetsToLL_M-10to50_UL17.root", std::make_tuple(18610.0, 68480179.0)},
        {directory + "PROC_DYJetsToLL_M-50_UL17.root", std::make_tuple(4895.0, 102863931.0)},
        {directory + "PROC_QCD_Pt-1000_MuEnrichedPt5.root", std::make_tuple(1.6212, 27478273.0)},
        {directory + "PROC_QCD_Pt-120To170_MuEnrichedPt5.root", std::make_tuple(25190.0, 39395152.0772)},
        {directory + "PROC_QCD_Pt-120to170_EMEnriched.root", std::make_tuple(62964.0, 9904361.6385)},
        {directory + "PROC_QCD_Pt-15To20_MuEnrichedPt5.root", std::make_tuple(3819570, 9021334.990524)},
 // {directory + "PROC_QCD_Pt-15to20_EMEnriched.root", std::make_tuple(, 7967858.631735)},
        {directory + "PROC_QCD_Pt-170To300_MuEnrichedPt5.root", std::make_tuple(8654.0, 73071987.0)},
        {directory + "PROC_QCD_Pt-20To30_MuEnrichedPt5.root", std::make_tuple(2960198.0, 64624161.353041)},
        {directory + "PROC_QCD_Pt-20to30_EMEnriched.root", std::make_tuple(5352960.0, 14166154.32915)},
        {directory + "PROC_QCD_Pt-300To470_MuEnrichedPt5.root", std::make_tuple(797.35, 58692920.280472)},
       {directory + "PROC_QCD_Pt-30To50_MuEnrichedPt5.root", std::make_tuple(1652471.0, 55644000.0)},
        {directory + "PROC_QCD_Pt-30to50_EMEnriched.root", std::make_tuple(9928000.0, 8784542.0)},
        {directory + "PROC_QCD_Pt-470To600_MuEnrichedPt5.root", std::make_tuple(79.022, 39491905.054731)},
        {directory + "PROC_QCD_Pt-50To80_MuEnrichedPt5.root", std::make_tuple(437504.0, 40322726.0)},
        {directory + "PROC_QCD_Pt-50to80_EMEnriched.root", std::make_tuple(2890800.0, 10210400.0)},
        {directory + "PROC_QCD_Pt-600To800_MuEnrichedPt5.root", std::make_tuple(25.095, 39321941.142106)},
        {directory + "PROC_QCD_Pt-70to300_EMEnriched.root", std::make_tuple(18810.0, 3678200.0)},
        {directory + "PROC_QCD_Pt-800To1000_MuEnrichedPt5.root", std::make_tuple(4.7075, 77715378.0)},
        {directory + "PROC_QCD_Pt-80To120_MuEnrichedPt5.root", std::make_tuple(106033.0, 45986550.685169)},
        {directory + "PROC_QCD_Pt-80to120_EMEnriched.root", std::make_tuple(350000, 9617412.941175)},
        {directory + "PROC_ST_tW_antitop_5f.root", std::make_tuple(35.85, 183284892.384979)},
        {directory + "PROC_ST_tW_top_5f.root", std::make_tuple(35.85, 184446306.893539)},
        {directory + "TTBar.root", std::make_tuple(831.76, 65117896.0)},
// {directory + "PROC_WJetsToLNu_UL17.root", std::make_tuple(0.0, 1030816976.467947)},
        {directory + "PROC_WJetsToLNu_0J_UL17.root", std::make_tuple(49670, 9552834599026.218750)},
        {directory + "PROC_WJetsToLNu_1J_UL17.root", std::make_tuple(8264, 4883768082289.562500)},
        {directory + "PROC_WJetsToLNu_2J_UL17.root", std::make_tuple(3226, 1030816976.467947)},

	{directory + "process_ST_t-channel_antitop_4f.root", std::make_tuple(80.95, 4462868882.059891)},
        {directory + "process_ST_t-channel_top_4f.root", std::make_tuple(136.02, 9090911402.50351)}
    };
    
    for (const auto& [filename, params] : file_info) {
        double cross_section = std::get<0>(params);
        double sum_gen_weight = std::get<1>(params);
        
        std::cout << "Processing file: " << filename << " with cross_section: " 
                  << cross_section << " and sum_gen_weight: " << sum_gen_weight << std::endl;
        
        // Pass the filename, cross_section, and sum_gen_weight to create_normalized_histogram
        create_normalized_histogram(filename, cross_section, sum_gen_weight);
    }
}




