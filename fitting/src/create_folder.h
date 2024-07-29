#include <iostream>
using namespace std;
#include <TSystem.h>

void create_folder(const char* folderPath, bool deleteOld = false)
{
	//Delete old directory for saving files
	if (deleteOld == true)
		gSystem->Exec((string("rm -r ") + string(folderPath) + string("*")).c_str());

	//Check if dir exists and create
	if (gSystem->AccessPathName(folderPath))
		if (gSystem->mkdir(folderPath, true))
		{
			cerr << "ERROR: \"" << folderPath << "\" path could not be found and could not be created\n";
			cerr << "Sugestion: try to create manually this folder path\n";
			abort();
		}
		else
			cout << "Directory created at \"" << folderPath << "\" OK\n";
	else
		cout << "Directory found at \"" << folderPath << "\" OK\n";
}