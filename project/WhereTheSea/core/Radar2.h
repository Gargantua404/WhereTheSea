#include <vector>
#include <list>
#include "../libEasyBMP/EasyBMP.h"
#include <cstdlib>

using namespace std;
bool operator == (const RGBApixel a, const RGBApixel b);
class Radar
{
private:
	int freq = 1, tick = 1;
	bool firstImage = true, shouldCloseOutputFile = false, shouldCloseLogFile = false;
	BMP world;
	const int minDisplayedSize = 15, minMonitoredSize = 15;
	const double maxDelta = 10;
	vector<double> ox, oy, os, ovx, ovy;
	RGBApixel white = { 255,255,255,0 }, black = { 0,0,0,0 }, green = { 0,255,0,0 }, red = { 0,0,255,0 };
	FILE *logFile, *outputFile;
	int dx[4] = { 0, 1, 0, -1 }, dy[4] = { 1, 0, -1, 0 };
	bool iw(int x, int y);
	void cleanPixel(int x, int y);
	void clean();
	vector<double> fillingPixel(RGBApixel from, RGBApixel to, int x, int y);
	void findObjects();
	double delta(double x1, double y1, double s1, double x2, double y2, double s2);
	BMP nextStep(BMP image);
	void writeOutput();
public:
	Radar();
	~Radar();
	void setFreq(int newFreq);
	void clearCashe();
	void setOutputFile(const string newOutputFileName);
	void setOutputFile(FILE *newOutputFile);
	void setLogFile(const string newLogFileName);
	void setLogFile(FILE *newLogFile);
	int run(const list<string> inputFileNames, bool createOutputImage, const string outputImageFileName);
	int run(const list<string> inputFileNames);
	BMP getImage();
};
