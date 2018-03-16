//#include <opencv2/opencv.hpp>
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/features2d.hpp"
//#include "opencv2/imgproc/imgproc.hpp"

#include <fstream>
#include "Projet_ROB.h"

using namespace std;
using namespace cv;


//Fonctions necessaires :
	//redimensionnement		OK
	//pré-traitement
	//find features
	//match features
	//rotation estimation	OK
	//image warping			OK

	// + sauvegarde image


vector<Mat> test2(){
	vector<string> mapPaths = {"./Maps/Brest/map_3.pgm", "./Maps/Brest/map_4.pgm"};
	vector<Mat> maps;

	for(unsigned int i=0; i < mapPaths.size(); i++){
		maps.push_back(imread(mapPaths[i], CV_LOAD_IMAGE_GRAYSCALE));

		maps[i] = redimensionnement(maps[i]);
		// maps[i] = mapTobinary(maps[i]);
		threshold(maps[i], maps[i], 70, 255, CV_THRESH_BINARY_INV);
//imshow("i "+to_string(i), maps[i]);

		//--------------------------------------------------//
		morphologyEx(maps[i], maps[i], MORPH_CLOSE, Mat::ones(3, 3, CV_8UC1), Point(-1, -1), 2);

		dilate(maps[i], maps[i], Mat(), Point(-1, -1), 1.5);
		GaussianBlur(maps[i], maps[i], Size(3, 3), 3, 3);
		erode(maps[i], maps[i], Mat(), Point(-1, -1), 2);

		threshold(maps[i], maps[i], 70, 255, CV_THRESH_BINARY);

		dilate(maps[i], maps[i], Mat(), Point(-1, -1), 2);
		GaussianBlur(maps[i], maps[i], Size(3, 3), 3, 3);
		erode(maps[i], maps[i], Mat(), Point(-1, -1), 2);
		threshold(maps[i], maps[i], 70, 255, CV_THRESH_BINARY);
	}

	waitKey(0);

	return maps;
}


Mat drawOrigin(Mat map, float resolution, float X, float Y)
{
	int XPixels = abs(X)/resolution, YPixels = abs(Y)/resolution;

	circle(map, Point(XPixels, map.rows-YPixels), 3, 125, 3, 8, 0);

	return map;
}


void testOrigin()
{
	vector<string> mapPaths = {"./Maps/grandesMaps/my_map3.pgm", "./Maps/grandesMaps/my_map4.pgm"};
	vector<string> yamlPaths = {"./Maps/grandesMaps/my_map3.yaml", "./Maps/grandesMaps/my_map4.yaml"};
	vector<Mat> maps;

	vector<vector<Vec4i>> linesTotal;

	for(unsigned int i=0; i < mapPaths.size(); i++){
		maps.push_back(imread(mapPaths[i], CV_LOAD_IMAGE_GRAYSCALE));

		maps[i] = preTraitement(maps[i]);

		fstream yaml;
		yaml.open(yamlPaths[i].c_str());

		string lineResolution, lineOrigin;

		while (lineOrigin == "")
		{
			string line;
			getline(yaml, line);

			if (line.find("origin:") == 0) lineOrigin = line;
			if (line.find("resolution:") == 0) lineResolution = line;
		}

		yaml.close();

		int posResolution = lineResolution.find(" ");
		float resolution = stof(lineResolution.substr(posResolution, lineResolution.size()-posResolution));

		int posX = lineOrigin.find("["), posY = lineOrigin.find(","), posZ = lineOrigin.find(", 0");
		float X = stof(lineOrigin.substr(posX+1, posY-posX+1));
		float Y = stof(lineOrigin.substr(posY+2, posZ-posY+2));

		maps[i] = drawOrigin(maps[i], resolution, X, Y);

		imshow("i "+to_string(i), maps[i]);

		linesTotal.push_back(lineDetection(maps[i]));
	}
}



/* Methode finale necessaire:
** 1- ouverture de deux images
** 2- elimination des pixels de bruit
** 3- determination des points d'interet des deux images
** 4- matching des points d'interet communs
** 5- estimation de la rotation (homographie)
** 6- fusion des deux cartes
**
** BONUS : rectification de la distortion de certaines images (capteurs mal calibres)
*/

int main(int argc, char **argv)
{
	vector<vector<KeyPoint>> keypointsTotal;
	vector<KeyPoint> newKeypoints;
	vector<vector<vector<float>>> segmentsTotal;
	vector<vector<float>> segments;
	vector<vector<float>> goodMatches;
	vector<Mat> descriptorsTotal;
	Mat descriptors;
	vector<Mat> maps;

	vector<vector<Vec4i>> linesTotal;
/*
	maps = test2();

	for (unsigned int i = 0; i < 2; i++)
	{
		char img[100];
		string windowName = "Essai_";
		windowName += i + '0';

		sprintf(img,"./Maps/Brest/map_%d.pgm", i+3);

		Mat map = imread(img, CV_LOAD_IMAGE_GRAYSCALE);

		map = redimensionnement(map);

		map = preTraitement(map);

		linesTotal.push_back(lineDetection(map));

//maps[i] = lineDetection(maps[i]);

		newKeypoints = featuresFinding(maps[i]);
		keypointsTotal.push_back(newKeypoints);

		descriptors = descriptorsFinding(maps[i], newKeypoints);
		descriptorsTotal.push_back(descriptors);

		segments = featuresFinding(maps[i]);
		segmentsTotal.push_back(segments);

	}

	//vector< DMatch > good_matches = featuresMatching(maps, descriptorsTotal, keypointsTotal);
	//goodMatches = segmentsMatching(segmentsTotal, maps);

	//Mat H = rotationEstimation(good_matches, keypointsTotal);
	//Mat H = rotationEstimation(goodMatches);

	//cout << H << endl;

	//string tx =  type2str( H.type() );
	//printf("Matrix: %s %dx%d \n", tx.c_str(), H.cols, H.rows );

	//imagesWarping(H, maps);
*/
	testOrigin();

	waitKey(0);

	return 0;
}
