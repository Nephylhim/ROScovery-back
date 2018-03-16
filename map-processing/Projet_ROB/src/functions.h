#ifndef PROJECT_FUNCTIONS_H
#define PROJECT_FUNCTIONS_H

#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;


Mat redimensionnement(Mat img);
vector<Mat> preTraitement_Thomas();
Mat preTraitement_Wistan(Mat img);

Mat lineDetection(Mat img);
void lineMatching(vector<vector<Vec4i>> lines);
vector<KeyPoint> pointsFinding(Mat img);
Mat descriptorsFinding(Mat map, vector<KeyPoint> keypoints);
vector<vector<float>> segmentsFinding(Mat img);
vector< DMatch > pointsMatching(vector<Mat> maps, vector<Mat> descriptorsTotal, vector<vector<KeyPoint>> keypointsTotal);
vector<vector<float>> segmentsMatching (vector<vector<vector<float>>> segs2Pictures, vector<Mat> maps);

Mat homographyEstimation(vector< DMatch > good_matches, vector<vector<KeyPoint>> keypointsTotal);
Mat rotationEstimation(vector<vector<float>> goodMatches);
void imagesWarping(Mat H, vector<Mat> maps);

void templateMatching(Mat src, Mat img);

Mat drawOrigin(Mat map, float resolution, float X, float Y);



#endif //PROJECT_FUNCTIONS_H
