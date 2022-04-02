# Docker 실행
Docker Hub에서 Ubuntu Docker Image pull

```console
docker pull ubuntu
```

Ubuntu OS 실행
```console
docker run -d -it -p 8899:8899 --network="host" --mount type=bind,source="$(pwd)",target="/root/host" ubuntu
```

# OpenCV 설치

Ubuntu Docker 안에서 실행

```console
docker exec -it [CONTAINER ID] /bin/bash
cd ~
apt update
apt upgrade
apt install sudo
sudo apt install screen vim wget curl cmake g++ wget unzip build-essential libjpeg-dev libtiff5-dev libpng-dev libavcodec-dev libavformat-dev libswscale-dev libdc1394-22-dev libxvidcore-dev libx264-dev libxine2-dev libv4l-dev v4l-utils libgstreamer1.0-dev git libgstreamer-plugins-base1.0-dev libgtk-3-dev libmysqlcppconn-dev -y 
mkdir opencv
wget -O opencv-4.0.0.zip https://github.com/opencv/opencv/archive/4.0.0.zip
wget -O opencv_contrib-4.0.0.zip https://github.com/opencv/opencv_contrib/archive/4.0.0.zip
unzip opencv-4.0.0.zip
unzip opencv_contrib-4.0.0.zip
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local -D BUILD_WITH_DEBUG_INFO=OFF -D BUILD_EXAMPLES=ON -D BUILD_opencv_python3=ON -D INSTALL_PYTHON_EXAMPLES=ON -D OPENCV_ENABLE_NONFREE=ON -D OPENCV_EXTRA_MODULES_PATH=../opencv_contrib-4.0.0/modules -D OPENCV_GENERATE_PKGCONFIG=ON -D WITH_TBB=ON ../opencv-4.0.0/
nproc
make -j 4
sudo make install
sudo ldconfig
```
아래 소스 코드로 OpenCV 설치 테스트
```c++
/*
 *
 * cvout_sample just demonstrates the serial out capabilities of cv::Mat
 *  That is, cv::Mat M(...); cout << M;  Now works.
 *
 */
#include "opencv2/core.hpp"
#include <iostream>
using namespace std;
using namespace cv;
static void help(char** argv)
{
    cout
    << "\n------------------------------------------------------------------\n"
    << " This program shows the serial out capabilities of cv::Mat\n"
    << "That is, cv::Mat M(...); cout << M;  Now works.\n"
    << "Output can be formatted to OpenCV, matlab, python, numpy, csv and \n"
    << "C styles Usage:\n"
    << argv[0]
    << "\n------------------------------------------------------------------\n\n"
    << endl;
}
int main(int argc, char** argv)
{
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
    return 0;
}
```
```console
g++ opencv_test.cpp -o opencv_test $(pkg-config --cflags --libs opencv4)
```
# Node.js 설치

```console
sudo curl -sL https://deb.nodesource.com/setup_16.x | sudo -E bash -
sudo apt install -y nodejs
node -v
npm -v
mkdir lunit
cd lunit/
npm init
npm install --save express
npm install --save mysql
npm install --save express-fileupload 
npm install --save cors
```

아래 소스 코드로 Node.js 테스트
```javascript
const express = require('express')
const app = express()
const port = 8899

app.get('/', (req, res) => {
  res.send('Hello World!')
})

app.listen(port, () => {
  console.log(`Example app listening on port ${port}`)
})
```
mysql console에서 lunit 계정 아이디 패스워드로 접속하도록 변경
```console
mysql> alter user 'lunit'@'%' identified with mysql_native_password by 'lunit-test'
```

# processing_worker build

```console
g++ -Wall -I/usr/include/cppconn image_processing_worker.cpp -o image_processing_worker $(pkg-config --cflags --libs opencv4) -L/usr/lib -lpthread -lmysqlcppconn
```
