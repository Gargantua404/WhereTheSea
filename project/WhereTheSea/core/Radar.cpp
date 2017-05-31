#define _CRT_SECURE_NO_WARNINGS
#include "Radar.h"
//��������� ������ ��������, ������-�� ����� ��� � ����������
bool operator == (const RGBApixel a, const RGBApixel b) {
	return a.Red == b.Red && a.Green == b.Green && a.Blue == b.Blue && a.Alpha == b.Alpha;
}
//���������, �� ������� �� ���������� ������� �� ������� ��������
bool Radar::iw(int x, int y)
{
	if (x < 0)
		return false;
	if (y < 0)
		return false;
	if (x >= world.TellWidth())
		return false;
	if (y >= world.TellHeight())
		return false;
    if (!((world.GetPixel(x,y) == white) || (world.GetPixel(x,y) == black) || (world.GetPixel(x,y) == green)))
        return false;
	return true;
}
//���� ����� � ������ ��� ����� �������� ������ ���� ����� ��, ������������� � ��������������� ���� � ���������� �� �������
void Radar::cleanPixel(int x, int y)
{
	RGBApixel p = world.GetPixel(x, y);
	if ((p == white) || (p == black))
	{
        blackAndWhitePixels++;
        int same = 0, all = 0;
		for (int h = 0; h < 4; h++)
			if (iw(x + dx[h], y + dy[h]))
            {
                all++;
				if (world.GetPixel(x + dx[h], y + dy[h]) == p)
					same++;
            }
        if ((same < 2) && (all > 1))
		{
            clearedPixels++;
			if (p == white)
				world.SetPixel(x, y, black);
			if (p == black)
				world.SetPixel(x, y, white);
            for (int h = 0; h < 4; h++)
				if (iw(x + dx[h], y + dy[h]))
                    cleanPixel(x + dx[h], y + dy[h]);
		}
	}
}
//�������� cleanPixel �� ���� �������� �����������, ����� ������� ������� ����������� �� ����
double Radar::clean()
{
    blackAndWhitePixels = 0;
    clearedPixels = 0;
	for (int x = 0; x < world.TellWidth(); x++)
		for (int y = 0; y < world.TellHeight(); y++)
			cleanPixel(x, y);
    return (double)clearedPixels / blackAndWhitePixels;
}
//�������, ��������������, ������� �������� ������, � ����� ����� ����� � ������� ���� ��������
vector<double> Radar::fillingPixel(RGBApixel from, RGBApixel to, int x, int y)
{
	vector<double> fdata = *(new vector<double>(3));
	fdata[0] = 0; fdata[1] = 0; fdata[2] = 0;
	if (!iw(x, y))
		return fdata;
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
//�������� �������: ������������� ������ ����� �������� � ������, ���� ���������, ��� �� ������� ����, �� ������������� � ������, ����� ����������
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
                if (noize < 0.15) minMonitoredSize = (int)(noize * 100);
                else minMonitoredSize = (int)(noize * 100 * (noize * 10 - 0.5));
				if (odata[0] >= minMonitoredSize)
				{
					os.push_back(odata[0]);
					ox.push_back(odata[1] / odata[0]);
					oy.push_back(odata[2] / odata[0]);
				}
				else
					fillingPixel(green, black, x, y);
			}
		}
    ovx = *(new vector<double>(ox.size(), 0));
	ovy = *(new vector<double>(ox.size(), 0));
}
//������� ����������� ��������, ����� � ������ ��������
double Radar::delta(double x1, double y1, double s1, double x2, double y2, double s2)
{
	double d = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
	double ds = s1 / s2;
	if (ds < 1)
		ds = 1 / ds;
    return 20 * d / (s1 + s2) + ds;
}
//��������� �����������, ���� �������, ���������� ������� �� ������� �� ������� �����������, ���������� �� �������� �� ��������� ���������
BMP Radar::nextStep(BMP image, bool createRedArrows)
{
	vector<double> oldOx = ox, oldOy = oy, oldOs = os; //���������� ������� � ����������� �����������
	world = *(new BMP(image));
    noize = clean();
	findObjects();
    ot = *(new vector<bool>(ox.size(), false)); //���������� �������� �������� ����������
	if (firstImage) //���� ����������� ������, �� �� ���� �� ����� ���������� �������� ��������
	{
		firstImage = false;
		return world;
	}
    vector<int> costsI, costsJ; //������� ��� ���������, ����� ������ ��� ���������� � ������/������� ������� costs
	vector<vector<double>> costs = *(new vector<vector<double>>(oldOx.size(), *(new vector<double>))); //������� ����������� ��������
	BMP imageOut = world;
	for (int i = 0; i < oldOx.size(); i++)
		costsI.push_back(i);
	for (int j = 0; j < ox.size(); j++)
		costsJ.push_back(j);
	for (int i = 0; i < oldOx.size(); i++)
		for (int j = 0; j < ox.size(); j++)
			costs[i].push_back(delta(oldOx[i], oldOy[i], oldOs[i], ox[j], oy[j], os[j])); //��������� �������
	while ((costs.size() > 0) && (costs[0].size() > 0)) //���� ������� �� ����� (���� � ������� �� ��������� ��������� ���������, ��� ����� ���������� � ������)
	{
		int minArgI = 0;
		int minArgJ = 0;
		for (int i = 0; i < costs.size(); i++) //������� ���������� �������
		{
			for (int j = 0; j < costs[0].size(); j++)
				if (costs[i][j] < costs[minArgI][minArgJ])
				{
					minArgI = i;
					minArgJ = j;
				}
		}
		int oo = costsI[minArgI];
		int no = costsJ[minArgJ];
		if (costs[minArgI][minArgJ] <= maxDelta) //���� �� ���������
		{
			double x = ox[no], y = oy[no];
			ovx[no] = ox[no] - oldOx[oo]; //���������� �������� �������
			ovy[no] = oy[no] - oldOy[oo];
            ot[no] = true; //� ����������, ��� �� � �����
			if (createRedArrows) //���� ����� ������� ����� �������� (��� ����� ������ ��� ������, ������� �� ����������� �� �������)
			{
				double tx = x + (x - oldOx[oo]) * 2, ty = y + (y - oldOy[oo]) * 2;
				while ((x - tx) * (x - tx) + (y - ty) * (y - ty) > 1) //������ ������� �����, ������������ ��������
				{
					if (iw((int)x, (int)y))
						imageOut.SetPixel((int)x, (int)y, red);
					double dCoord = sqrt((x - tx) * (x - tx) + (y - ty) * (y - ty));
					x += (tx - x) / dCoord;
					y += (ty - y) / dCoord;
				}
			}
			costs.erase(costs.begin() + minArgI); //������� ������� � ������ �� �������
			costsI.erase(costsI.begin() + minArgI); //����� ������� ��������������� �������� �� ��������������� ��������
			costsJ.erase(costsJ.begin() + minArgJ);
			for (int i = 0; i < costs.size(); i++)
				costs[i].erase(costs[i].begin() + minArgJ);
		}
		else costs.clear(); //���� ���� ���������� ������� ������� �������, ������� ������� � ������� �� �����
	}
	return imageOut;
}
double Radar::timeInterval()
{
    double res = 60 * (mm - omm) + ss - oss + (double)(cc - occ) / 100;
	if (ohh != hh) res += 3600;
	return res;
}
//������� ������ ��������� �������� � ���� ������.
void Radar::writeOutput()
{
	outputFile = fopen(outputFileName.c_str(), "a");
    fprintf(outputFile, "%04d-%02d-%02d-%02d-%02d-%02d-%02d\n", YYYY, MM, DD, hh, mm, ss, cc);
    fprintf(outputFile, "noize = %f\n", noize);
	for (int i = 0; i < ox.size(); i++)
	{
		double xd = (ox[i] - ((double)world.TellWidth() / 2)); //������������� � �������� ����������
		double yd = (oy[i] - ((double)world.TellHeight() / 2));
        double objectDistance = sqrt(xd * xd + yd * yd) * scale;
		double objectAzimuth = atan2(xd, -yd);
		double speedDistance = sqrt(ovx[i] * ovx[i] + ovy[i] * ovy[i]) / scale * timeInterval(); //� ��������� ������� �� ������� ����� �������������
		double speedAzimuth = atan2(ovx[i], -ovy[i]);
        double objectRealSize = os[i] * scale * scale;
        fprintf(outputFile, "%f %f %f ", objectRealSize, objectDistance, objectAzimuth);
        if (ot[i])
            fprintf(outputFile, "%f %f\n", speedDistance, speedAzimuth);
        else
            fprintf(outputFile, "Speed unknown\n");
	}
	fclose(outputFile);
	outputFile = NULL;
}

//��� ������������� private ������ � ���������� public

//�����������. Scale - ���������� ������ � ������
Radar::Radar(double newScale)
{
	scale = newScale;
	logFile = NULL;
	outputFile = NULL;
}
//���������� ��������� ������ �� �����, ������� ����������� �������� ������
Radar::~Radar()
{
	if (outputFile != NULL)
		fclose(outputFile);
	if (logFile != NULL)
		fclose(logFile);
}
//����� �����-������ ����� �������� �������
void Radar::setFreq(int newFreq)
{
}

void Radar::setScale(int newScale)
{
    scale=newScale;
}

void Radar::setIdentThreshold(double newThreshold){
    //MUFFLE
}

//�������� ������� � ������� �����������
void Radar::clearCashe()
{
	firstImage = true;
	ox.clear();
	oy.clear();
	os.clear();
}
//���������� ���� ������
void Radar::setOutputFile(const string newOutputFileName)
{
    if (logFile != NULL)
        fprintf(logFile, "Set new output file %s\n", newOutputFileName.c_str());
	if (outputFile != NULL)
		fclose(outputFile);
	outputFileName = newOutputFileName;
}
//���������� ���� � �����
void Radar::setLogFile(const string newLogFileName)
{
	if (logFile != NULL)
	{
		fprintf(logFile, "Set new log file %s\n", newLogFileName.c_str());
		fclose(logFile);
	}
	logFileName = newLogFileName;
	logFile = fopen(logFileName.c_str(), "a");
	fprintf(logFile, "This is new log file\n");
}
void Radar::setLogFile(FILE *newLogFile)
{
    logFileName="";
    logFile =newLogFile;
}

//��������� �������� ����� � ����������� (������ ������ ������) � ����� �� �������� ����������� � ��� ����� ��� ����
//���������� ����, ���� �� ������, ������ ���������� ������ ���� ������, �� ����� ���� ����� ����� ������
int Radar::run(const list<string> inputFileNames, bool createOutputImage, const string outputImageFileName)
{
	if (logFile != NULL) //������� ���
	{
		fprintf(logFile, "run:\n inputFileNames = ");
		for (list<string>::const_iterator i = inputFileNames.begin(); i != inputFileNames.end(); i++)
		{
			if (i != inputFileNames.begin()) 
				fprintf(logFile, ", ");
			fprintf(logFile, (*i).c_str());
		}
		fprintf(logFile, "\n");
		if (createOutputImage)
			fprintf(logFile, " outputImageFileName = %s\n", outputImageFileName.c_str());
	}
	if (inputFileNames.size() != freq) //������ ������ ���� ������ �����
		return 1;
	string inputImageFileName = inputFileNames.front();
	size_t found = inputImageFileName.find_last_of("/\\"); //�������� ����� �� �������� �����
	if (!firstImage)
	{
		oYYYY = YYYY;
		oMM = MM;
		oDD = DD;
		ohh = hh;
		omm = mm;
		oss = ss;
        occ = cc;
	}
    sscanf(inputImageFileName.substr(found+1).c_str(), "%d-%d-%d-%d-%d-%d-%d", &YYYY, &MM, &DD, &hh, &mm, &ss, &cc);
	BMP inputImage;
	inputImage.ReadFromFile(inputImageFileName.c_str());
	if (createOutputImage) //���� ����� �������� �����������, ������ ���, ����� ������ ������������ ������
		nextStep(inputImage, createOutputImage).WriteToFile(outputImageFileName.c_str());
	else
		nextStep(inputImage, createOutputImage);
	writeOutput(); //����� � ���� ������
	return 0;
}
//���� �������� ����������� �� �����, �� ��� �������� ������������ ���� �������
int Radar::run(const list<string> inputFileNames)
{
	return run(inputFileNames, false, "");
}
//���������� ������� ����������� ����, ����� ��� �������
BMP Radar::getImage()
{
	return world;
}
