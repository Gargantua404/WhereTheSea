#define _CRT_SECURE_NO_WARNINGS
#include "Radar.h"
//Равенство цветов пикселей, почему-то этого нет в библиотеке
bool operator == (const RGBApixel a, const RGBApixel b) {
	return a.Red == b.Red && a.Green == b.Green && a.Blue == b.Blue && a.Alpha == b.Alpha;
}
//Проверяем, не выходят ли координаты пикселя за границу картинки
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
//Если рядом с чёрным или белым пикселем меньше двух таких же, перекрашиваем в противополодный цвет и вызываемся от соседей
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
//Вызываем cleanPixel от всех пикселей изображения, таким образом очищаем изображение от шума
double Radar::clean()
{
    blackAndWhitePixels = 0;
    clearedPixels = 0;
	for (int x = 0; x < world.TellWidth(); x++)
		for (int y = 0; y < world.TellHeight(); y++)
			cleanPixel(x, y);
    return (double)clearedPixels / blackAndWhitePixels;
}
//Заливка, подсчитывающая, сколько пикселей залили, а также сумму иксов и игреков этих пикселей
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
//Выделяем объекты: перекрашиваем группы белых пикселей в зелёный, если оказалось, что их слишком мало, то перекрашиваем в чёрный, иначе запоминаем
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
//Метрика непохожести объектов, можно и другую написать
double Radar::delta(double x1, double y1, double s1, double x2, double y2, double s2)
{
	double d = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
	double ds = s1 / s2;
	if (ds < 1)
		ds = 1 / ds;
    return 20 * d / (s1 + s2) + ds;
}
//Принимаем изображение, ищем объекты, определяем похожие на объекты со старого изображения, определяем их скорости по изменению положения
BMP Radar::nextStep(BMP image, bool createRedArrows)
{
	vector<double> oldOx = ox, oldOy = oy, oldOs = os; //Запоминаем объекты с предыдущего изображения
	world = *(new BMP(image));
    noize = clean();
	findObjects();
    ot = *(new vector<bool>(ox.size(), false)); //Изначально скорости объектов неизвестны
	if (firstImage) //Если изображение первое, то мы пока не можем определить скорости объектов
	{
		firstImage = false;
		return world;
	}
    vector<int> costsI, costsJ; //Векторы для выяснения, какой индекс был изначально у строки/столбца матрицы costs
	vector<vector<double>> costs = *(new vector<vector<double>>(oldOx.size(), *(new vector<double>))); //Матрица непохожести объектов
	BMP imageOut = world;
	for (int i = 0; i < oldOx.size(); i++)
		costsI.push_back(i);
	for (int j = 0; j < ox.size(); j++)
		costsJ.push_back(j);
	for (int i = 0; i < oldOx.size(); i++)
		for (int j = 0; j < ox.size(); j++)
			costs[i].push_back(delta(oldOx[i], oldOy[i], oldOs[i], ox[j], oy[j], os[j])); //Заполняем матрицу
	while ((costs.size() > 0) && (costs[0].size() > 0)) //Пока матрица не пуста (если в матрице не сотанется маленьких элементов, она будет превращена в пустую)
	{
		int minArgI = 0;
		int minArgJ = 0;
		for (int i = 0; i < costs.size(); i++) //Находим наименьший элемент
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
		if (costs[minArgI][minArgJ] <= maxDelta) //Если он небольшой
		{
			double x = ox[no], y = oy[no];
			ovx[no] = ox[no] - oldOx[oo]; //Запоминаем скорость объекта
			ovy[no] = oy[no] - oldOy[oo];
            ot[no] = true; //И запоминаем, что мы её знаем
			if (createRedArrows) //Если нужны красные линии скорости (они нужны только для дебага, поэтому не нормируются по времени)
			{
				double tx = x + (x - oldOx[oo]) * 2, ty = y + (y - oldOy[oo]) * 2;
				while ((x - tx) * (x - tx) + (y - ty) * (y - ty) > 1) //Рисуем красную линию, обозначающую скорость
				{
					if (iw((int)x, (int)y))
						imageOut.SetPixel((int)x, (int)y, red);
					double dCoord = sqrt((x - tx) * (x - tx) + (y - ty) * (y - ty));
					x += (tx - x) / dCoord;
					y += (ty - y) / dCoord;
				}
			}
			costs.erase(costs.begin() + minArgI); //Удаляем столбец и строку из матрицы
			costsI.erase(costsI.begin() + minArgI); //Также удаляем соответствующие значения их вспомогательных векторов
			costsJ.erase(costsJ.begin() + minArgJ);
			for (int i = 0; i < costs.size(); i++)
				costs[i].erase(costs[i].begin() + minArgJ);
		}
		else costs.clear(); //Если даже наименьший элемент слишком большой, очищаем матрицу и выходим из цикла
	}
	return imageOut;
}
double Radar::timeInterval()
{
    double res = 60 * (mm - omm) + ss - oss + (double)(cc - occ) / 100;
	if (ohh != hh) res += 3600;
	return res;
}
//Выводим список найденных объектов в файл вывода.
void Radar::writeOutput()
{
	outputFile = fopen(outputFileName.c_str(), "a");
    fprintf(outputFile, "%04d-%02d-%02d-%02d-%02d-%02d-%02d\n", YYYY, MM, DD, hh, mm, ss, cc);
    fprintf(outputFile, "noize = %f\n", noize);
	for (int i = 0; i < ox.size(); i++)
	{
        /*double xd = (ox[i] - ((double)world.TellWidth() / 2)); //Пересчитываем в полярные координаты
		double yd = (oy[i] - ((double)world.TellHeight() / 2));
        double objectDistance = sqrt(xd * xd + yd * yd) * scale;
        double objectAzimuth = atan2(xd, -yd);*/

        double lat=0,lon=0, h=0;
        LocGeoc.Reverse(ox[i]*scale,oy[i]*scale,0.0,lat,lon,h); //get geographic from cartesian coordinates of the objects

        /*double speedDistance = sqrt(ovx[i] * ovx[i] + ovy[i] * ovy[i]) / scale * timeInterval(); //И учитываем разницу во времени между изображениями
        double speedAzimuth = atan2(ovx[i], -ovy[i]);*/

        double objectRealSize = os[i] * scale * scale;

        //fprintf(outputFile, "%f %f %f ", objectRealSize, objectDistance, objectAzimuth);
        int degLat=0, degLon=0, minLat=0, minLon=0;
        double secLat=0.0, secLon=0.0;
        degree2Full(lat,'a',degLat,minLat,secLat);
        degree2Full(lon,'o',degLon,minLon,secLon);
        fprintf(outputFile, "%15f %+2d %2d %15f %2d %2d %15f", objectRealSize, degLat, minLat, secLat, degLon, minLon, secLon);
        if (ot[i]){
            //fprintf(outputFile, "%f %f\n", speedDistance, speedAzimuth);
            //taking into account  the bias of the beginning of coordinate system
            double dOx=0,dOy=0,dOh=0;
            LocGeoc.Forward(olat0,olon0,0.0,dOx,dOy,dOh);// get cartesian from geographic coordinates
            fprintf(outputFile, "%+15f %+15f\n", (ovx[i]-dOx)*scale/timeInterval(), (ovy[i]-dOy)*scale/timeInterval());
        }
        else
            //fprintf(outputFile, "Speed unknown\n");
            fprintf(outputFile, "    UNKNOWN          UNKNOWN    \n");
	}
	fclose(outputFile);
	outputFile = NULL;
}

//Тут заканчиваются private методы и начинаются public

//Конструктор. Scale - количество метров в пкселе
Radar::Radar(double newScale):lat0(0),olat0(0),lon0(0),olon0(0),LocGeoc(0,0)
{
	scale = newScale;
	logFile = NULL;
	outputFile = NULL;
}
//Деструктор закрывает только те файлы, которые открывались методами класса
Radar::~Radar()
{
	if (outputFile != NULL)
		fclose(outputFile);
	if (logFile != NULL)
		fclose(logFile);
}
//Может когда-нибудь стать непустым методом
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

//Забываем объекты с прошлых изображений
void Radar::clearCashe()
{
	firstImage = true;
	ox.clear();
	oy.clear();
	os.clear();
}
//Определяем файл вывода
void Radar::setOutputFile(const string newOutputFileName)
{
    if (logFile != NULL)
        fprintf(logFile, "Set new output file %s\n", newOutputFileName.c_str());
	if (outputFile != NULL)
		fclose(outputFile);
	outputFileName = newOutputFileName;
}
//Определяем файл с логом
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

//Принимаем название файла с изображения (сейчас всегда одного) и нужно ли выходное изображение и имя файла для него
//Возвращаем ноль, если всё хорошо, сейчас определяет только одну ошибку, но может быть потом будет больше
int Radar::run(const list<string> inputFileNames, bool createOutputImage, const string outputImageFileName)
{
	if (logFile != NULL) //Выводим лог
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
	if (inputFileNames.size() != freq) //список должен быть нужной длины
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
    /*size_t found = inputImageFileName.find_last_of("/\\"); //Выделяем время из названия файла
    const char * lastPartFileName=inputImageFileName.substr(found+1).c_str() ;
    sscanf(lastPartFileName, "%d-%d-%d-%d-%d-%d-%d", &YYYY, &MM, &DD, &hh, &mm, &ss, &cc);*/
    //scanf(inputImageFileName.c_str(), "%d-%d-%d-%d-%d-%d-%d-%f-%f", &YYYY, &MM, &DD, &hh, &mm, &ss, &cc, &lat0, &lon0);
    size_t np=0;
    size_t nextPos=0;
    //bool isDigit=true;
    string tempSubstr;
    tempSubstr=inputImageFileName.substr(0,4);
    if(tempSubstr.find_first_not_of( "0123456789" ) == string::npos){
        YYYY=stoi(tempSubstr);
        tempSubstr=inputImageFileName.substr(5,2);
        if(tempSubstr.find_first_not_of( "0123456789" ) == string::npos){
            MM=stoi(tempSubstr);
            tempSubstr=inputImageFileName.substr(8,2);
            if(tempSubstr.find_first_not_of( "0123456789" ) == string::npos){
                DD=stoi(tempSubstr);
                tempSubstr=inputImageFileName.substr(11,2);
                if(tempSubstr.find_first_not_of( "0123456789" ) == string::npos){
                    hh=stoi(tempSubstr);
                    tempSubstr=inputImageFileName.substr(14,2);
                    if(tempSubstr.find_first_not_of( "0123456789" ) == string::npos){
                        mm=stoi(tempSubstr);
                        tempSubstr=inputImageFileName.substr(17,2);
                        if(tempSubstr.find_first_not_of( "0123456789" ) == string::npos){
                            ss=stoi(tempSubstr);
                            tempSubstr=inputImageFileName.substr(20,inputImageFileName.find_first_of("_")-20);
                            if(tempSubstr.find_first_not_of( "0123456789" ) == string::npos){
                                cc=stoi(tempSubstr,&np);
if(MM<1 || MM >12 || DD<1 || DD >31 || hh <0 || hh>=24 || mm <0 || mm>=60 || ss<0 || ss>=60 || cc<0){
    if (logFile != NULL) {
        fprintf(logFile, "Incorrect date format in the name of the file: %s",inputImageFileName.c_str());
    }
    return -1;
}
int degLat=0, minLat=0,degLon=0, minLon=0;
double secLat=0.0, secLon=0.0;
nextPos=np+21;
tempSubstr=inputImageFileName.substr(nextPos,2); //degrees
if(tempSubstr.find_first_not_of( "0123456789" ) == string::npos){
    degLat=stoi(tempSubstr);
    tempSubstr=inputImageFileName.substr(nextPos+3,2);
    if(tempSubstr.find_first_not_of( "0123456789" ) == string::npos){
        minLat=stoi(tempSubstr);
        int inderSym2=(inputImageFileName.substr(nextPos+6)).find_first_of("_");
        tempSubstr=inputImageFileName.substr(nextPos+6,inderSym2);
        if(tempSubstr.find_first_not_of( "0123456789." ) == string::npos){
            secLat=stod(tempSubstr,&np);
            nextPos+=np+7;
            tempSubstr=inputImageFileName.substr(nextPos,2);
            if(tempSubstr.find_first_not_of( "0123456789." ) == string::npos){
                degLon=stoi(tempSubstr);
                tempSubstr=inputImageFileName.substr(nextPos+3,2);
                if(tempSubstr.find_first_not_of( "0123456789." ) == string::npos){
                    minLon=stoi(tempSubstr);
                    tempSubstr=inputImageFileName.substr(nextPos+6,inputImageFileName.substr(nextPos+6).size()-4);
                    if(tempSubstr.find_first_not_of( "0123456789." ) == string::npos){
                        secLon=stod(tempSubstr);
                        if(full2Degree(degLat,minLat,secLat,'a',lat0) !=0 || full2Degree(degLon,minLon,secLon,'o',lon0) !=0){
                            if (logFile != NULL) {
                                fprintf(logFile, "Incorrect format of geographic coordinates in the name of the file: %s",inputImageFileName.c_str());
                            }
                             return -1;
                        }
                        else{
                            //refresh center of cortesian coordinates
                            LocGeoc.Reset(lat0,lon0,0); // set the center of local cartesian coordinates
                        }
                    }
                }
            }
        }
    }
}
                            }
                        }
                    }
                }
            }
        }
    }

	BMP inputImage;
	inputImage.ReadFromFile(inputImageFileName.c_str());
	if (createOutputImage) //Если нужно выходное изображение, создаём его, иначе просто обрабатываем данные
		nextStep(inputImage, createOutputImage).WriteToFile(outputImageFileName.c_str());
	else
		nextStep(inputImage, createOutputImage);
	writeOutput(); //Пишем в файл вывода
	return 0;
}
//Если выходное изображение не нужно, то для удобства используется этот вариант
int Radar::run(const list<string> inputFileNames)
{
	return run(inputFileNames, false, "");
}
//Возвращаем текущее изображение мира, метод для отладки
BMP Radar::getImage()
{
	return world;
}

int Radar::degree2Full(double degIn, char type, int &degOut, int &minOut, double &secOut){
    ///type: a - lattitude , o-longitude
    if( (abs(degIn)>90 && type=='a') || ((degIn<0 || degIn>180) && type=='o')){
        return -1;
    }
    else{
        degOut=floor(degIn);
        double temp = (degIn - degOut)*60;
        minOut= floor(temp);
        secOut= (temp-minOut)*60;
        return 0;
    }
}

int Radar::full2Degree(int degIn, int minIn, double secIn, char type, double &degOut){
    ///type: a - langitude , o-longitude
    if( (abs(degIn)>90 && type=='a') || ((degIn<0 || degIn>180) && type=='o') || abs(minIn)>=60 || floor(secIn)>=60){
        return -1;
    }
    else{
        degOut= degIn + (minIn/60) + (secIn/3600);
        return 0;
    }
}
