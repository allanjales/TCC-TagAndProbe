#include <sstream>
#include <iomanip>

double roundTo(double value, int decimal)
{
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(decimal) << value;
	std::string valueStr = oss.str();
	return std::stod(valueStr);
}

void roundTH2DLabels(TH2D* heff, int decimal)
{
    for (int i = 0; i<=heff->GetNbinsX(); i++)
    {
        for (int j = 0; j<=heff->GetNbinsY(); j++)
        {
            double value = heff->GetBinContent(i,j);
            double uncertain = heff->GetBinError(i,j);
            heff->SetBinContent(i,j, roundTo(value, decimal));
            heff->SetBinError(i,j, roundTo(uncertain, decimal));
        }
    }
}