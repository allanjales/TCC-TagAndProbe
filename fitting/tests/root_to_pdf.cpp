#include <TFile.h>
#include <TCanvas.h>
#include <TH2D.h>
#include <TStyle.h>

#include <ostream>
using namespace std;

#include "../src/get_efficiency_TH2D.h"
#include "../src/create_folder.h"
#include "../src/roundTH2DLabels.h"

void data_mc_ratio(string location, string muonId, string type, bool should_save = true)
{
	TFile* file0 = TFile::Open(string(location + "Eta_Pt_" + muonId + ".root").c_str());
	TH2D* heff = (TH2D*)file0->Get(string(muonId + "_Eta_Pt_" + type + "_Efficiency").c_str());
	//heff->SetTitle(string("Data/MC for " + muonId + " (" + type + ")").c_str());
	roundTH2DLabels(heff, 3);

	TCanvas* c1 = new TCanvas("c1", "c1", 1280, 900);
	c1->cd();
	heff->Draw("colztexte");

	//heff->SetMaximum(1.);
	//heff->SetMinimum(0.88);
	heff->SetMaximum(1.06);
	heff->SetMinimum(0.4);
	gStyle->SetOptStat(0);
	c1->SetLogx();

	if (should_save)
	{
		string output_folder_name = location;
		create_folder(output_folder_name.c_str());
		c1->SaveAs(string(output_folder_name + muonId + "_" + type + ".png").c_str());
	}

	delete heff;
	delete file0;
	delete c1;
}

void root_to_pdf()
{
	vector<string> locations = {"Jpsi_Run_2011/", "Jpsi_MC_2020/"};
	vector<string> muonIds = {"trackerMuon", "standaloneMuon", "globalMuon"};
	vector<string> types = {"Final", "Nominal", "Systematic", "2xGauss", "MassUp", "MassDown", "BinUp", "BinDown"};

	for (auto location : locations)
		for (auto muonId : muonIds)
			for (auto type : types)
				data_mc_ratio(location, muonId, type, true);
}