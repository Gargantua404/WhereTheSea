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
	vector<double> oldox = ox, oldoy = oy, oldos = os;
	vector<int> costsi, costsj;
	world = *(new BMP(image));
	clean();
	findObjects();
	if (firstImage)
	{
		firstImage = false;
		return world;
	}
	BMP imageOut = world;
	vector<vector<double>> costs;
	for (int i = 0; i < oldox.size(); i++)
		costsi.push_back(i);
	for (int j = 0; j < ox.size(); j++)
		costsj.push_back(j);
	for (int i = 0; i < oldox.size(); i++)
	{
		vector<double> newvector;
		costs.push_back(newvector);
		for (int j = 0; j < ox.size(); j++)
			costs[i].push_back(delta(oldox[i], oldoy[i], oldos[i], ox[j], oy[j], os[j]));
	}
	if (costs.size() > costs[0].size()) //now less
	{
		vector<double> mins;
		for (int i = 0; i < costs.size(); i++)
		{
			int minarg = 0;
			vector<double> tl = costs[i];
			for (int j = 0; j < costs[0].size(); j++)
				if (tl[j] < tl[minarg])
					minarg = j;
			mins.push_back(tl[minarg]);
		}
		while (costs.size() > costs[0].size())
		{
			int maxminarg = 0;
			for (int i = 0; i < mins.size(); i++)
				if (mins[i] > mins[maxminarg]) maxminarg = i;
			mins.erase(mins.begin() + maxminarg);
			costsi.erase(costsi.begin() + maxminarg);
			costs.erase(costs.begin() + maxminarg);
		}
	}
	if (costs.size() < costs[0].size()) //now more
	{
		vector<double> mins;
		for (int j = 0; j < costs[0].size(); j++)
		{
			int minarg = 0;
			vector<double> tl1, tl2 = costs[0];
			for (int i = 0; i < costs.size(); i++)
			{
				tl1 = costs[i];
				if (tl1[j] < tl2[j])
				{
					minarg = i;
					tl2 = costs[minarg];
				}
			}
			mins.push_back(tl2[j]);
		}
		while (costs.size() < costs[0].size())
		{
			int maxminarg = 0;
			for (int i = 0; i < mins.size(); i++)
				if (mins[i] > mins[maxminarg]) maxminarg = i;
			mins.erase(mins.begin() + maxminarg);
			costsj.erase(costsj.begin() + maxminarg);
			for (int i = 0; i < costs.size(); i++)
				costs[i].erase(costs[i].begin() + maxminarg);
		}
	}
	while (costs.size() > 0)
	{
		int minargi = 0;
		int minargj = 0;
		vector<double> tl1, tl2 = costs[0];
		for (int i = 0; i < costs.size(); i++)
		{
			tl1 = costs[i];
			for (int j = 0; j < costs[0].size(); j++)
				if (tl1[j] < tl2[minargj])
				{
					minargi = i;
					tl2 = costs[minargi];
					minargj = j;
				}
		}
		int oo = costsi[minargi]; //oldobject
		int no = costsj[minargj]; //newobject
		if (tl2[minargj] <= maxdelta)
		{
			double x = ox[no], y = oy[no];
			double tx = x + (x - oldox[oo]) * 2, ty = y + (y - oldoy[oo]) * 2;
			ovx[no] = ox[no] - oldox[oo];
			ovy[no] = oy[no] - oldoy[oo];
			while ((x - tx) * (x - tx) + (y - ty) * (y - ty) > 1)
			{
				if (iw((int)x, (int)y))
					imageOut.SetPixel((int)x, (int)y, red);
				double dcoord = sqrt((x - tx) * (x - tx) + (y - ty) * (y - ty));
				x += (tx - x) / dcoord;
				y += (ty - y) / dcoord;
			}
			costs.erase(costs.begin() + minargi);
			costsi.erase(costsi.begin() + minargi);
			for (int i = 0; i < costs.size(); i++)
				costs[i].erase(costs[i].begin() + minargj);
			costsj.erase(costsj.begin() + minargj);
		}
		else costs.clear();
	}
	return imageOut;
}
void Radar::writeLog()
{
	char str[200];
	sprintf_s(str, 200, "tick #%d\n", tick);
	fputs(str, logFile);
	tick++;
	for (int i = 0; i < ox.size(); i++)
	{
		sprintf_s(str, 200, "%f %f %f %f %f\n", ox[i], oy[i], os[i], ovx[i], ovy[i]);
		fputs(str, logFile);
	}
}
Radar::Radar()
{
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
void Radar::setPath(const string newOutputFileName)
{
	outputFile = *(new string(newOutputFileName));
}
void Radar::setPath(FILE *newLogFile)
{
	logFile = newLogFile;
}
int Radar::run(const list<string> inputFileNames)
{
	if (inputFileNames.size() != freq) return 1;
	BMP inputImage;
	inputImage.ReadFromFile(inputFileNames.front().c_str());
	nextStep(inputImage).WriteToFile(outputFile.c_str());
	writeLog();
	return 0;
}
BMP Radar::getImage()
{
	return world;
}