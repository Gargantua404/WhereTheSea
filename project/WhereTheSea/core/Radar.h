#include <vector>
#include <list>
#include "../libEasyBMP/EasyBMP.h"
#include <cstdlib>
#include <time.h>

#include <../libGeographic/include/GeographicLib/LocalCartesian.hpp>
using namespace GeographicLib;

using namespace std;

bool operator == (const RGBApixel a, const RGBApixel b);
class Radar
{
private:
	int freq = 1;
    int YYYY = 0, MM = 0, DD = 0, hh = 0, mm = 0, ss = 0, cc = 0;
    int oYYYY, oMM, oDD, ohh, omm, oss, occ;
    int blackAndWhitePixels, clearedPixels;
    double scale = 100;
    double noize;
	bool firstImage = true;
	BMP world;
    int minMonitoredSize;
	const double maxDelta = 10;
	vector<double> ox, oy, os, ovx, ovy;
    vector<bool> ot; //traced
	const RGBApixel white = { 255,255,255,0 }, black = { 0,0,0,0 }, green = { 0,255,0,0 }, red = { 0,0,255,0 };
	string logFileName, outputFileName;
	FILE *logFile, *outputFile;
	int dx[4] = { 0, 1, 0, -1 }, dy[4] = { 1, 0, -1, 0 };

    double lat0 ,olat0; //latitude in degrees [-90,90] of the center of the image
    double lon0, olon0; //longitude in degrees [-180,180] of the center of the image
    LocalCartesian LocGeoc; // class for geographic to cartesian coordinates convertation
    //initialized by (lat=0,lon=0,h=0)

	bool iw(int x, int y);
	void cleanPixel(int x, int y);
    double clean();
	vector<double> fillingPixel(RGBApixel from, RGBApixel to, int x, int y);
	void findObjects();
	double delta(double x1, double y1, double s1, double x2, double y2, double s2);
	BMP nextStep(BMP image, bool createRedArrows);
    double timeInterval();
	void writeOutput();
public:
	Radar(double newScale);
	~Radar();
	void setFreq(int newFreq);
	void clearCashe();
	void setOutputFile(const string newOutputFileName);
	void setLogFile(const string newLogFileName);
    void setLogFile(FILE * newLogFile);
    void setScale(int newScale);
    void setMOA(double newMOA);
	int run(const list<string> inputFileNames, bool createOutputImage, const string outputImageFileName);
	int run(const list<string> inputFileNames);
	BMP getImage();

    int degree2Full(double degIn, char type, char &signOut, int &degOut, int &minOut, double &secOut);
    int full2Degree(char signIn,int degIn, int minIn, double secIn, char type, double &degOut);

};
