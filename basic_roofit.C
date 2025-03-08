#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooDataSet.h"
#include "RooPlot.h"
#include "TCanvas.h"

void basic_roofit() {

	RooRealVar x("x","observable x",0,100);
	RooRealVar mean("mean","Mean",50,0,100);
	RooRealVar sigma("sigma","Sigma",10,0.1,30);

	RooGaussian gauss("gauss","Gaussian PDF", x , mean ,sigma);

	RooDataSet* data=gauss.generate(x,1000);

	gauss.fitTo(*data);

	RooPlot* frame=x.frame();
	data->plotOn(frame);
	gauss.plotOn(frame);

	TCanvas* canvas=new TCanvas("canvas", "Roofit example",800,600);
	frame->Draw();
	canvas->SaveAs("basic_roofit.png");
}
