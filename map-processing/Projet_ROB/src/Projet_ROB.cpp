//#include <opencv2/opencv.hpp>
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/features2d.hpp"
//#include "opencv2/imgproc/imgproc.hpp"

#include "Projet_ROB.h"

using namespace std;
using namespace cv;


//Fonctions necessaires :
	//redimensionnement		OK
	//pr�-traitement
	//find features
	//match features
	//rotation estimation	OK
	//image warping			OK

	// + sauvegarde image



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

void test1(){
	vector<vector<KeyPoint>> keypointsTotal;
	vector<KeyPoint> newKeypoints;
	vector<Mat> descriptorsTotal;
	Mat descriptors;
	vector<Mat> maps;

	for (int i = 1; i < 3; i++)
	{
		char img[100];
		string windowName = "Essai_";
		windowName += i + '0';

		sprintf(img,"./Maps/Brest/map_%d.pgm", i+2);

		Mat map = imread(img, CV_LOAD_IMAGE_GRAYSCALE);

		imshow(windowName, map);

		//windowName += i + '0';

		map = redimensionnement(map);

		map = preTraitement(map);

		//imshow(windowName, map);

		maps.push_back(map);

		newKeypoints = featuresFinding(map);
		keypointsTotal.push_back(newKeypoints);

		descriptors = descriptorsFinding(map, newKeypoints);
		descriptorsTotal.push_back(descriptors);

	}

	vector< DMatch > good_matches = featuresMatching(maps, descriptorsTotal, keypointsTotal);

//	if (good_matches.size() > 50)
//	{
	Mat H = rotationEstimation(good_matches, keypointsTotal);

	//string tx =  type2str( H.type() );
	//printf("Matrix: %s %dx%d \n", tx.c_str(), H.cols, H.rows );

	//imagesWarping(H);
//	}

	waitKey(0);
}


void test2(){
	vector<string> mapPaths = {"./Maps/Brest/map_3.pgm", "./Maps/Brest/map_4.pgm"};
	vector<Mat> maps;

	for(int i=0; i < mapPaths.size(); i++){
		maps.push_back(imread(mapPaths[i], CV_LOAD_IMAGE_GRAYSCALE));

		maps[i] = redimensionnement(maps[i]);
		// maps[i] = mapTobinary(maps[i]);
		threshold(maps[i], maps[i], 70, 255, CV_THRESH_BINARY_INV);
		imshow("i "+to_string(i), maps[i]);

		//--------------------------------------------------//
		morphologyEx(maps[i], maps[i], MORPH_CLOSE, Mat::ones(3, 3, CV_8UC1), Point(-1, -1), 2);

		dilate(maps[i], maps[i], Mat(), Point(-1, -1), 1.5);
		GaussianBlur(maps[i], maps[i], Size(3, 3), 3, 3);
		erode(maps[i], maps[i], Mat(), Point(-1, -1), 2);

		threshold(maps[i], maps[i], 70, 255, CV_THRESH_BINARY);

		dilate(maps[i], maps[i], Mat(), Point(-1, -1), 2.0);
		GaussianBlur(maps[i], maps[i], Size(3, 3), 3, 3);
		erode(maps[i], maps[i], Mat(), Point(-1, -1), 2);
		threshold(maps[i], maps[i], 70, 255, CV_THRESH_BINARY);

		imshow("it "+to_string(i), maps[i]);
	}

	
	waitKey(0);
}

int main(int argc, char **argv)
{
//	test1();
	test2();

	return 0;
}
