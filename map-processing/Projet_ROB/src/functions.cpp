#include "functions.h"

using namespace std;
using namespace cv;


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

Mat mapTobinary(Mat img){
	Mat imgOut = Mat::zeros(img.rows, img.cols, CV_8UC1);

	for (int j = 0; j < img.cols; j++)
	{
		for (int i = 0; i < img.rows; i++)
		{
			if ((int)img.at<unsigned char>(i, j) == 0) imgOut.at<unsigned char>(i, j) = 0;
			else imgOut.at<unsigned char>(i, j) = 255;
		}
	}

	return imgOut;
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
		}
	}

	drawContours(imgOut2, contoursFinal, -1, 255, CV_FILLED, 8);

	Mat imgOut3 = Mat::zeros(img.rows, img.cols, CV_8UC1);
	Mat imgOut4 = Mat::zeros(img.rows, img.cols, CV_8UC1);

	dilate(imgOut2, imgOut3, Mat(), Point(-1, -1), 3);
	erode(imgOut3, imgOut4, Mat(), Point(-1, -1), 2);

	imshow ("Test_new", imgOut4);
	waitKey();


	return imgOut4;
}


//determine les points d'interet de chaque image
vector<KeyPoint> featuresFinding(Mat img)
{
	// on peut passer en parametres : FAST, STAR, SIFT, SURF, ORB, BRISK, MSER, GFTT, HARRIS, Dense, SimpleBlob
	vector<KeyPoint> keypoints;


	//	Méthode manuelle (HARRIS corner detection)

	Mat imgOut = Mat::zeros(img.rows, img.cols, CV_8UC1);
	cornerHarris(img, imgOut, 6, 5, 0.05, BORDER_DEFAULT);

	imgOut.convertTo(imgOut,CV_8UC1, 255.0);

	vector<vector<Point> > contours;
	findContours(imgOut, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	//Get the moments
	vector<Moments> mu(contours.size() );
	for(unsigned int i = 0; i < contours.size(); i++ ) mu[i] = moments( contours[i], false );

	//Get the mass centers
	vector<Point2f> centre;
	for(unsigned int i = 0; i < contours.size(); i++ ) centre.push_back(Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00));

	for(unsigned int i = 0; i < centre.size(); i++) {
		keypoints.push_back(KeyPoint(centre[i], 4, -1, 0, 0, -1));
	}

	drawKeypoints(img, keypoints, img, 125);

//cout << keypoints.size() << endl;

	imshow("Manu", img);
	waitKey();

	return keypoints;
}

Mat descriptorsFinding(Mat map, vector<KeyPoint> keypoints){
	Mat descriptors;
	Ptr<FeatureDetector> detector = ORB::create();

	detector->compute(map, keypoints, descriptors);

	return descriptors;
}


//trouve des points d'interet communs entre les images
vector< DMatch > featuresMatching(vector<Mat> maps, vector<Mat> descriptorsTotal, vector<vector<KeyPoint>> keypointsTotal)
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

/*
	sort(matches.begin(), matches.end());

	//Draw first 5 matches
	vector< DMatch > good_matches;
	for (int i = 0; i < 5; i++)
	{
		good_matches.push_back(matches[i]);
	}

	Mat imgout;
	drawMatches(maps[1], keypointsTotal[1], maps[0], keypointsTotal[0], good_matches, imgout);
*/

	Mat imgout;
	drawMatches(maps[1], keypointsTotal[1], maps[0], keypointsTotal[0], good_matches, imgout);

	imshow("Bilan", imgout);

	return good_matches;
}


//estimation de l'homographie
Mat rotationEstimation(vector< DMatch > good_matches, vector<vector<KeyPoint>> keypointsTotal)
{
	// Calcul de l'homographie entre les 2 images
	vector<Point2f> pt_img1;
	vector<Point2f> pt_img2;

	for( int i = 0; i < (int)good_matches.size(); i++ ) {
		pt_img1.push_back(keypointsTotal[0][ good_matches[i].queryIdx ].pt );
		pt_img2.push_back(keypointsTotal[1][ good_matches[i].trainIdx ].pt );
	}
	Mat H = findHomography( pt_img1, pt_img2, CV_RANSAC );

	cout << pt_img1.size() << " " << pt_img2.size() << endl;
	cout << H.size() << endl;

	return H;
}


//fusion des cartes
void imagesWarping(Mat H, vector<Mat> maps)
{
	// transformation de l'image suivant l'homographie trouvée
	Mat result;

	warpPerspective(maps[0],result,H,Size(maps[0].cols+maps[1].cols,maps[0].rows));

	Mat half(result,cv::Rect(0,0,maps[1].cols,maps[1].rows));

	maps[1].copyTo(half);
	imshow( "Result", result );
}
