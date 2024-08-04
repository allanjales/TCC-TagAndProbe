#include <TFile.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TStyle.h>

#include <ostream>
using namespace std;

#include "../src/get_efficiency_TH2D.h"
#include "../src/create_folder.h"
#include "../src/roundTH2DLabels.h"

void create_2d_efficiency(string foldername, string muonId, string sufix, bool should_save = true)
{
	TFile* file0 = TFile::Open(string("../results/efficiencies/systematic_2D/"+foldername+"/Eta_Pt_"+muonId+".root").c_str());
	TH2D* hall  = (TH2D*)file0->Get(string("histograms/all"  + sufix).c_str());
	TH2D* hpass = (TH2D*)file0->Get(string("histograms/pass" + sufix).c_str());
	TH2D* heff = get_efficiency_TH2D(hall, hpass, "Pt", "Eta", muonId);

	roundTH2DLabels(heff, 3);

	TCanvas* c1 = new TCanvas();
	c1->SetCanvasSize(1600, 900);
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
		string output_folder_name = "../results/efficiencies/systematic_2D/"+foldername+"/";
		create_folder(output_folder_name.c_str());
		c1->SaveAs(string(output_folder_name + muonId + sufix + ".png").c_str());
	}
	
	delete heff;
	delete hall;
	delete hpass;
	delete file0;
}

void create_2d_efficiency()
{
	vector<string> foldernames = {"Jpsi_Run_2011", "Jpsi_MC_2020"};
	vector<string> MuonIds = {"trackerMuon", "standaloneMuon", "globalMuon"};
	vector<string> sufixes = {"_nominal", "_systematic", "_final"};

	for (auto foldername : foldernames)
		for (auto muonId : MuonIds)
			for (auto sufix : sufixes)
				create_2d_efficiency(foldername, muonId, sufix, true);
}