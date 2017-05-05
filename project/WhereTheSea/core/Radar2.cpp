#include "Radar2.h"

bool operator == (const RGBApixel a, const RGBApixel b) {
	return a.Red == b.Red && a.Green == b.Green && a.Blue == b.Blue && a.Alpha == b.Alpha;
}

bool Radar::iw(int x, int y)
{
	if (x < 0) return false;
	if (y < 0) return false;
	if (x >= world.TellWidth()) return false;
	if (y >= world.TellHeight()) return false;
	return true;
}
void Radar::cleanPixel(int x, int y)
	{
		RGBApixel p = world.GetPixel(x, y);
		if ((p == white) || (p == black))
		{
			int same = 0;
			for (int h = 0; h < 4; h++)
				if (iw(x + dx[h], y + dy[h]))
					if (world.GetPixel(x + dx[h], y + dy[h]) == p) same++;
			if (same < 2)
			{
				if (p == white) world.SetPixel(x, y, black);
				if (p == black) world.SetPixel(x, y, white);
				for (int h = 0; h < 4; h++)
					if (iw(x + dx[h], y + dy[h]))
						cleanPixel(x + dx[h], y + dy[h]);
			}
		}
	}
void Radar::clean()
{
	for (int x = 0; x < world.TellWidth(); x++)
		for (int y = 0; y < world.TellHeight(); y++)
			cleanPixel(x, y);
}
vector<double> Radar::fillingPixel(RGBApixel from, RGBApixel to, int x, int y)
{
	vector<double> fdata = *(new vector<double>(3));
	fdata[0] = 0; fdata[1] = 0; fdata[2] = 0;
	if (!iw(x, y)) return fdata;
	if (world.GetPixel(x, y) == from)
	{
		world.SetPixel(x, y, to);
		fdata[0] = 1; fdata[1] = x; fdata[2] = y;
		for (int h = 0; h < 4; h++)
		{

			vector<double> fdata1 = fillingPixel(from, to, x + dx[h], y + dy[h]);
			fdata[0] += fdata1[0];
			fdata[1] += fdata1[1];
			fdata[2] += fdata1[2];
		}
	}
	return fdata;
}
void Radar::findObjects()
{
	ox.clear();
	oy.clear();
	os.clear();
	for (int x = 0; x < world.TellWidth(); x++)
		for (int y = 0; y < world.TellHeight(); y++)
		{
			RGBApixel p = world.GetPixel(x, y);
			if (p == white)
			{
				vector<double> odata = fillingPixel(white, green, x, y);
				if (odata[0] >= minMonitoredSize)
				{
					os.push_back(odata[0]);
					ox.push_back(odata[1] / odata[0]);
					oy.push_back(odata[2] / odata[0]);
				}
				else if (odata[0] >= minDisplayedSize) fillingPixel(green, white, x, y);
				else fillingPixel(green, black, x, y);
			}
		}
	ovx = *(new vector<double>(ox.size(), 0));
	ovy = *(new vector<double>(ox.size(), 0));
}
double Radar::delta(double x1, double y1, double s1, double x2, double y2, double s2)
{
	double d = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
	double ds = s1 / s2;
	if (ds < 1) ds = 1 / ds;
	return 20 * d / (s1 + s2) + ds;
}
BMP Radar::nextStep(BMP image)
{
	vector<double> oldOx = ox, oldOy = oy, oldOs = os;
	world = *(new BMP(image));
	clean();
	findObjects();
	if (firstImage)
	{
		firstImage = false;
		return world;
	}
	vector<int> costsI, costsJ;
	vector<vector<double>> costs;
	BMP imageOut = world;
	for (int i = 0; i < oldOx.size(); i++)
		costsI.push_back(i);
	for (int j = 0; j < ox.size(); j++)
		costsJ.push_back(j);
	costs.resize(oldOx.size(), *(new vector<double>));
	for (int i = 0; i < oldOx.size(); i++)
		for (int j = 0; j < ox.size(); j++)
			costs[i].push_back(delta(oldOx[i], oldOy[i], oldOs[i], ox[j], oy[j], os[j]));
	while ((costs.size() > 0) && (costs[0].size() > 0))
	{
		int minArgI = 0;
		int minArgJ = 0;
		vector<double> tl1, tl2 = costs[0];
		for (int i = 0; i < costs.size(); i++)
		{
			tl1 = costs[i];
			for (int j = 0; j < costs[0].size(); j++)
				if (tl1[j] < tl2[minArgJ])
				{
					minArgI = i;
					tl2 = costs[minArgI];
					minArgJ = j;
				}
		}
		int oo = costsI[minArgI]; //oldobject
		int no = costsJ[minArgJ]; //newobject
		if (tl2[minArgJ] <= maxDelta)
		{
			double x = ox[no], y = oy[no];
			double tx = x + (x - oldOx[oo]) * 2, ty = y + (y - oldOy[oo]) * 2;
			ovx[no] = ox[no] - oldOx[oo];
			ovy[no] = oy[no] - oldOy[oo];
			while ((x - tx) * (x - tx) + (y - ty) * (y - ty) > 1)
			{
				if (iw((int)x, (int)y))
					imageOut.SetPixel((int)x, (int)y, red);
				double dCoord = sqrt((x - tx) * (x - tx) + (y - ty) * (y - ty));
				x += (tx - x) / dCoord;
				y += (ty - y) / dCoord;
			}
			costs.erase(costs.begin() + minArgI);
			costsI.erase(costsI.begin() + minArgI);
			costsJ.erase(costsJ.begin() + minArgJ);
			for (int i = 0; i < costs.size(); i++)
				costs[i].erase(costs[i].begin() + minArgJ);
		}
		else costs.clear();
	}
	return imageOut;
}
void Radar::writeOutput()
{
    fprintf(outputFile,"tick #%d\n",tick);
	tick++;
	for (int i = 0; i < ox.size(); i++)
	{
        fprintf(outputFile,"%f %f %f %f %f\n",ox[i], oy[i], os[i], ovx[i], ovy[i]);
	}
}
Radar::Radar()
{
    logFile=NULL;
    outputFile=NULL;
}
Radar::~Radar()
{
	if (shouldCloseOutputFile) fclose(outputFile);
	if (shouldCloseLogFile) fclose(logFile);
}
void Radar::setFreq(int newFreq)
{
}
void Radar::clearCashe()
{
	firstImage = true;
	ox.clear();
	oy.clear();
	os.clear();
}
void Radar::setOutputFile(const string newOutputFileName)
{
	if (logFile != NULL)
	{
        fprintf(logFile,"Set new output file with path %s\n", newOutputFileName.c_str());
	}
	if (shouldCloseOutputFile) fclose(outputFile);
    fopen_s(&outputFile, newOutputFileName.c_str(), "a");
	shouldCloseOutputFile = true;
}
void Radar::setOutputFile(FILE *newOutputFile)
{
	if (logFile != NULL) fputs("Set new output file\n", logFile);
	if (shouldCloseOutputFile) fclose(outputFile);
	outputFile = newOutputFile;
	shouldCloseOutputFile = false;
}
void Radar::setLogFile(const string newLogFileName)
{
    if (logFile != NULL)
	{
        fprintf(logFile,"Set new log file with path %s\n", newLogFileName.c_str());
	}
	if (shouldCloseLogFile) fclose(logFile);
	fopen_s(&logFile, newLogFileName.c_str(), "a");
	shouldCloseLogFile = true;
	if (logFile != NULL) fputs("This is new log file\n", logFile);
}
void Radar::setLogFile(FILE *newLogFile)
{

    if (logFile != NULL) fputs("Set new log file\n", logFile);
    if (shouldCloseLogFile) fclose(logFile);
	logFile = newLogFile;
	shouldCloseLogFile = false;
	if (logFile != NULL) fputs("This is new log file\n", logFile);
}
int Radar::run(const list<string> inputFileNames, bool createOutputImage, const string outputImageFileName)
{
	if (logFile != NULL)
	{
		fputs("run:\n", logFile);
		fputs(" inputFileNames = ", logFile);
		for (list<string>::const_iterator i = inputFileNames.begin(); i != inputFileNames.end(); i++)
		{
			if (i != inputFileNames.begin()) fputs(", ", logFile);
			fputs((*i).c_str(), logFile);
		}
		fputs("\n", logFile);
		if (createOutputImage)
		{
            fprintf(logFile," outputImageFileName = %s\n", outputImageFileName.c_str());
		}
	}
	if (inputFileNames.size() != freq) return 1;
	string inputImageFileName = inputFileNames.front();
	BMP inputImage;
	inputImage.ReadFromFile(inputImageFileName.c_str());
	if (createOutputImage) nextStep(inputImage).WriteToFile(outputImageFileName.c_str());
	else nextStep(inputImage);
	writeOutput();
	return 0;
}
int Radar::run(const list<string> inputFileNames)
{
	return run(inputFileNames, false, "");
}
BMP Radar::getImage()
{
	return world;
}
