// -----------------------------------
// Author: Allan Jales
// -----------------------------------
// Descrption: This macro will make a 2D histogram using the TH2D class from ROOT.
// Also, it is using systematic uncertainties to make the efficiency calculus.
// -----------------------------------
// Outputs:
// - a .root file containing the efficiency TH2D histogram for each systematic uncertainties and the final result;
// - a .png file for each bin fitted.
// -----------------------------------

#include <TFile.h>
#include <TH2D.h>
#include <iostream>
#include <chrono>
using namespace std;

#include "src/create_folder.h"
#include "src/create_TH2D.h"
#include "src/yields_n_errs_to_TH2Ds_bin.h"
#include "src/get_efficiency_TH2D.h"
#include "src/StopWatch.h"

// Change if you need
#include "src/dofits/DoFit_Jpsi_Run.h"
#include "src/dofits/DoFit_Jpsi_Run_2xGaus.h"
//#include "src/dofits/DoFit_Jpsi_MC.h"
//#include "src/dofits/DoFit_Jpsi_MC_2xGaus.h"

// Bins to study
string xquantity = "Pt";
double xbins[] = {0.0, 3.4, 4.0, 5.0, 6.0, 8.0, 40.};
string yquantity = "Eta";
double ybins[] = {0.0, 0.4, 0.6, 0.95, 1.2, 1.4, 2.4};
// Note: the y axis is absolute!

void plot_sys_efficiency_2d(string muonId)
{
	//Path where is going to save fit results png for every bin 
	string filename = yquantity + "_" + xquantity + "_" + muonId;
	string path_bins_fit_folder = string("results/systematic_2D/") + output_folder_name + "/bins_fit/" + filename + "/";
	create_folder(path_bins_fit_folder.c_str(), true);


	//Path where is going to save the efficiency results
	string directoryToSave = string("results/systematic_2D/") + output_folder_name + "/";
	create_folder(directoryToSave.c_str());

	//Get number of bins
	const int nbinsy = sizeof(ybins)/sizeof(*ybins) - 1;
	const int nbinsx = sizeof(xbins)/sizeof(*xbins) - 1;

	string file_path = directoryToSave + filename + ".root";
	TFile* generatedFile = new TFile(file_path.c_str(),"recreate");
	generatedFile->mkdir("histograms/");
	generatedFile->   cd("histograms/");

	TH2D *hist_all_nominal     = create_TH2D("all_nominal"   ,  "All Nominal",     xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	TH2D *hist_all_2gauss      = create_TH2D("all_2xGauss"   ,  "All 2xGauss",     xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	TH2D *hist_all_massup      = create_TH2D("all_MassUp"    ,  "All MassUp",      xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	TH2D *hist_all_massdown    = create_TH2D("all_MassDown"  ,  "All MassDown",    xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	TH2D *hist_all_binup       = create_TH2D("all_BinUp"     ,  "All BinUp",       xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	TH2D *hist_all_bindown     = create_TH2D("all_BinDown"   ,  "All BinDown",     xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	TH2D *hist_all_systematic  = create_TH2D("all_systematic",  "All Systematic",  xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	TH2D *hist_all_final       = create_TH2D("all_final"     ,  "All Final",       xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);

	TH2D *hist_pass_nominal    = create_TH2D("pass_nominal"   , "Pass Nominal",    xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	TH2D *hist_pass_2gauss     = create_TH2D("pass_2xGauss"   , "Pass 2xGauss",    xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	TH2D *hist_pass_massup     = create_TH2D("pass_MassUp"    , "Pass MassUp",     xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	TH2D *hist_pass_massdown   = create_TH2D("pass_MassDown"  , "Pass MassDown",   xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	TH2D *hist_pass_binup      = create_TH2D("pass_BinUp"     , "Pass BinUp",      xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	TH2D *hist_pass_bindown    = create_TH2D("pass_BinDown"   , "Pass BinDown",    xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	TH2D *hist_pass_systematic = create_TH2D("pass_systematic", "Pass Systematic", xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	TH2D *hist_pass_final      = create_TH2D("pass_final"     , "Pass Final",      xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);

	//Loop and fits
	StopWatch Watcher;
	const double default_min = _mmin;
	const double default_max = _mmax;
	string conditions;
	for (int i = 0; i < nbinsx; i++)
	{
		for (int j = 0; j < nbinsy; j++)
		{
			//Creates conditions
			conditions = string(    "ProbeMuon_" + xquantity + ">=" + to_string(xbins[i]  ));
			conditions +=       string(" && ProbeMuon_" + xquantity + "< " + to_string(xbins[i+1]));
			conditions +=       string(" && abs(ProbeMuon_" + yquantity + ")>=" + to_string(ybins[j]  ));
			conditions +=       string(" && abs(ProbeMuon_" + yquantity + ")< " + to_string(ybins[j+1]));

			string mmin_string;
			string mmax_string;
			double* yields_n_errs = NULL;
			double  yields_n_errs_systematic[4] = {0}; // [yield_all, yield_pass, err_all, err_pass]
			double  yields_n_errs_final[4] = {0};

			//Nominal
			cout << "Nominal calculation -----\n";
			_mmin = default_min;
			_mmax = default_max;
			_nbins = 100;
			prefix_file_name = "nominal_";
			yields_n_errs = doFit(conditions, muonId, string(path_bins_fit_folder + prefix_file_name).c_str());
			yields_n_errs_systematic[0] =  yields_n_errs[0];
			yields_n_errs_systematic[1] =  yields_n_errs[1];
			yields_n_errs_final[0] =  yields_n_errs[0];
			yields_n_errs_final[1] =  yields_n_errs[1];
			yields_n_errs_final[2] =  yields_n_errs[2];
			yields_n_errs_final[3] =  yields_n_errs[3];
			yields_n_errs_to_TH2Ds_bin(hist_all_nominal, hist_pass_nominal, i+1, j+1, yields_n_errs);
			delete[] yields_n_errs;

			//2Gauss
			cout << "2xGassians calculation -----\n";
			_mmin = default_min;
			_mmax = default_max;
			_nbins = 100;
			prefix_file_name = "2xgaus_";
			yields_n_errs = doFit2xGaus(conditions, muonId, string(path_bins_fit_folder + prefix_file_name).c_str());
			yields_n_errs_systematic[2] += pow(yields_n_errs[2], 2);
			yields_n_errs_systematic[3] += pow(yields_n_errs[3], 2);
			yields_n_errs_to_TH2Ds_bin(hist_all_2gauss, hist_pass_2gauss, i+1, j+1, yields_n_errs);
			delete[] yields_n_errs;

			//MassUp
			cout << "MassUp calculation -----\n";
			_mmin = default_min - 0.05;
			_mmax = default_max + 0.05;
			_nbins = 100;
			mmin_string = to_string(_mmin);
			mmax_string = to_string(_mmax);
			replace(mmin_string.begin(), mmin_string.end(), '.', 'p');
			replace(mmax_string.begin(), mmax_string.end(), '.', 'p');
			prefix_file_name  = string("mass_") + mmin_string.substr(0, mmin_string.length()-4) + string("_");
			prefix_file_name +=                   mmax_string.substr(0, mmax_string.length()-4) + string("_");
			yields_n_errs = doFit(conditions, muonId, string(path_bins_fit_folder + prefix_file_name).c_str());
			yields_n_errs_systematic[2] += pow(yields_n_errs[2], 2);
			yields_n_errs_systematic[3] += pow(yields_n_errs[3], 2);
			yields_n_errs_to_TH2Ds_bin(hist_all_massup, hist_pass_massup, i+1, j+1, yields_n_errs);
			delete[] yields_n_errs;

			//MassDown
			cout << "MassDown calculation -----\n";
			_mmin = default_min + 0.05;
			_mmax = default_max - 0.05;
			_nbins = 100;
			mmin_string = to_string(_mmin);
			mmax_string = to_string(_mmax);
			replace(mmin_string.begin(), mmin_string.end(), '.', 'p');
			replace(mmax_string.begin(), mmax_string.end(), '.', 'p');
			prefix_file_name  = string("mass_") + mmin_string.substr(0, mmin_string.length()-4) + string("_");
			prefix_file_name +=                   mmax_string.substr(0, mmax_string.length()-4) + string("_");
			yields_n_errs = doFit(conditions, muonId, string(path_bins_fit_folder + prefix_file_name).c_str());
			yields_n_errs_systematic[2] += pow(yields_n_errs[2], 2);
			yields_n_errs_systematic[3] += pow(yields_n_errs[3], 2);
			yields_n_errs_to_TH2Ds_bin(hist_all_massdown, hist_pass_massdown, i+1, j+1, yields_n_errs);
			delete[] yields_n_errs;

			//BinUp
			cout << "BinUp calculation -----\n";
			_mmin = default_min;
			_mmax = default_max;
			_nbins = 105;
			prefix_file_name = "binfit105_";
			yields_n_errs = doFit(conditions, muonId, string(path_bins_fit_folder + prefix_file_name).c_str());
			yields_n_errs_systematic[2] += pow(yields_n_errs[2], 2);
			yields_n_errs_systematic[3] += pow(yields_n_errs[3], 2);
			yields_n_errs_to_TH2Ds_bin(hist_all_binup, hist_pass_binup, i+1, j+1, yields_n_errs);
			delete[] yields_n_errs;

			//BinDown
			cout << "BinDown calculation -----\n";
			_mmin = default_min;
			_mmax = default_max;
			_nbins = 95;
			prefix_file_name = "binfit95_";
			yields_n_errs = doFit(conditions, muonId, string(path_bins_fit_folder + prefix_file_name).c_str());
			yields_n_errs_systematic[2] += pow(yields_n_errs[2], 2);
			yields_n_errs_systematic[3] += pow(yields_n_errs[3], 2);
			yields_n_errs_to_TH2Ds_bin(hist_all_bindown, hist_pass_bindown, i+1, j+1, yields_n_errs);
			delete[] yields_n_errs;



			// Make the systematic calculations
			yields_n_errs_systematic[2] = sqrt(yields_n_errs_systematic[2]);
			yields_n_errs_systematic[3] = sqrt(yields_n_errs_systematic[3]);
			yields_n_errs_to_TH2Ds_bin(hist_all_systematic, hist_pass_systematic, i+1, j+1, yields_n_errs_systematic);



			// Make the statistic+systematic calculations
			yields_n_errs_final[2] = sqrt(pow(yields_n_errs_final[2], 2) + pow(yields_n_errs_systematic[2], 2));
			yields_n_errs_final[3] = sqrt(pow(yields_n_errs_final[3], 2) + pow(yields_n_errs_systematic[3], 2));
			yields_n_errs_to_TH2Ds_bin(hist_all_final, hist_pass_final, i+1, j+1, yields_n_errs_final);
		}
	}
	Watcher.Stop();

	generatedFile->cd("/");
	get_efficiency_TH2D(hist_all_nominal,    hist_pass_nominal,    xquantity, yquantity, muonId, "Nominal"   );
	get_efficiency_TH2D(hist_all_2gauss,     hist_pass_2gauss,     xquantity, yquantity, muonId, "2xGauss"   );
	get_efficiency_TH2D(hist_all_massup,     hist_pass_massup,     xquantity, yquantity, muonId, "MassUp"    );
	get_efficiency_TH2D(hist_all_massdown,   hist_pass_massdown,   xquantity, yquantity, muonId, "MassDown"  );
	get_efficiency_TH2D(hist_all_binup,      hist_pass_binup,      xquantity, yquantity, muonId, "BinUp"     );
	get_efficiency_TH2D(hist_all_bindown,    hist_pass_bindown,    xquantity, yquantity, muonId, "BinDown"   );
	get_efficiency_TH2D(hist_all_systematic, hist_pass_systematic, xquantity, yquantity, muonId, "Systematic");
	get_efficiency_TH2D(hist_all_final,      hist_pass_final,      xquantity, yquantity, muonId, "Final"     );

	generatedFile->Write();
	generatedFile->Close();
		
	delete generatedFile;

	cout << "\n------------------------\n";
	cout << "Output: " << file_path;
	cout << "\nTook " << Watcher.TimeElapsedString() << ".";
	cout << "\n------------------------\n";
}

void plot_sys_efficiency_2d()
{
	StopWatch Watcher;
	plot_sys_efficiency_2d("trackerMuon");
	plot_sys_efficiency_2d("standaloneMuon");
	plot_sys_efficiency_2d("globalMuon");
	cout << "Took " << Watcher.TimeElapsedString() << " for all calculation combinations for " + string(output_folder_name) + "\n";
}