#ifndef DOFIT_HEADER
#define DOFIT_HEADER
//We start by declaring the nature of our dataset. (Is the data real or simulated?)
const char* output_folder_name = "Jpsi_MC_2020";

//Header of this function
double _mmin = 2.8;
double _mmax = 3.3;
double _nbins = 0; //Let it 0 if dont want to change

//Information for output at the end of run
const char* fit_functions = "2xGaussians + Exponential";
string prefix_file_name = "";
#endif

#include <TMath.h>
#include <RooRealVar.h>
#include <RooDataSet.h>
#include <RooCategory.h>
#include <RooFormulaVar.h>
#include <RooDataHist.h>
#include <RooPlot.h>
#include <RooGaussian.h>
#include <RooCBShape.h>
#include <RooExponential.h>
#include <RooAddPdf.h>
#include <RooFitResult.h>
#include <RooSimultaneous.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <RooChi2Var.h>
#include <TLatex.h>

#include <iostream>
using namespace std;
using namespace RooFit;

//Returns array with [yield_all, yield_pass, err_all, err_pass]
#ifdef DEFAULT_FUCTION_NAME_USED
	double* doFit2xGaus
#else
	#define DEFAULT_FUCTION_NAME_USED
	double* doFit
#endif
(string condition, string muonId, const char* savePath = NULL)
{
	cout << "----- Fitting data on bin -----\n";
	cout << "Conditions: " << condition << "\n";
	cout << "-------------------------------\n";

	string MuonId_str = "";
	if      (muonId == "trackerMuon")    MuonId_str = "PassingProbeTrackingMuon";
	else if (muonId == "standaloneMuon") MuonId_str = "PassingProbeStandAloneMuon";
	else if (muonId == "globalMuon")     MuonId_str = "PassingProbeGlobalMuon";
	
	const char *filename = "DATA/TagAndProbe_Jpsi_MC.root";
	const char *treename = "tagandprobe";
	string cutVarString = "TagMuon_Pt >= 7.0 && fabs(TagMuon_Eta) <= 2.4";
	if (!condition.empty())
		cutVarString += " && " + condition;

	RooCategory MuonId_var(MuonId_str.c_str(), MuonId_str.c_str());
	MuonId_var.defineType("Passing", 1);
	MuonId_var.defineType("Failing", 0);
	RooRealVar InvariantMass("InvariantMass", "InvariantMass", _mmin, _mmax);
	RooRealVar ProbeMuon_Pt ("ProbeMuon_Pt",  "ProbeMuon_Pt",   0., 40.);
	RooRealVar ProbeMuon_Eta("ProbeMuon_Eta", "ProbeMuon_Eta", -2.4, 2.4);
	RooRealVar ProbeMuon_Phi("ProbeMuon_Phi", "ProbeMuon_Phi", -TMath::Pi(), TMath::Pi());
	RooRealVar TagMuon_Pt   ("TagMuon_Pt",    "TagMuon_Pt",     0., 40.);
	RooRealVar TagMuon_Eta  ("TagMuon_Eta",   "TagMuon_Eta",   -2.4, 2.4);
	RooRealVar TagMuon_Phi  ("TagMuon_Phi",   "TagMuon_Phi",   -TMath::Pi(), TMath::Pi());
	RooArgSet vars(MuonId_var, InvariantMass, ProbeMuon_Pt, ProbeMuon_Eta, ProbeMuon_Phi,
		TagMuon_Pt, TagMuon_Eta, TagMuon_Phi);

	if (_nbins > 0) InvariantMass.setBins(_nbins);
		_nbins = InvariantMass.getBinning().numBins();

	cout << "Preparing RooDataSet for 'ALL'...\n";
	RooDataSet* Data_ALL = new RooDataSet("Data_All", "Data_All", vars, ImportFromFile(filename, treename), Cut(cutVarString.c_str())); 

	// Create a subset of dataset for events that pass the MuonId condition
	cout << "Preparing RooDataSet for 'PASSING'...\n";
	RooDataSet* Data_PASS = new RooDataSet("Data_Pass", "Data_Pass", Data_ALL, *Data_ALL->get(), (MuonId_str + "==1").c_str());
	
	RooDataHist dh_ALL (Data_ALL->GetName(),  Data_ALL->GetTitle(),  RooArgSet(InvariantMass), *Data_ALL);
	RooDataHist dh_PASS(Data_PASS->GetName(), Data_PASS->GetTitle(), RooArgSet(InvariantMass), *Data_PASS);

	//SIGNAL VARIABLES
	RooRealVar mean  ("mean", "mean", 3.094, 3.07, 3.2);
	RooRealVar sigma1("sigma1", "sigma1", 0.05*(_mmax-_mmin), 0., 0.5*(_mmax-_mmin));
	RooRealVar sigma2("sigma2", "sigma2", 0.038);
	   
	//FIT FUNCTIONS
	RooGaussian gaussian1("GS1", "GS1", InvariantMass, mean, sigma1);
	RooGaussian gaussian2("GS2", "GS2", InvariantMass, mean, sigma2);
	RooRealVar frac1     ("frac1","frac1", 0.5);
	RooAddPdf  signal    ("signal", "signal", RooArgList(gaussian1, gaussian2), RooArgList(frac1));

	//BACKGROUND VARIABLES
	RooRealVar a0("a0", "a0", 0, -10, 0, "");

	//BACKGROUND FUNCTION
	RooExponential background("background", "background", InvariantMass, a0);

	RooRealVar n_signal_all ("n_signal_all", "n_signal_all", Data_ALL->sumEntries()/2, 0., Data_ALL->sumEntries());
	RooRealVar n_signal_pass("n_signal_pass", "n_signal_pass", Data_PASS->sumEntries()/2, 0., Data_PASS->sumEntries());

	RooRealVar n_back_all ("n_back_all", "n_back_all", Data_ALL->sumEntries()/2, 0., Data_ALL->sumEntries());
	RooRealVar n_back_pass("n_back_pass", "n_back_pass", Data_PASS->sumEntries()/2, 0., Data_PASS->sumEntries());

	RooAddPdf model_all ("model_all", "model_all", RooArgList(signal, background), RooArgList(n_signal_all, n_back_all));
	RooAddPdf model_pass("model_pass", "model_pass", RooArgList(signal, background), RooArgList(n_signal_pass, n_back_pass));
	
	// SIMULTANEOUS FIT
	RooCategory sample("sample","sample") ;
	sample.defineType("All") ;
	sample.defineType("Passing") ;
	RooDataHist combData("combData", "combined data", InvariantMass,
		Index(sample), Import("ALL", dh_ALL), Import("PASSING", dh_PASS));
	
	RooSimultaneous simPdf("simPdf","simultaneous pdf", sample);
	simPdf.addPdf(model_all,"ALL");
	simPdf.addPdf(model_pass,"PASSING");
	
	RooFitResult* fitres = new RooFitResult;
	fitres = simPdf.fitTo(combData, RooFit::Save());

	RooRealVar* yield_all  = (RooRealVar*)fitres->floatParsFinal().find("n_signal_all");
	RooRealVar* yield_pass = (RooRealVar*)fitres->floatParsFinal().find("n_signal_pass");
	delete fitres;

	// OUTPUT ARRAY
	double* output = new double[4];
	
	output[0] = yield_all->getVal();
	output[1] = yield_pass->getVal();
	
	output[2] = yield_all->getError();
	output[3] = yield_pass->getError();

	// DRAWING
	RooPlot* frame_all = InvariantMass.frame(RooFit::Title("Invariant Mass"));
	frame_all->SetTitle("ALL");
	frame_all->SetXTitle("#mu^{+}#mu^{-} invariant mass [GeV/c^{2}]");
	
	Data_ALL->plotOn(frame_all, Name("data"));
	model_all.plotOn(frame_all, Name("total"));
	model_all.plotOn(frame_all, RooFit::Components("GS1"), Name("GS1"), RooFit::LineStyle(kDashed), RooFit::LineColor(kGreen));
	model_all.plotOn(frame_all, RooFit::Components("GS2"), Name("GS2"), RooFit::LineStyle(kDashed), RooFit::LineColor(kMagenta - 5));
	model_all.plotOn(frame_all, RooFit::Components("background"), Name("background"), RooFit::LineStyle(kDashed), RooFit::LineColor(kRed));
	
	TCanvas* c_all = new TCanvas();
	c_all->SetLeftMargin(0.11);
	c_all->SetRightMargin(0.09);
	c_all->cd();
	frame_all->Draw();

	TLegend legendAll(0.70, 0.96, 0.96, 0.92);
	legendAll.AddEntry(frame_all->findObject("data"), "Data");
	legendAll.AddEntry(frame_all->findObject("total"), "Total");
	legendAll.AddEntry(frame_all->findObject("GS1"), "Gaussian 1");
	legendAll.AddEntry(frame_all->findObject("GS2"), "Gaussian 2");
	legendAll.AddEntry(frame_all->findObject("background"), "Background");
	legendAll.SetTextSize(0.04);
	legendAll.SetY1(0.96 - (legendAll.GetTextSize()+0.01)*legendAll.GetNRows());
	legendAll.Draw();

	RooPlot* frame_pass = InvariantMass.frame(RooFit::Title("Invariant Mass"));
	frame_pass->SetTitle("PASSING");
	frame_pass->SetXTitle("#mu^{+}#mu^{-} invariant mass [GeV/c^{2}]");

	Data_PASS->plotOn(frame_pass, Name("data"));
	model_pass.plotOn(frame_pass, Name("total"));
	model_pass.plotOn(frame_pass, RooFit::Components("GS1"), Name("GS1"), RooFit::LineStyle(kDashed), RooFit::LineColor(kGreen));
	model_pass.plotOn(frame_pass, RooFit::Components("GS2"), Name("GS2"), RooFit::LineStyle(kDashed), RooFit::LineColor(kMagenta - 5));
	model_pass.plotOn(frame_pass, RooFit::Components("background"), Name("background"), RooFit::LineStyle(kDashed), RooFit::LineColor(kRed));
	
	TCanvas* c_pass = new TCanvas();
	c_pass->SetLeftMargin(0.11);
	c_pass->SetRightMargin(0.09);
	c_pass->cd();
	frame_pass->Draw();

	TLegend legendPass(0.70, 0.96, 0.96, 0.92);
	legendPass.AddEntry(frame_pass->findObject("data"), "Data");
	legendPass.AddEntry(frame_pass->findObject("total"), "Total");
	legendPass.AddEntry(frame_pass->findObject("GS1"), "Gaussian 1");
	legendPass.AddEntry(frame_pass->findObject("GS2"), "Gaussian 2");
	legendPass.AddEntry(frame_pass->findObject("background"), "Background");
	legendPass.SetTextSize(0.04);
	legendPass.SetY1(0.96 - (legendPass.GetTextSize()+0.01)*legendPass.GetNRows());
	legendPass.Draw();

	if (savePath != NULL)
	{
		c_pass->SaveAs((string(savePath) + condition + "_PASS.png").c_str());
		c_all->SaveAs ((string(savePath) + condition + "_ALL.png").c_str());
	}

	delete Data_ALL;
	delete Data_PASS;

	delete c_all;
	delete c_pass;

	delete frame_all;
	delete frame_pass;

	cout << "---------- Fit ended ----------\n";
	// [yield_all, yield_pass, err_all, err_pass]
	return output;
}