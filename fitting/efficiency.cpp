// -----------------------------------
// Author: Allan Jales
// -----------------------------------
// Descrption: This macro will make a 1D efficiency histogram for the dataset
// -----------------------------------
// Outputs:
// - a .root file containing the efficiency histogram;
// - a .png file for each bin fitted.
// -----------------------------------

// Change if you need
// #include "src/dofits/DoFit_Jpsi_Run.h"
#include "src/dofits/DoFit_Jpsi_MC.h"

#include "src/create_folder.h"
#include "src/get_efficiency.h"
#include "src/make_TH1D.h"
#include "src/StopWatch.h"

#include <TFile.h>

vector<double> GetBinsListOf(string quantity)
{
	if (quantity == "Pt")
		return {0., 2.0, 3.4, 4.0, 4.4, 4.7, 5.0, 5.6, 5.8, 6.0, 6.2, 6.4, 6.6, 6.8, 7.3, 9.5, 13.0, 17.0, 40.};
	if (quantity == "Eta")
		return {-2.4, -1.8, -1.4, -1.2, -1.0, -0.8, -0.5, -0.2, 0, 0.2, 0.5, 0.8, 1.0, 1.2, 1.4, 1.8, 2.4};
	if (quantity == "Phi")
		return {-3.0, -1.8, -1.6, -1.2, -1.0, -0.7, -0.4, -0.2, 0, 0.2, 0.4, 0.7, 1.0, 1.2, 1.6, 1.8, 3.0};
	return {};
}

void efficiency(string muonId, string quantity)
{
	vector<double> bins = GetBinsListOf(quantity);

	//Path where is going to save results png for every bin 
	string path_bins_fit_folder = string("results/efficiency/") + output_folder_name + "/bins_fit/" + quantity + "_" + muonId + "/";
	create_folder(path_bins_fit_folder.c_str(), true);

	// Loop for every bin and fit it
	int nbins = bins.size()-1;
	double** yields_n_errs = new double*[nbins];
	for (int i = 0; i < nbins; i++)
	{
		//Creates conditions
		string conditions = string(    "ProbeMuon_" + quantity + ">=" + to_string(bins[i]  ));
		conditions +=       string(" && ProbeMuon_" + quantity + "< " + to_string(bins[i+1]));

		//Stores [yield_all, yield_pass, err_all, err_pass]
		yields_n_errs[i] = doFit(conditions, muonId, path_bins_fit_folder.c_str());
	}

	//Path where is going to save efficiency 
	string directoryToSave = string("results/efficiency/") + output_folder_name + string("/");
	create_folder(directoryToSave.c_str());

	//Create file
	string file_path = directoryToSave + prefix_file_name + quantity + "_" + muonId + ".root";
	TFile* generatedFile = new TFile(file_path.c_str(),"recreate");
	
	//Create histograms
	generatedFile->mkdir("histograms/");
	generatedFile->   cd("histograms/");
	TH1D *yield_all  = make_TH1D("ALL" , yields_n_errs, 0, &bins[0], nbins, quantity);
	TH1D *yield_pass = make_TH1D("PASS", yields_n_errs, 1, &bins[0], nbins, quantity);
	
	//Create efficiencies
	generatedFile->   cd("/");
	get_efficiency(yield_all, yield_pass, quantity, muonId, "", true);

	//Write file
	generatedFile->Write();

	cout << "\n[Settings]\n";
	cout << output_folder_name << "\n"<< quantity << " " << muonId << "\n";
	cout << "Fitting:     " << fit_functions << "\n";
	cout << "Fit between: " << _mmin << " and " << _mmax << " GeV\n";
	cout << "Bins:        " << _nbins << "\n";

	cout << "\n------------------------\n";
	cout << "Output: " << file_path;
	cout << "\n------------------------\n";
}

void efficiency()
{
	vector<string> muonIds = {"trackerMuon", "standaloneMuon", "globalMuon"};
	vector<string> quantities = {"Pt", "Eta", "Phi"};

	StopWatch Watcher;
	for (auto muonId : muonIds)
		for (auto quantity : quantities)
			efficiency(muonId, quantity);
	cout << "Took " << Watcher.TimeElapsedString() << " for all calculation combinations for " + string(output_folder_name) + "\n";
}