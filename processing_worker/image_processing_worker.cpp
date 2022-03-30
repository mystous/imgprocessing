#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <string>
using namespace std;
using namespace cv;

const string PATCHIFIED = "2";
const string CALCULATING = "3";
const string CALCULATED = "4";
const string PROCESSING_FAIL = "5";

const int patchWidth = 256;
const int patchHeight = 256;

void insertPatchFileInformation(string imageId, int maxX, int maxY){
  int i, j;
  string queryString = "insert into lunit.patchImageMeta (`imageId`, `xIndex`, `yIndex`, `patchImageLocation`, `saliencyStatus`, `histogramStatus`) values ";

  for (i = 0; i < maxX; ++i) {
    for (j = 0; j < maxY; ++j) {
      queryString += "('";
      queryString += imageId;
      queryString += "', '";
      queryString += to_string(i + 1);
      queryString += "', '";
      queryString += to_string(j + 1);
      queryString += "', '";
      queryString += "http://121.160.75.246:8080/files/patches/";
      queryString += imageId;
      queryString += "_";
      queryString += to_string(i + 1);
      queryString += "_";
      queryString += to_string(j + 1);
      queryString += ".jpg";
      queryString += "', '3', '3')";
      if( i != maxX - 1 || j != maxY -1 ){
        queryString += " , ";
      }
    }
  }
  queryString += ";";
  cout << queryString << endl;
}
void updateImageStatus(string imageId, string imgStatus, int maxX = -1, int maxY = -1){
  string queryString = "update lunit.imageMeta set `status` = '";

  queryString += imgStatus;
  queryString += "', `maxXIndex` = '";
  queryString += to_string(maxX);
  queryString += "', `maxYIndex` = '";
  queryString += to_string(maxY);
  queryString += "' where (`imageId` = '";
  queryString += imageId;
  queryString += "');";
  cout << queryString << endl;
}

void calculatePatchNumbersAndCloutSize(Size imageSize, int &maxX, int &maxY, int &cloutWidth, int &cloutHeight){
  cloutWidth = imageSize.width % patchWidth;
  cloutHeight = imageSize.height % patchHeight;

  maxX = imageSize.width / patchWidth + ( 0 == cloutWidth ? 0 : 1) ;
  maxY = imageSize.height / patchHeight + ( 0 == cloutHeight ? 0 : 1) ;

  cloutWidth = ( 0 == cloutWidth ) ? patchWidth : cloutWidth;
  cloutHeight = ( 0 == cloutHeight ) ? patchHeight : cloutHeight;
}

string getPatchName(string imageId, int x, int y){
  string patchFileName = "./patch_data/";
  patchFileName += imageId;
  patchFileName += "_";
  patchFileName += to_string(x);
  patchFileName += "_";
  patchFileName += to_string(y);
  patchFileName += ".jpg";
  return patchFileName;
}

void patchingImage(string imageId, string fileName, int &maxX, int &maxY){
  int i, j, cropWidth, cropHeight;
  int cloutWidth, cloutHeight;

  Mat originalImage = imread(fileName);
  if( nullptr == originalImage.data )
  {
    cout << "Oops" << endl;
    updateImageStatus(imageId, PROCESSING_FAIL);
    return;
  }

  Size imageSize = originalImage.size();
  calculatePatchNumbersAndCloutSize(imageSize, maxX, maxY, cloutWidth, cloutHeight);
  
  string patchFileName = imageId;
  for( i = 0 ; i < maxY ; ++i ){
    for( j = 0 ; j < maxX ; ++j ){
      cropWidth = ( j + 1 == maxX ) ? cloutWidth : patchWidth;
      cropHeight = ( i + 1 == maxY ) ? cloutHeight : patchHeight;
      
      Mat patchImage = Mat(Size(patchWidth, patchHeight), CV_8UC3, Scalar(255, 255, 255));
      Mat sourceImage = originalImage(Rect(patchWidth * j, patchHeight * i, cropWidth, cropHeight));
      Mat roi = patchImage(Rect(0, 0, cropWidth, cropHeight));
      sourceImage.copyTo(roi);
      imwrite(getPatchName(imageId, i + 1, j + 1), patchImage);
    }
  }

  cout << "Pathcing task of '" << imageId << "'is done." << endl;
  insertPatchFileInformation(imageId, maxX, maxY);
  updateImageStatus(imageId, PATCHIFIED, maxX, maxY);
}

int main(int argc, char** argv)
{
  string  imageId, fileName; 
  int maxX = 0, maxY = 0;
  if( 3 != argc ){
    cout << "ImageID and File Path have to pass!" << endl;
    exit(1);
  }

  imageId = argv[1];
  fileName = argv[2];

  cout << "Starting image processing for [" << imageId << " : " << fileName << "]" << endl;
  patchingImage(imageId, fileName, maxX, maxY);
  /*

    cv::CommandLineParser parser(argc, argv, "{help h||}");
    if (parser.has("help"))
    {
        help(argv);
        return 0;
    }
    Mat I = Mat::eye(4, 4, CV_64F);
    I.at<double>(1,1) = CV_PI;
    cout << "I = \n" << I << ";" << endl << endl;
    Mat r = Mat(10, 3, CV_8UC3);
    randu(r, Scalar::all(0), Scalar::all(255));
    cout << "r (default) = \n" << r << ";" << endl << endl;
    cout << "r (matlab) = \n" << format(r, Formatter::FMT_MATLAB) << ";" << endl << endl;
    cout << "r (python) = \n" << format(r, Formatter::FMT_PYTHON) << ";" << endl << endl;
    cout << "r (numpy) = \n" << format(r, Formatter::FMT_NUMPY) << ";" << endl << endl;
    cout << "r (csv) = \n" << format(r, Formatter::FMT_CSV) << ";" << endl << endl;
    cout << "r (c) = \n" << format(r, Formatter::FMT_C) << ";" << endl << endl;
    Point2f p(5, 1);
    cout << "p = " << p << ";" << endl;
    Point3f p3f(2, 6, 7);
    cout << "p3f = " << p3f << ";" << endl;
    vector<float> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    cout << "shortvec = " << Mat(v) << endl;
    vector<Point2f> points(20);
    for (size_t i = 0; i < points.size(); ++i)
        points[i] = Point2f((float)(i * 5), (float)(i % 7));
    cout << "points = " << points << ";" << endl;
    */
    return 0;
}
