#include "functions.h"
#include <cmath>

using namespace std;
using namespace cv;


//resize de l'image
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


//utilisation de thresholds
vector<Mat> preTraitement_Thomas(){
	vector<string> mapPaths = {"./Maps/Brest/map_3.pgm", "./Maps/Brest/map_4.pgm"};
	vector<Mat> maps;

	for(unsigned int i=0; i < mapPaths.size(); i++){
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

		dilate(maps[i], maps[i], Mat(), Point(-1, -1), 2);
		GaussianBlur(maps[i], maps[i], Size(3, 3), 3, 3);
		erode(maps[i], maps[i], Mat(), Point(-1, -1), 2);
		threshold(maps[i], maps[i], 70, 255, CV_THRESH_BINARY);
	}

	waitKey(0);

	return maps;
}


//utilisation des contours
Mat preTraitement_Wistan(Mat img)
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

	//imshow ("Test_new", imgOut4);
	//waitKey();


	return imgOut4;
}


//détection auto des lignes de la map
Mat lineDetection(Mat img)
{
	Mat dst, cdst;
	Canny(img, dst, 50, 200, 3);
	cvtColor(dst, cdst, CV_GRAY2BGR);

	vector<Vec4i> lines;
	HoughLinesP(dst, lines, 1, CV_PI/180, 50, 50, 10 );
	for( size_t i = 0; i < lines.size(); i++ )
	{
		if (lines[i][3] < lines[i][1])
		{
			int lx = lines[i][0], ly = lines[i][1];
			lines[i][0] = lines[i][2], lines[i][1] = lines[i][3];
			lines[i][2] = lx, lines[i][3] = ly;
		}

		Vec4i l = lines[i];
		line( img, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 4, CV_AA);
	}

	return img;
}


//matching des lignes (non-fonctionnel)
void lineMatching(vector<vector<Vec4i>> lines)
{
	//pour chaque vector : l[0] & l[1] = x & y point1, l[2] & l[3] = x & y point1

	for (unsigned int i = 0; i < lines[0].size(); i++)
	{
		for (unsigned int j = 0; j < lines[1].size(); j++)
		{
			vector<double> angles;
			phase(lines[0][i], lines[1][j], angles, true);
		}
	}
}


//determine les points d'interet de chaque image
vector<KeyPoint> pointsFinding(Mat img)
{
	// on peut passer en parametres : FAST, STAR, SIFT, SURF, ORB, BRISK, MSER, GFTT, HARRIS, Dense, SimpleBlob
	vector<KeyPoint> keypoints;


	//	Methode manuelle (HARRIS corner detection)

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


	imshow("Lignes Manu", img);
	waitKey();

	return keypoints;
}


//calcule les descripteurs des points
Mat descriptorsFinding(Mat map, vector<KeyPoint> keypoints){
	Mat descriptors;
	Ptr<FeatureDetector> detector = ORB::create();

	detector->compute(map, keypoints, descriptors);

	return descriptors;
}


//determine les segments d'interet de chaque image
vector<vector<float>> segmentsFinding(Mat img)
{
	// on peut passer en parametres : FAST, STAR, SIFT, SURF, ORB, BRISK, MSER, GFTT, HARRIS, Dense, SimpleBlob
	vector<KeyPoint> keypoints;


	//	Methode manuelle (HARRIS corner detection)

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

	vector<vector <float>> segments;

	for (unsigned int i = 0; i < centre.size(); i++)
	{
		for (unsigned int j = i+1; j < centre.size(); j++)
		{
			double distance = sqrt(pow((double)(centre[i].x-centre[j].x), 2) + pow((double)(centre[i].y-centre[j].y), 2));

			if (distance > 0)
			{
				int nbBlancs = 0;

				LineIterator it(img, centre[i], centre[j], 8);
				for(int i = 0; i < it.count; i++, ++it)
				{
				    Point pt= it.pos();
				    if (img.at<unsigned char>(pt) == 255) nbBlancs ++;
				}
				if ((double)nbBlancs/(double)it.count > 0.8)
				{
					line(img, centre[i], centre[j], 125, 3, 8, 0);
					vector<float> unSegment = {centre[i].x, centre[i].y, centre[j].x, centre[j].y, (float)distance};
					segments.push_back(unSegment);
				}
			}
		}
	}

	imshow("Lignes Manu", img);
	waitKey();

	return segments;
}


//trouve des points d'interet communs entre les images
vector< DMatch > pointsMatching(vector<Mat> maps, vector<Mat> descriptorsTotal, vector<vector<KeyPoint>> keypointsTotal)
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

	Mat imgout;
	drawMatches(maps[1], keypointsTotal[1], maps[0], keypointsTotal[0], good_matches, imgout);

	imshow("Bilan", imgout);

	return good_matches;
}


//trouve des segments communs entre les images
vector<vector<float>> segmentsMatching (vector<vector<vector<float>>> segs2Pictures, vector<Mat> maps)
{
	vector<vector<float>> segsMatches;
	float epsilon = 4;

	for (unsigned int i = 0; i < segs2Pictures[0].size(); i++)
	{
		for (unsigned int j = 0; j < segs2Pictures[1].size(); j++)
		{
			if (abs(segs2Pictures[0][i][4] - segs2Pictures[1][j][4]) < epsilon)
			{
				segsMatches.push_back(segs2Pictures[0][i]), segsMatches.push_back(segs2Pictures[1][j]);
				segs2Pictures[0][i][4] = 0, segs2Pictures[1][j][4] = 0;
			}
		}
	}

	return segsMatches;
}


//estimation de l'homographie
Mat homographyEstimation(vector< DMatch > good_matches, vector<vector<KeyPoint>> keypointsTotal)
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


//estimation de la rotation entre deux cartes (non-fonctionnel)
Mat rotationEstimation(vector<vector<float>> goodMatches)
{
	Point2f ptA, ptB, ptC;

	for(int i = 0; i < (int)goodMatches.size(); i+=2)
	{
		ptA = Point2f(0, 0);
		ptB = Point2f(goodMatches[i][2]-goodMatches[i][0], goodMatches[i][3]-goodMatches[i][1]);
		ptC = Point2f(goodMatches[i+1][2]-goodMatches[i+1][0], goodMatches[i+1][3]-goodMatches[i+1][1]);

		cout << ptA << " " << ptB << " " << ptC << endl;

		double angle = atan2(ptB.y - ptA.y, ptB.x - ptA.x) - atan2(ptC.y - ptA.y, ptC.x - ptA.x)*180/M_PI;

		cout << angle << endl;
	}

	Mat H;
	return H;
}


//fusion des cartes
void imagesWarping(Mat H, vector<Mat> maps)
{
	// transformation de l'image suivant l'homographie trouvÃ©e
	Mat result;

	warpPerspective(maps[0],result,H,Size(maps[0].cols+maps[1].cols,maps[0].rows));

	Mat half(result,cv::Rect(0,0,maps[1].cols,maps[1].rows));

	maps[1].copyTo(half);
	imshow( "Result", result );
}


//methode du "template matching" (non-fonctionnel)
void templateMatching(Mat src, Mat img)
{
	Mat result;

	matchTemplate(src, img, result, CV_TM_CCOEFF_NORMED);

	imshow("Template", result);
}


//affichage de l'origine du repère sur la carte
Mat drawOrigin(Mat map, float resolution, float X, float Y)
{
	int XPixels = abs(X)/resolution, YPixels = abs(Y)/resolution;

	circle(map, Point(XPixels, map.rows-YPixels), 3, 125, 3, 8, 0);

	return map;
}


