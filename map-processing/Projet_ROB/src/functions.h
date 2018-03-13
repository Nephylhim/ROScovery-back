#ifndef PROJECT_FUNCTIONS_H
#define PROJECT_FUNCTIONS_H

#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

string type2str(int type);
Mat redimensionnement(Mat img);
Mat preTraitement(Mat img);
vector<KeyPoint> featuresFinding(Mat img);
Mat descriptorsFinding(Mat map, vector<KeyPoint> keypoints);
vector< DMatch > featuresMatching(vector<Mat> maps, vector<Mat> descriptorsTotal, vector<vector<KeyPoint>> keypointsTotal);
Mat rotationEstimation(vector< DMatch > good_matches, vector<vector<KeyPoint>> keypointsTotal);
void imagesWarping(Mat H, vector<Mat> maps);

#endif //PROJECT_FUNCTIONS_H
