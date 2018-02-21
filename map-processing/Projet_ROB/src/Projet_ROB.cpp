#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d.hpp"

using namespace std;
using namespace cv;



// ID : redessiner la forme (long mais précis)

// ligne par ligne, par détection des points d'angle (90° normalement)




//Fonctions necessaires :
	//redimensionnement		OK
	//pré-traitement
	//find features
	//match features
	//rotation estimation	OK
	//image warping			OK

	// + sauvegarde image


//donne la taille et le type d'une matrice si erreur de lancement a cause des images
string type2str(int type) {
  string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;

  //a placer dans le programme a tester
	//string tx =  type2str( img.type() );
	//printf("Matrix: %s %dx%d \n", tx.c_str(), img.cols, img.rows );
}



vector<vector<KeyPoint>> keypointsTotal;
vector<Mat> descriptorsTotal;
vector<Mat> maps;


//resize de l'image (x2)
Mat redimensionnement(Mat img)
{
	//pixels blancs : 254
	//pixels gris : 205
	//pixels noirs : 0

	int minH = 10000, maxH = 0, minL = 10000, maxL = 0;

	for (int j = 0; j < img.cols; j++)
	{
		for (int i = 0; i < img.rows; i++)
		{
			if ((int)img.at<unsigned char>(i, j) != 205)
			{
				if (i < minH) minH = i;
				if (i > maxH) maxH = i;
				if (j < minL) minL = j;
				if (j > maxL) maxL = j;
			}
		}
	}

	Rect myROI(minL-5, minH-5, maxL-minL+10, maxH-minH+10);
	Mat croppedImage = img(myROI);

	return croppedImage;
}


//elimine les bruits de l'image
Mat preTraitement(Mat img)
{
	Mat imgOut = Mat::zeros(img.rows, img.cols, CV_8UC1);
	Mat imgOut2 = Mat::zeros(img.rows, img.cols, CV_8UC1);

	for (int j = 0; j < img.cols; j++)
	{
		for (int i = 0; i < img.rows; i++)
		{
			if ((int)img.at<unsigned char>(i, j) == 0) imgOut.at<unsigned char>(i, j) = 255;
			else imgOut.at<unsigned char>(i, j) = 0;
		}
	}

	vector<vector<Point> > contours, contoursFinal;
	findContours(imgOut, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	for (unsigned int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > 10)
		{
			contoursFinal.push_back(contours[i]);
/*
			for (unsigned int j = 0; j < contours[i].size(); j++)
			{

			}
*/
		}
	}

	drawContours(imgOut2, contoursFinal, -1, 255, CV_FILLED, 8);

	return imgOut2;
}


//determine les points d'interet de chaque image
void featuresFinding(Mat img)
{
	// on peut passer en parametres : FAST, STAR, SIFT, SURF, ORB, BRISK, MSER, GFTT, HARRIS, Dense, SimpleBlob
	Ptr<FeatureDetector> detector = ORB::create();
	vector<KeyPoint> keypoints;
	Mat descriptors;

	Mat imgout = Mat::zeros(img.rows, img.cols, CV_8UC1);

	detector->detect(img, keypoints);
	drawKeypoints(img, keypoints, imgout, 125);

	detector->compute(img, keypoints, descriptors);

	keypointsTotal.push_back(keypoints);
	descriptorsTotal.push_back(descriptors);

//imshow("Test", imgout);
//waitKey();
}


//trouve des points d'interet communs entre les images
vector< DMatch > featuresMatching(vector<Mat> maps)
{
	// Matching descriptors
	BFMatcher matcher;
	vector< DMatch > matches;

	matcher.match( descriptorsTotal[0], descriptorsTotal[1], matches );

cout << "Matches size : " << matches.size() << endl;

	double max_dist = 0; double min_dist = 100;

	// Quick calculation of max and min distances between keypoints
	for( int i = 0; i < descriptorsTotal[0].rows; i++ )
	{
		double dist = matches[i].distance;
		if( dist < min_dist ) min_dist = dist;
		if( dist > max_dist ) max_dist = dist;
	}

	// Draw only "good" matches (i.e. whose distance is less than min_dist )
	vector< DMatch > good_matches;
	for( int i = 0; i < descriptorsTotal[0].rows; i++ )
		if( matches[i].distance <= 2*min_dist )
			good_matches.push_back( matches[i]);

cout << "Good Matches size : " << good_matches.size() << endl;
//cout << keypointsTotal[0].size() << endl << keypointsTotal[1].size() << endl;

	Mat imgout;
	drawMatches(maps[1], keypointsTotal[1], maps[0], keypointsTotal[0], good_matches, imgout);

	imshow("Bilan", imgout);

	return good_matches;
}


//estimation de l'homographie
Mat rotationEstimation(vector< DMatch > good_matches)
{
	// Calcul de l'homographie entre les 2 images
	vector<Point2f> pt_img1;
	vector<Point2f> pt_img2;

	for( int i = 0; i < (int)good_matches.size(); i++ ) {
		pt_img1.push_back(keypointsTotal[0][ good_matches[i].queryIdx ].pt );
		pt_img2.push_back(keypointsTotal[1][ good_matches[i].trainIdx ].pt );
	}
	Mat H = findHomography( pt_img1, pt_img2, CV_RANSAC );

	return H;
}


//fusion des cartes
void imagesWarping(Mat H)
{
	// transformation de l'image suivant l'homographie trouvée
	Mat result;

	warpPerspective(maps[0],result,H,Size(maps[0].cols+maps[1].cols,maps[0].rows));

	Mat half(result,cv::Rect(0,0,maps[1].cols,maps[1].rows));

	maps[1].copyTo(half);
	imshow( "Result", result );
}




/* Methode finale necessaire:
** 1- ouverture de deux images
** 2- elimination des pixels de bruit (+ dessin propre des contours ?)
** 3- determination des points d'interet des deux images
** 4- matching des points d'interet communs
** 5- estimation de la rotation (homographie)
** 6- fusion des deux cartes
**
** BONUS : rectification de la distortion de certaines images (capteurs mal calibres)
*/

int main(int argc, char **argv)
{
	for (int i = 1; i < 3; i++)
	{
		char img[100];
		string windowName = "Essai_";
		windowName += i + '0';

		sprintf(img,"./Maps/Brest/map_%d.pgm", i);

		Mat map = imread(img, CV_LOAD_IMAGE_GRAYSCALE);

		imshow(windowName, map);

		//windowName += i + '0';

		map = redimensionnement(map);

		map = preTraitement(map);

		imshow(windowName, map);

		maps.push_back(map);

		featuresFinding(map);
	}

	vector< DMatch > good_matches = featuresMatching(maps);

	if (good_matches.size() > 100)
	{
		Mat H = rotationEstimation(good_matches);

		imagesWarping(H);
	}

	waitKey(0);
	return 0;
}
