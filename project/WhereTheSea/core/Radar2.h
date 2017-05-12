#include <vector>
#include <list>
#include "../libEasyBMP/EasyBMP.h"
#include <cstdlib>
#include <time.h>

using namespace std;
bool operator == (const RGBApixel a, const RGBApixel b);
class Radar
{
private:
	int freq = 1;
	int YYYY = 0, MM = 0, DD = 0, hh = 0, mm = 0, ss = 0;
	int oYYYY, oMM, oDD, ohh, omm, oss;
	double latitude, longitude, scale;
	bool firstImage = true;
	BMP world;
	const int minMonitoredSize = 15;
	const double maxDelta = 10;
	vector<double> ox, oy, os, ovx, ovy;
	const RGBApixel white = { 255,255,255,0 }, black = { 0,0,0,0 }, green = { 0,255,0,0 }, red = { 0,0,255,0 };
	string logFileName, outputFileName;
	FILE *logFile, *outputFile;
	int dx[4] = { 0, 1, 0, -1 }, dy[4] = { 1, 0, -1, 0 };
	bool iw(int x, int y);
	void cleanPixel(int x, int y);
	void clean();
	vector<double> fillingPixel(RGBApixel from, RGBApixel to, int x, int y);
	void findObjects();
	double delta(double x1, double y1, double s1, double x2, double y2, double s2);
	BMP nextStep(BMP image, bool createRedArrows);
	int timeInterval();
	void writeOutput();
public:
	Radar(double newScale);
	~Radar();
	void setFreq(int newFreq);
	void clearCashe();
	void setOutputFile(const string newOutputFileName);
	void setLogFile(const string newLogFileName);
	int run(const list<string> inputFileNames, bool createOutputImage, const string outputImageFileName);
	int run(const list<string> inputFileNames);
	BMP getImage();
};
