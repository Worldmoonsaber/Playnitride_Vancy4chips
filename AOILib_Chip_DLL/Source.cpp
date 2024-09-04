#include "pch.h"
#include "AOILib_Vancy4chips_V1.h"


#include<opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp> //mophorlogical operation
#include<opencv2/core.hpp>
#include <numeric>
#include <chrono>
#include "../Seccheck4chip_V1/Seccheck4chip_lib_V1.h"


using namespace cv;
using namespace std;

void Uchips_Vacancycheck(thresP thresParm, ImgP imageParm, SettingP chipsetting, sizeTD target, unsigned int* imageIN,
					unsigned int* imageOUT, unsigned char* imageGray, float boolResult[], float outputLEDX[], float outputLEDY[])
{

	#pragma region 格式轉換
	thresP_ _thresParm;

	_thresParm.thresmode = thresParm.thresmode;

	for (int i = 0; i < 3; i++)
	{
		_thresParm.bgmax[i] = thresParm.bgmax[i];
		_thresParm.bgmin[i] = thresParm.bgmin[i];
		_thresParm.fgmax[i] = thresParm.fgmax[i];
		_thresParm.fgmin[i] = thresParm.fgmin[i];
	}

	ImgP_ _imageParm;

	_imageParm.correctTheta = imageParm.correctTheta;
	_imageParm.imgcols = imageParm.cols;
	_imageParm.imgrows = imageParm.rows;
	_imageParm.Outputmode = imageParm.Outputmode;
	_imageParm.PICmode = imageParm.PICmode;

	SettingP_ _chipsetting;

	_chipsetting.carx = chipsetting.carx;
	_chipsetting.cary = chipsetting.cary;

	for (int i = 0; i < 4; i++)
		_chipsetting.interval[i] = chipsetting.interval[i];

	for (int i = 0; i < 3; i++)
	{
		_chipsetting.xpitch[i] = chipsetting.xpitch[i];
		_chipsetting.ypitch[i] = chipsetting.ypitch[i];
	}

	sizeTD_ _target;

	_target.TDheight = target.TDheight;
	_target.TDmaxH = target.TDmaxH;
	_target.TDmaxW = target.TDmaxW;
	_target.TDminH = target.TDminH;
	_target.TDminW = target.TDminW;
	_target.TDwidth = target.TDwidth;
#pragma endregion
	
	Mat rawimg, cropedRImg, gauBGR;
	Mat Gimg, drawF2;
	vector<Point> Fourchipspt;

	Point piccenter;
	Point2f creteriaPoint;
	Point IMGoffset=Point(0,0);

	//output parameters::
	Point crossCenter;
	int boolflag = 0;

	Mat image_input(imageParm.rows, imageParm.cols, CV_8UC4, &imageIN[0]); // THIS IS THE INPUT IMAGE, POINTER TO DATA			
	image_input.copyTo(rawimg);

	Mat image_output(800, 1200, CV_8UC4, &imageOUT[0]);
	Mat thres_output(800, 1200, CV_8UC1, &imageGray[0]);

	try
	{
		if (rawimg.empty())
		{
			boolflag = 8;
			throw "something wrong::input image failure";
		} //check if image is empty

	} //try loop
	catch (const char* message)
	{

		std::cout << "check catch state:: " << boolflag << endl;


	}//catch loop

	if ((chipsetting.interval[0] + 1) * chipsetting.interval[1] > rawimg.cols || (chipsetting.interval[0] + 1) * chipsetting.interval[2] > rawimg.rows)
	{
		boolflag == 7;
	}
	

	if (boolflag == 0) //&& imageParm.Outputmode == 0
	{
		/*image with CROP  process :::*/
		/*Automatically crop image via pitch setting*/
		piccenter = find_piccenter(rawimg);
		IMGoffset.x = piccenter.x - int((chipsetting.interval[0] + 1) * chipsetting.interval[1]);
		IMGoffset.y = piccenter.y - int((chipsetting.interval[0] + 1) * chipsetting.interval[2]);
		Rect Cregion(IMGoffset.x, IMGoffset.y, int((chipsetting.interval[0] + 1) * chipsetting.interval[1]) * 2, int((chipsetting.interval[0] + 1) * chipsetting.interval[2]) * 2);
		cropedRImg = CropIMG(rawimg, Cregion);


		/*Resize image to speed up*/
		_chipsetting.interval[1] = _chipsetting.interval[1] / 2; //490
		_chipsetting.interval[2] = _chipsetting.interval[2] / 2; //273 
		_chipsetting.xpitch[0] = _chipsetting.xpitch[0] / 2; //490
		_chipsetting.ypitch[0] = _chipsetting.ypitch[0] / 2; //273 

		_target.TDwidth = _target.TDwidth / 2;
		_target.TDheight = _target.TDheight / 2;
		cv::resize(cropedRImg, cropedRImg, Size(int(cropedRImg.cols / 2), int(cropedRImg.rows / 2)), INTER_LINEAR);

		///*///*image without CROP  process :::*/
		//sizeParm.CsizeW = rawimg.size[0];
		//sizeParm.CsizeH = sizeParm.CsizeW;
		//rawimg.copyTo(cropedRImg);

		/*Rotate picture::: */
		if (imageParm.correctTheta != 0)
		{
			cropedRImg = RotatecorrectImg(-1*imageParm.correctTheta, cropedRImg);
		}
		/*rotate end----------------*/



		creteriaPoint = find_piccenter(cropedRImg);
			
		std::tie(boolflag, Gimg, crossCenter, drawF2, Fourchipspt) = Uchip_singlephaseDownV3(boolflag, cropedRImg, _thresParm, _chipsetting, _target, creteriaPoint, IMGoffset, _imageParm);

		
		
	}

	std::cout << "check img state:: " << boolflag << endl;
	std::cout << "check center is ::" << crossCenter << endl;

	cv::resize(Gimg, Gimg, Size(1200, 800), INTER_LINEAR);
	cv::resize(drawF2, drawF2, Size(1200, 800), INTER_LINEAR);

	/*  :::::::OUTPUT area:::::::  */
	outputLEDX[0] = crossCenter.x ;
	outputLEDY[0] = crossCenter.y ;
	Gimg.copyTo(thres_output);
	drawF2.copyTo(image_output);
	boolResult[0] = boolflag;


	rawimg.release();
	cropedRImg.release();

	/*for (int i = 1; i < Fourchipspt.size() + 1; i++) //save corner coordinates
	{
		outputLEDX[i] = Fourchipspt[i - 1].x;
		outputLEDY[i] = Fourchipspt[i - 1].y;
	}*/
}

