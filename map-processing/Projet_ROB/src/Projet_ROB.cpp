//#include <opencv2/opencv.hpp>
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/features2d.hpp"
//#include "opencv2/imgproc/imgproc.hpp"

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
	return 0;
}
