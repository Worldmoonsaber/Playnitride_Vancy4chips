#pragma once
#pragma once
#pragma once

#include<opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp> //mophorlogical operation
#include<opencv2/core.hpp>
#include <chrono>
#include <numeric>
#include <fstream>

using namespace cv;
using namespace std;

typedef struct
{
	int interval[4];
	int xpitch[3]; //in unit of pixel 
	int ypitch[3];
	int carx;
	int cary;
}SettingP;


typedef struct
{
	double TDwidth;
	double TDmaxW;
	double TDminW;
	double TDheight;
	double TDmaxH;
	double TDminH;

}sizeTD;

typedef struct
{
	int thresmode; //0:gray ; 1:RGB ; 2:HSV ;3:AutoUP ; 4AutoDown
	int bgmax[3];
	int bgmin[3];
	int fgmax[3];
	int fgmin[3];

}thresP;

typedef struct
{
	int PICmode;
	int Outputmode;
	int imgcols;
	int imgrows;
	double correctTheta;
}ImgP;



/*Input function*/
std::tuple<int, Mat> Inputfunction();
std::tuple < vector<float>, vector<int>> dict_rectregion(int picorder);
void CreateRotImg(Mat src, int picsavenumber, double theta);




/*general operation*/
Point find_piccenter(Mat src);
Mat CropIMG(Mat img, Rect size);
//Mat KmeanOP(int knum, Mat src);

int findBoundary(Mat creteriaIMG, Rect inirect, char direction);
std::tuple<Rect, Point>FindMaxInnerRect(Mat src, Mat colorSRC, sizeTD target, Point TDcenter);
Mat RotatecorrectImg(double Rtheta, Mat src);




/*check sectional chips*/

std::tuple<vector<int>, vector<Rect>, vector<Point>, int>sectionalCheckFunction(Mat Reqgray, Point chipcenter, Rect chiprect,  SettingP chipsetting);


/******Single - phase chip:::*******/
//version 2.1
//std::tuple<int, Mat, Point, Mat>Uchip_singlephaseDownV2_1(int flag, Mat stIMG, thresP thresParm, SettingP chipsetting, sizeTD target, Point2f creteriaPoint, Point IMGoffset, ImgP imageParm);
std::tuple<int, Mat, Point, Mat, vector<Point>>Uchip_singlephaseDownV3(int flag, Mat stIMG, thresP thresParm, SettingP chipsetting, sizeTD target, Point2f creteriaPoint, Point IMGoffset, ImgP imageParm);


/******Dua - phase hcip:::********/
//std::tuple<int, Mat, Point, Mat>Uchip_dualphase(int flag, Mat stIMG, thresP thresParm, SettingP chipsetting, sizeTD target, Point2f creteriaPoint, Point IMGoffset, ImgP imageParm);
std::tuple<int, Mat, Point, Mat, vector<Point> >Uchip_dualphaseV2(int flag, Mat stIMG, thresP thresParm, SettingP chipsetting, sizeTD target, Point2f creteriaPoint, Point IMGoffset, ImgP imageParm);