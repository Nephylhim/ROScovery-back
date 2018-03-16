//#include <opencv2/opencv.hpp>
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/features2d.hpp"
//#include "opencv2/imgproc/imgproc.hpp"

#include <fstream>
#include "Projet_ROB.h"

using namespace std;
using namespace cv;


void demoUnRobot(char **argv)
{
	//string mapPath = "./maps/raw/WALL-E.pgm";
	//string yamlPath = "./pos/local/WALL-E.yaml";

	string mapPath = argv[1];
	string yamlPath = argv[2];

	Mat map = {imread(mapPath, CV_LOAD_IMAGE_GRAYSCALE)};

	vector<vector<Vec4i>> linesTotal;

	map = preTraitement_Wistan(map);

	fstream yaml;
	yaml.open(yamlPath.c_str());

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

	map = drawOrigin(map, resolution, X, Y);

	//resize(maps[i], maps[i], Size(), 0.5, 0.5, INTER_LINEAR);

	//map = lineDetection(map);

	//imshow("Carte Traitée", map);
	//waitKey();

	//imwrite( "./Maps/grandesMaps/my_map3.png", map);
	imwrite( argv[3], map);
}



/* Methode finale necessaire:
 *
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
	demoUnRobot(argv);

	return 0;
}
