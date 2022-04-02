#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/saliency/saliencySpecializedClasses.hpp>
#include <iostream>
#include <string>
#include <queue>
#include "mysql_connection.h"
#include "mysql_driver.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;
using namespace cv;
using namespace sql;

#define THREAD_POOL_COUNT 50
#define QUERY_CONNECTION

typedef struct{
  string imageId;
  int x;
  int y;
} THREAD_PARAM;

const string PATCHIFIED = "2";
const string CALCULATING = "3";
const string CALCULATED = "4";
const string PROCESSING_FAIL = "5";

const int patchWidth = 256;
const int patchHeight = 256;
string patchFilePath = "./patch_data/";
string saliencyMapPath = "./saliency_map/";

const string mysqlHost = "tcp://127.0.0.1:3306";
const string mysqlUser = "lunit";
const string mysqlPassword = "lunit-test";
const string mysqlDatabase = "lunit"; 

mysql::MySQL_Driver *mysqlDriver = nullptr;
Connection *mysqlConnection = nullptr;
Statement *mysqlStatement = nullptr;
queue<THREAD_PARAM> processingQueue;
pthread_cond_t cond;
pthread_mutex_t threadMutex;
pthread_mutex_t sqlMutex;
int threadCount = THREAD_POOL_COUNT;

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
#ifdef QUERY_CONNECTION
  mysqlStatement->execute(queryString);
#endif // QUERY_CONNECTION
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
  queryString += "')";
#ifdef QUERY_CONNECTION
  mysqlStatement->execute(queryString);
#endif // QUERY_CONNECTION
}

void updatePatchStatus(string imageId, Point element, string &histogram_string, bool saliencyMapResult, bool histogramResult, string &queryString){
  pthread_mutex_lock(&sqlMutex);
  queryString = "update lunit.patchImageMeta set `saliencyStatus` = '";
  
  queryString += saliencyMapResult ? "4" : "3";
  queryString += "', `histogramStatus` = '";
  queryString += histogramResult ? "4" : "3";
  queryString += "', `histogram` = '";
  queryString += histogram_string;
  queryString += "' where (`xIndex` = '";
  queryString += to_string(element.x);
  queryString += "' and `yIndex` = '";
  queryString += to_string(element.y);
  queryString += "' and `imageId` = '";
  queryString += imageId;
  queryString += "')";
#ifdef QUERY_CONNECTION
  mysqlStatement->execute(queryString);
#endif // QUERY_CONNECTION
  pthread_mutex_unlock(&sqlMutex);
}

void calculatePatchNumbersAndCloutSize(Size imageSize, int &maxX, int &maxY, int &cloutWidth, int &cloutHeight){
  cloutWidth = imageSize.width % patchWidth;
  cloutHeight = imageSize.height % patchHeight;

  maxX = imageSize.width / patchWidth + ( 0 == cloutWidth ? 0 : 1) ;
  maxY = imageSize.height / patchHeight + ( 0 == cloutHeight ? 0 : 1) ;

  cloutWidth = ( 0 == cloutWidth ) ? patchWidth : cloutWidth;
  cloutHeight = ( 0 == cloutHeight ) ? patchHeight : cloutHeight;
}

string getFileName(string imageId, string path, int x, int y){
  string fileName = path;
  fileName += imageId;
  fileName += "_";
  fileName += to_string(x);
  fileName += "_";
  fileName += to_string(y);
  fileName += ".jpg";
  return fileName;
}

string getSaliencyMapName(string imageId, int x, int y){
  return getFileName(imageId, saliencyMapPath, x, y);
}

string getPatchName(string imageId, int x, int y){
  return getFileName(imageId, patchFilePath, x, y);
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

void buildProcessingQueue(string imageId, int maxX, int maxY){
  int i, j;

  for( i = 0 ; i < maxY ; ++i ){
    for(j = 0; j < maxX ; ++j ){
      THREAD_PARAM element;
      element.imageId = imageId;
      element.x = j + 1;
      element.y = i + 1;
      processingQueue.push(element);
    }
  }
}

bool processingSaliencyMap(string imageId, Point element){
  Mat intermediateMap, saliencyMap;
  string patchFileName = getPatchName(imageId, element.x, element.y);
  string saliencyMapFileName = getSaliencyMapName(imageId, element.x, element.y);
  Mat patchImage = imread(patchFileName);
  cv::saliency::StaticSaliencySpectralResidual::create()->computeSaliency(patchImage, intermediateMap);
  intermediateMap = intermediateMap * 255;
  intermediateMap.convertTo(saliencyMap, CV_8U);
  imwrite(saliencyMapFileName, saliencyMap);
  return true;
}

bool processingHistogram(string imageId, Point element, string &histogram_string){
  const int* channel_numbers = { 0 };
  float channel_range[] = { 0.0, 255.0 };
  const float* channel_ranges = channel_range;
  int number_bins = 255;
  MatND histogram;
  Mat patchImage = imread(getPatchName(imageId, element.x, element.y), 0);
  calcHist(&patchImage, 1, channel_numbers, Mat(), histogram, 1, &number_bins, &channel_ranges);
  for( int i = 0 ; i < 256 ; ++i ){
    histogram_string += to_string(cvRound(histogram.at<float>(i)));
    if( 255 != i ){
      histogram_string += ",";
    }
  }
  return true;
}
 
bool processingPatchingImage(string imageId, Point element, string queryString){
  string histogram_string;
  bool saliencyMapResult, histogramResult;

  cout << imageId << "[" << element.x << "," << element.y << "] is proceesssing." << endl; 

  saliencyMapResult = processingSaliencyMap(imageId, element);
  histogramResult = processingHistogram(imageId, element, histogram_string);
  updatePatchStatus(imageId, element, histogram_string, saliencyMapResult, histogramResult, queryString);
 
  return true;
}

void * threadWorker(void *param){
  while(!processingQueue.empty()){
    string queryString;
    pthread_mutex_lock(&threadMutex);
    THREAD_PARAM element = processingQueue.front();
    processingQueue.pop();
    pthread_mutex_unlock(&threadMutex);
    processingPatchingImage(element.imageId, Point(element.x, element.y), queryString);
  }
  pthread_exit(NULL);
}

void processingPathchingImages(string imageId, int maxX, int maxY){
  int i;
  //pthread_t threads[THREAD_POOL_COUNT];
  pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t)* threadCount);

  cout << threadCount << " of threads will be worked." << endl;

  buildProcessingQueue(imageId, maxX, maxY);
  pthread_mutex_init(&threadMutex, NULL);
  pthread_mutex_init(&sqlMutex, NULL);

  for( i = 0 ; i < threadCount ; ++i ){
    pthread_create(&threads[i], NULL, threadWorker, NULL);
  }

  for( i = 0 ; i < threadCount ; ++i ){
    pthread_join(threads[i], NULL);
  }

  pthread_mutex_destroy(&threadMutex);
  pthread_mutex_destroy(&sqlMutex);

  cout << threadCount << " of threads are done." << endl;

  free(threads);
}

void updateFileLcation(string imageId){
  patchFilePath += imageId;
  patchFilePath += "/";
  mkdir(patchFilePath.c_str(), 0666);
  saliencyMapPath += imageId;
  saliencyMapPath += "/";
  mkdir(saliencyMapPath.c_str(), 0666);
}

void initMySQL(){
  mysqlDriver = mysql::get_mysql_driver_instance();
  mysqlConnection = mysqlDriver->connect(mysqlHost, mysqlUser, mysqlPassword);
  mysqlStatement = mysqlConnection->createStatement();
  mysqlStatement->execute("use lunit");
}

void finalMySQL(){
  delete mysqlStatement;
  delete mysqlConnection;
}

int main(int argc, char** argv)
{
  string  imageId, fileName; 
  int maxX = 0, maxY = 0;
  if( 3 > argc ){
    cout << "ImageID and File Path have to pass!" << endl;
    exit(1);
  }

  initMySQL();

  imageId = argv[1];
  fileName = argv[2];
  threadCount = atoi(argv[3]);

  updateFileLcation(imageId);
  cout << "Starting image processing for [" << imageId << " : " << fileName << "]" << endl;
  patchingImage(imageId, fileName, maxX, maxY);
  processingPathchingImages(imageId, maxX, maxY);
  
  finalMySQL();
  return 0;
}
