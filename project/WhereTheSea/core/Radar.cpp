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
                /*if (noize < 0.15) minMonitoredSize = (int)(noize * 100);
                else minMonitoredSize = (int)(noize * 100 * (noize * 10 - 0.5));*/
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
        /*double xd = (ox[i] - ((double)world.TellWidth() / 2)); //������������� � �������� ����������
		double yd = (oy[i] - ((double)world.TellHeight() / 2));
        double objectDistance = sqrt(xd * xd + yd * yd) * scale;
        double objectAzimuth = atan2(xd, -yd);*/

        double lat=0,lon=0, h=0;
        LocGeoc.Reverse(ox[i]*scale,oy[i]*scale,0.0,lat,lon,h); //get geographic from cartesian coordinates of the objects

        /*double speedDistance = sqrt(ovx[i] * ovx[i] + ovy[i] * ovy[i]) / scale * timeInterval(); //� ��������� ������� �� ������� ����� �������������
        double speedAzimuth = atan2(ovx[i], -ovy[i]);*/

        double objectRealSize = os[i] * scale * scale;

        //fprintf(outputFile, "%f %f %f ", objectRealSize, objectDistance, objectAzimuth);
        char signLat='+',signLon='-';
        int degLat=0, degLon=0, minLat=0, minLon=0;
        double secLat=0.0, secLon=0.0;
        degree2Full(lat,'a',signLat,degLat,minLat,secLat);
        degree2Full(lon,'o',signLon,degLon,minLon,secLon);
        fprintf(outputFile, "%15f %1c%2d %2d %15f %1c%3d %2d %15f", objectRealSize,signLat, degLat, minLat, secLat,signLon, degLon, minLon, secLon);
        if (ot[i]){
            //fprintf(outputFile, "%f %f\n", speedDistance, speedAzimuth);
            //taking into account  the bias of the beginning of coordinate system
            double dOx=0,dOy=0,dOh=0;
            LocGeoc.Forward(olat0,olon0,0.0,dOx,dOy,dOh);// get cartesian from geographic coordinates
            fprintf(outputFile, "%+15f %15f\n", (ovx[i]-dOx)*scale/timeInterval(), (ovy[i]-dOy)*scale/timeInterval());
        }
        else
            //fprintf(outputFile, "Speed unknown\n");
            fprintf(outputFile, "    UNKNOWN          UNKNOWN    \n");
	}
	fclose(outputFile);
	outputFile = NULL;
}

//��� ������������� private ������ � ���������� public

//�����������. Scale - ���������� ������ � ������
Radar::Radar(double newScale):lat0(0),olat0(0),lon0(0),olon0(0),LocGeoc(0,0)
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

void Radar::setMOA(double newMOA){
    minMonitoredSize = (int)ceil(newMOA / (scale * scale));
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

    //save previous date
	if (!firstImage)
	{
		oYYYY = YYYY;
		oMM = MM;
		oDD = DD;
		ohh = hh;
		omm = mm;
		oss = ss;
        occ = cc;
        olat0=lat0;
        olon0=lon0;
    }
    string inputImageFileName = inputFileNames.front();
    char lat0sign='+',lon0sign='+';
    int lat0d=0, lat0m=0,lon0d=0, lon0m=0;
    double lat0s=0.0, lon0s=0.0;
    sscanf(inputImageFileName.c_str(), "%d-%d-%d-%d-%d-%d-%d_%c%d-%d-%lf_%c%d-%d-%lf", &YYYY, &MM, &DD, &hh, &mm, &ss, &cc, &lat0sign,&lat0d, &lat0m,&lat0s, &lon0sign,&lon0d, &lon0m, &lon0s);
    if (!(lat0sign=='+' || lat0sign=='-')|| !(lon0sign=='+'|| lon0sign=='-')||MM<1 || MM >12 || DD<1 || DD >31 || hh <0 || hh>=24 || mm <0 || mm>=60 || ss<0 || ss>=60 || cc<0 || cc>0)
    {
    if (logFile != NULL)
    fprintf(logFile, "Incorrect date format in the name of the file: %s", inputImageFileName.c_str());
    return -1;
    }
    if(full2Degree(lat0sign,lat0d, lat0m, lat0s, 'a', lat0) !=0 || full2Degree(lon0sign,lon0d, lon0m, lon0s, 'o', lon0) !=0)
    {
    if (logFile != NULL)
    fprintf(logFile, "Incorrect format of geographic coordinates in the name of the file: %s", inputImageFileName.c_str());
    return -1;
    }
    LocGeoc.Reset(lat0,lon0,0);

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

int Radar::degree2Full(double degIn, char type, char &signOut,int &degOut, int &minOut, double &secOut){
    ///type: a - lattitude , o-longitude
    if( ((degIn)>90 && type=='a') || ((degIn<-180 || degIn>180) && type=='o')){
        return -1;
    }
    else{
        int mult=0;
        if(degIn<0){
            signOut='-';
            mult=-1;
        }else{
            signOut='+';
            mult=1;
        }
        degOut=int(degIn)*mult;
        double temp = (degIn - degOut*mult)*60;
        minOut= int(temp)*mult;
        secOut= mult*(temp-minOut*mult)*60;
        return 0;
    }
}

int Radar::full2Degree(char signIn,int degIn, int minIn, double secIn, char type, double &degOut){
    ///type: a - langitude , o-longitude
    if(!(signIn =='+' || signIn =='-' )){
        return -1;
    }
    else if( ( (degIn>90 ||degIn<0)  && type=='a') || ((degIn<0 || degIn>180) && type=='o') || minIn>=60 || minIn<0 || secIn>=60|| secIn<0){
        return -1;
    }
    else{
        int mult=0;
        if(signIn=='-'){
            mult=-1;
        }else if(signIn=='+'){
            mult=1;
        }
        else{
            return -1;
        }
        degOut= mult*(degIn + (minIn/60) + (secIn/3600));
        return 0;
    }
}
