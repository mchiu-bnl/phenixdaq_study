//
TGraph *g_haggerty = 0;		// measured BBC accepted rate curve (from noise)

// Get John Haggerty's max curve
void gethaggerty(const char *fname = "livetime_measurements_acceptrate_20100129.txt")
{
  g_haggerty = new TGraph(fname);
  g_haggerty->SetName("g_haggerty");
  g_haggerty->GetHistogram()->SetXTitle("accepted rate (Hz)");
  g_haggerty->GetHistogram()->SetYTitle("livetime");
}

void plot_sphenixdaq()
{
  const char *fname_model = "sphenixdaq_376endat_0conv_count4.root";  // sPHENIX DAQ model
  const char *fname_model14 = "phenixdaq_860endat_400conv_count4.root";  // PHENIX Run14 DAQ model
  const char *fname_data14 = "run14auau200_rate.root";                  // Real Data
  const char *fname_data13 = "run13pp510v43_rate.root";                  // Real Data

  TCanvas *c_sphenix_daqrate = new TCanvas("sphenix_daqrate","sPHENIX DAQ Rate",1100,850);

  // Draw the sPHENIX model 
  TFile *tfile_model = new TFile(fname_model,"READ");
  TGraph *modelreal = (TGraph*)tfile_model->Get("modelreal");

  modelreal->SetLineColor(1);
  modelreal->SetLineWidth(3);
  modelreal->SetMaximum(1.02);
  modelreal->SetMinimum(0.5);
  modelreal->Draw("acp");
  modelreal->GetHistogram()->SetTitle("sPHENIX DAQ");
  modelreal->GetHistogram()->SetYTitle("Livetime");
  modelreal->GetHistogram()->SetXTitle("Rate (Hz)");

  // Draw the PHENIX Run14 model
  TFile *tfile_model14 = new TFile(fname_model14,"READ");
  TGraph *modelreal14 = (TGraph*)tfile_model14->Get("modelreal");

  // Reduce livetime curve by 1% to account for dead crossing
  Int_t n = modelreal14->GetN();
  Double_t *y = modelreal14->GetY();
  for (int i=0; i<n; i++)
  {
    y[i] -= 0.007;
  }

  modelreal14->SetLineColor(1);
  modelreal14->SetMarkerColor(1);
  modelreal14->SetLineWidth(3);
  modelreal14->SetLineStyle(9);
  modelreal14->Draw("cp");

  // BBC measured rate
  /*
  gethaggerty();
  g_haggerty->SetLineColor(1);
  g_haggerty->Draw("same");
  */

  // Draw the livetimes from run14auau200
  TFile *tfile_data14 = new TFile(fname_data14,"READ");
  TTree *t14 = (TTree*)tfile_data14->Get("t");
  t14->SetMarkerColor(4);
  t14->SetMarkerStyle(20);
  t14->SetMarkerSize(0.5);
  t14->Draw("livetime:eventrate","","same");

  // Draw the livetimes from run13pp510 
  TFile *tfile_data13 = new TFile(fname_data13,"READ");
  TTree *t13 = (TTree*)tfile_data13->Get("t");
  t13->SetMarkerColor(2);
  t13->SetMarkerStyle(20);
  t13->SetMarkerSize(0.5);
  t13->Draw("livetime:eventrate","","same");

  TLegend *legend = new TLegend(0.7,0.7,0.9,0.89);
  legend->SetBorderSize(0);
  legend->AddEntry(modelreal,"sPHENIX DAQ MC","l");
  legend->AddEntry(modelreal14,"PHENIX DAQ MC","l");
  legend->AddEntry(t14,"PHENIX Run14 AuAu","p");
  legend->AddEntry(t13,"PHENIX Run13 pp","p");
  legend->Draw();
}

