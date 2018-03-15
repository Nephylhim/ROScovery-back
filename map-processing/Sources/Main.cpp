/**
 * @Author: Thibault Napoléon <Imothep>
 * @Company: ISEN Yncréa Ouest
 * @Email: thibault.napoleon@isen-ouest.yncrea.fr
 * @Created Date: 06-Nov-2017 - 10:49:44
 * @Last Modified: 28-Feb-2018 - 19:57:02
 */

// Includes.
#include "Main.hpp"

//------------------------------------------------------------------------------
//--- Main function ------------------------------------------------------------
//------------------------------------------------------------------------------
int main(int argc, char** argv)
{
  Mat image1Lines;
  Mat image2Lines;
  Mat image1 = imread("map_3.pgm");
  Mat image2 = imread("map_2.pgm");

  image1.copyTo(image1Lines);
  image2.copyTo(image2Lines);
  cvtColor(image1, image1, CV_BGR2GRAY);
  cvtColor(image2, image2, CV_BGR2GRAY);
  threshold(image1, image1, 50, 255, THRESH_BINARY);
  threshold(image2, image2, 50, 255, THRESH_BINARY);
  image1 = 255 - image1;
  image2 = 255 - image2;

  vector<Vec4i> lines;
  HoughLinesP(image1, lines, 1, CV_PI/180, 50, 50, 10 );
  for( size_t i = 0; i < lines.size(); i++ )
  {
    Vec4i l = lines[i];
    line(image1Lines, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
  }
  lines.clear();
  HoughLinesP(image2, lines, 1, CV_PI/180, 50, 50, 10 );
  for( size_t i = 0; i < lines.size(); i++ )
  {
    Vec4i l = lines[i];
    line(image2Lines, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
  }

  imshow("Image 1", image1);
  imshow("Image 1 Lines", image1Lines);
  imshow("Image 2", image2);
  imshow("Image 2 Lines", image2Lines);
  waitKey();

	return EXIT_SUCCESS;
}
