#include "pch.h"
#include "AOILib_Vancy4chips_V1.h"


#include<opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp> //mophorlogical operation
#include<opencv2/core.hpp>
#include <numeric>
#include <chrono>


using namespace cv;
using namespace std;


/*general operation*/
Point find_piccenter(Mat src);
Mat CropIMG(Mat img, Rect size);
int findBoundary(Mat creteriaIMG, Rect inirect, char direction);
std::tuple<Rect, Point>FindMaxInnerRect(Mat src, Mat colorSRC, sizeTD target, Point TDcenter);
Mat RotatecorrectImg(double Rtheta, Mat src);

std::tuple<vector<int>, vector<Rect>, vector<Point>, int>sectionalCheckFunction(Mat Reqgray, Point chipcenter, Rect chiprect, SettingP chipsetting);


/******Single - phase chip:::*******/
//version 3
std::tuple<int, Mat, Point, Mat, vector<Point>>Uchip_singlephaseDownV3(int flag, Mat stIMG, thresP thresParm, SettingP chipsetting, sizeTD target, Point2f creteriaPoint, Point IMGoffset, ImgP imageParm);


/******Dua - phase hcip:::********/
//version 2
std::tuple<int, Mat, Point, Mat, vector<Point> >Uchip_dualphaseV2(int flag, Mat stIMG, thresP thresParm, SettingP chipsetting, sizeTD target, Point2f creteriaPoint, Point IMGoffset, ImgP imageParm);


/*chip algorithm */



void Uchips_Vacancycheck(thresP thresParm, ImgP imageParm, SettingP chipsetting, sizeTD target, unsigned int* imageIN,
					unsigned int* imageOUT, unsigned char* imageGray, float boolResult[], float outputLEDX[], float outputLEDY[])
{

	
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
		chipsetting.interval[1] = chipsetting.interval[1] / 2; //490
		chipsetting.interval[2] = chipsetting.interval[2] / 2; //273 
		target.TDwidth = target.TDwidth / 2;
		target.TDheight = target.TDheight / 2;
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
		

		//start to ISP//////////////////////////
		if (thresParm.fgmin[imageParm.PICmode] != 99999 && thresParm.bgmax[imageParm.PICmode] != 99999 && thresParm.thresmode == 0)
		{
			
			std::tie(boolflag, Gimg, crossCenter, drawF2, Fourchipspt) = Uchip_dualphaseV2(boolflag, cropedRImg, thresParm, chipsetting, target, creteriaPoint, IMGoffset, imageParm);

		}

		else
		{
			std::tie(boolflag, Gimg, crossCenter, drawF2, Fourchipspt) = Uchip_singlephaseDownV3(boolflag, cropedRImg, thresParm, chipsetting, target, creteriaPoint, IMGoffset, imageParm);;
		}

		
		
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

	/*for (int i = 1; i < Fourchipspt.size() + 1; i++) //save corner coordinates
	{
		outputLEDX[i] = Fourchipspt[i - 1].x;
		outputLEDY[i] = Fourchipspt[i - 1].y;
	}*/
}



#pragma region generaloperation

Point find_piccenter(Mat src) {
	int piccenterx = int(src.size().width * 0.5);
	int piccentery = int(src.size().height * 0.5);
	Point piccenter = Point(piccenterx, piccentery);
	return piccenter;
}

Mat CropIMG(Mat img, Rect size)
{
	Mat croppedIMG;
	img(size).copyTo(croppedIMG);
	return croppedIMG;

}

int findBoundary(Mat creteriaIMG, Rect inirect, char direction)
{
	int step = 1;
	auto findRecr = inirect;
	int BoundaryVal;

	switch (direction)
	{
	case 'L':
		while (true)
		{
			//const auto count = cv::countNonZero(inverted_mask(inside_rect));
			const auto count = cv::countNonZero(creteriaIMG(findRecr));
			if (count > 0)
			{
				BoundaryVal = findRecr.x;
				break;
			}
			findRecr.x -= step;
		}
		break;
	case 'T':
		while (true)
		{
			const auto count = cv::countNonZero(creteriaIMG(findRecr));
			if (count > 0)
			{
				BoundaryVal = findRecr.y;
				break;
			}
			findRecr.y -= step;
		}
		break;
	case 'R':
		while (true)
		{
			const auto count = cv::countNonZero(creteriaIMG(findRecr));
			if (count > 0)
			{
				BoundaryVal = findRecr.x;
				break;
			}
			findRecr.x += step;
		}
		break;

		break;
	case 'B':
		while (true)
		{
			const auto count = cv::countNonZero(creteriaIMG(findRecr));
			if (count > 0)
			{
				BoundaryVal = findRecr.y;
				break;
			}
			findRecr.y += step;
		}
		break;

	default:
		std::cout << "****** Error case mode ******" << endl;
		break;

	}


	std::cout << "finish findboundary~" << endl;
	std::cout << "fi";
	return BoundaryVal;
}

std::tuple<Rect, Point>FindMaxInnerRect(Mat src, Mat colorSRC, sizeTD target, Point TDcenter)
{
	//output:::
	Rect innerboundary;
	Point center = TDcenter;
	Mat markcolor = Mat::zeros(colorSRC.size(), CV_8UC4);
	colorSRC.copyTo(markcolor);
	cv::circle(markcolor, center, 2, Scalar(180, 180, 180), 5);
	cv::circle(src, center, 2, Scalar(180, 180, 180), 5);
	//
	//find inner rect:
	Size ksize;
	Mat src2;
	ksize = Size(15, 15);
	Mat Kcomclose = Mat::ones(ksize, CV_8UC1);
	cv::morphologyEx(src, src2, cv::MORPH_CLOSE, Kcomclose, Point(-1, -1), 4);//4
	threshold(src2, src2, 175, 255, THRESH_BINARY);
	vector<Vec4i> hierarchy;
	vector<vector<Point>> contours, reqCon;
	vector<Point> approx;
	Rect retCOMP;
	vector<Point> reqCenter;

	Rect fineRect;

	cv::findContours(src2, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	double areasrc = cv::contourArea(contours[0]);

	for (int i = 0; i < contours.size(); i++)
	{
		areasrc = cv::contourArea(contours[i]);
		if (areasrc < target.TDwidth * target.TDminH * target.TDheight)
		{
			Rect bdrect = cv::boundingRect(contours[i]);
			cv::rectangle(src2, bdrect, Scalar(255, 255, 255), -1);
		}
	}
	cv::findContours(src2, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	cv::drawContours(src2, contours, -1, Scalar(255, 255, 255), -1);
	//cv::approxPolyDP(contours[0], approx, arcLength(contours[0], true) * 0.02, true); //

	//std::cout << "check approx size : " << approx.size() << endl;

	//Step.NEW7-find inner rect (via tiny scanning mechanism) ::
	cv::Mat inverted_mask;
	cv::bitwise_not(src2, inverted_mask);
	cv::Mat pointsmsk = Mat::zeros(src.size(), CV_8UC1);;
	cv::findNonZero(src2, pointsmsk);
	const cv::Rect outside_rect = cv::boundingRect(pointsmsk);
	Mat TDrect = Mat::zeros(src.size(), CV_8UC1);
	src.copyTo(TDrect);
	cv::rectangle(TDrect, outside_rect, Scalar(180, 180, 180), 1); //Scalar(0, 0, 0)

	/*std::cout << "check area : " << outside_rect.width * outside_rect.height << " // " << target.TDwidth * target.TDheight << endl;
	std::cout << "calculate area : " << outside_rect.width * outside_rect.height << " // " << target.TDwidth * target.TDheight << endl;*/
	int step_w, step_h;


	/*if ((outside_rect.width * outside_rect.height) / (target.TDwidth * target.TDheight) > 0.8
		&& (outside_rect.width * outside_rect.height) / (target.TDwidth * target.TDheight) < 1.15 )*/


	std::cout << "[select inner rect...]" << endl;
	if (outside_rect.width > outside_rect.height)
	{
		step_w = 1;//2
		step_h = 1;//1
	}
	else
	{
		step_w = 1;//1
		step_h = 1;//2
	}


	auto inside_rect = outside_rect;


	while (true)
	{
		//const auto count = cv::countNonZero(inverted_mask(inside_rect));
		const auto count = cv::countNonZero(inverted_mask(inside_rect));


		if (count == 0)
		{
			// we found a rectangle we can use!
			break;
		}

		inside_rect.x += step_w;
		inside_rect.y += step_h;
		inside_rect.width -= (step_w * 2);
		inside_rect.height -= (step_h * 2);
	}



	//cv::rectangle(TDrect, inside_rect, Scalar(100, 100, 100), 1); //Scalar(0, 0, 0)

	/*std::cout << "check inside rect:: " << inside_rect << endl;
	std::cout << "check outside_rect:: " << outside_rect << endl;*/

	//Step.NEW8-find inner rect boundary ::

	cv::rectangle(inverted_mask, Rect(0, 0, inverted_mask.size().width, inverted_mask.size().height), Scalar(255, 255, 255), 1);
	Rect line = Rect(inside_rect.x, inside_rect.y, inside_rect.width, 1);
	//cv::rectangle(gamimg, line, Scalar(0, 0, 0), 1); //Scalar(0, 0, 0)
	const auto count = cv::countNonZero(inverted_mask(line));
	//std::cout << "999999999999999999999999999999999: " << count << endl;
	int leftBound;
	//Rect Leftline = Rect(inside_rect.x, inside_rect.y, 1, inside_rect.height); //360,355 
	Rect Leftline = Rect(int(inside_rect.x + 1), inside_rect.y + int(inside_rect.height * 0.15), 1, int(0.7 * inside_rect.height)); //360,355 
	//cv::rectangle(colorSRC, Leftline, Scalar(88, 50, 155), 2);
	leftBound = findBoundary(inverted_mask, Leftline, 'L');
	//std::cout << "check left boundary " << leftBound << endl;


	int topBound;
	//Rect Topline = Rect(inside_rect.x, inside_rect.y, inside_rect.width, 1);
	Rect Topline = Rect(inside_rect.x + int(0.15 * inside_rect.width), int(inside_rect.y + 1), int(0.7 * inside_rect.width), 1);
	topBound = findBoundary(inverted_mask, Topline, 'T');
	//std::cout << "check Top boundary " << topBound << endl;

	int RightBound;
	//Rect Rightline = Rect(inside_rect.x + inside_rect.width, inside_rect.y, 1, inside_rect.height);
	Rect Rightline = Rect(inside_rect.x + int(inside_rect.width - 1), inside_rect.y + int(inside_rect.height * 0.15), 1, int(0.7 * inside_rect.height));
	//cv::rectangle(colorSRC, Rightline, Scalar(88, 50, 155), 2);
	RightBound = findBoundary(inverted_mask, Rightline, 'R');
	//std::cout << "check right boundary " << RightBound << endl;

	int BottomBound;
	//Rect bottomline = Rect(inside_rect.x, inside_rect.y + inside_rect.height, inside_rect.width, 1);
	Rect bottomline = Rect(inside_rect.x + int(0.15 * inside_rect.width), inside_rect.y + int(inside_rect.height - 1), int(0.7 * inside_rect.width), 1);
	BottomBound = findBoundary(inverted_mask, bottomline, 'B');
	//std::cout << "check bottom boundary " << BottomBound << endl;

	//innerboundary = Rect(leftBound, topBound, (RightBound - leftBound), (BottomBound - topBound));

	innerboundary = Rect(leftBound, topBound, (RightBound - leftBound), (BottomBound - topBound));

	center = Point(int(innerboundary.x + innerboundary.width * 0.5), int(innerboundary.y + innerboundary.height * 0.5));
	//std::cout << "previous center is :: " << center << endl;

	cv::rectangle(markcolor, innerboundary, Scalar(0, 255, 255), 1);

	//std::cout << "innerboundary.width " << innerboundary.width << "/*/* " << target.TDwidth << endl;





	//Step.NEW9-Mark inner rect::
	cv::rectangle(markcolor, innerboundary, Scalar(0, 0, 255), 1);
	cv::rectangle(markcolor, fineRect, Scalar(255, 0, 0), 1);
	cv::rectangle(TDrect, innerboundary, Scalar(50, 50, 50), 2);
	cv::circle(markcolor, center, 2, Scalar(20, 20, 20), 5);
	//check area::
	/*std::cout << "check dimension-width::: " << innerboundary.width << " ||| " << target.TDwidth << endl;
	std::cout << "check dimension-height::: " << innerboundary.height << " ||| " << target.TDheight << endl;
	std::cout << "check center::: " << center << endl;*/

	std::cout << "fini" << endl;

	//
	return { innerboundary,center };

}

Mat RotatecorrectImg(double Rtheta, Mat src)
{
	Point2f center((src.cols - 1) / 2.0, (src.rows - 1) / 2.0);
	// using getRotationMatrix2D() to get the rotation matrix
	Mat rotation_matix = getRotationMatrix2D(center, Rtheta, 1.0);
	//angle:: ++ mean counter-clockwise rotation (the coordinate origin is assumed to be the top-left corner)

	// we will save the resulting image in rotated_image matrix
	Mat rotated_image;
	// rotate the image using warpAffine
	warpAffine(src, rotated_image, rotation_matix, src.size());
	Mat patchrotaIMG;
	rotated_image.copyTo(patchrotaIMG);

	for (int y = 0; y < rotated_image.rows; y++)
	{
		for (int x = 0; x < rotated_image.cols; x++)
		{
			if (rotated_image.at<Vec3b>(Point(x, y)) == Vec3b(0, 0, 0))
			{
				patchrotaIMG.at<Vec3b>(Point(x, y)) = Vec3b(255, 255, 255);
			}
		}
	}

	return patchrotaIMG;
}

std::tuple<vector<int>, vector<Rect>, vector<Point>, int>sectionalCheckFunction(Mat Reqgray, Point chipcenter, Rect chiprect, SettingP chipsetting)
{
	vector<int> Fourchipsstate = { 0,0,0,0 };
	vector<Rect> Fourchipsbdbox;
	vector<Point> Fourchipspt;
	int bolResultstate = 0;

	Mat mnotedimg;
	cvtColor(Reqgray, mnotedimg, COLOR_GRAY2BGR);

	vector<Point> Fdirection = { Point(-1,-1), Point(1,-1), Point(-1,1), Point(1,1) };
	vector <Point>Fpoint;
	cv::circle(mnotedimg, chipcenter, 30, Scalar::all(40), -1);

	/*patternmatch template*/


	Mat tempIMG = Mat::zeros(Size(chiprect.width, chiprect.height), CV_8UC1);
	int Nzeronum = 0;
	cv::bitwise_not(tempIMG, tempIMG);

	for (int i = 0; i < chipsetting.interval[0] + 1; i++)
	{
		Fpoint.push_back(Point(chipcenter.x + Fdirection[i].x * chipsetting.interval[0] * chipsetting.interval[1],
			chipcenter.y + Fdirection[i].y * chipsetting.interval[0] * chipsetting.interval[2]));

		//std::cout << "Fpoint;;" << Fpoint[Fpoint.size()-1] << endl;
		/*mark onto picture:: */
		cv::circle(mnotedimg, Fpoint[Fpoint.size() - 1], 30, Scalar::all(110), -1);

		Point IMGoffset = Point(0, 0);
		IMGoffset.x = Fpoint[Fpoint.size() - 1].x - chiprect.width * 0.7;
		IMGoffset.y = Fpoint[Fpoint.size() - 1].y - chiprect.height * 0.7;
		Rect Cregion(IMGoffset.x, IMGoffset.y, chiprect.width * 1.5, chiprect.height * 1.5);

		//std::cout << "Cregion;;" << Cregion  <<"chiprect width /hieght "<< double(chiprect.width)<<"  /  " << int(chiprect.height) << endl;

		Mat searchArea;
		Mat matchimg;
		try
		{
			searchArea = CropIMG(Reqgray, Cregion);
			cv::matchTemplate(searchArea, tempIMG, matchimg, TM_SQDIFF); //TM_SQDIFF  //TM_CCOEFF_NORMED
			double minVal; double maxVal; Point minLoc; Point maxLoc;
			minMaxLoc(matchimg, &minVal, &maxVal, &minLoc, &maxLoc, Mat());



			Fourchipsbdbox.push_back(Rect(minLoc + IMGoffset, Point(minLoc.x + tempIMG.cols, minLoc.y + tempIMG.rows) + IMGoffset));
			Fourchipspt.push_back(Fpoint[Fpoint.size() - 1]);

			cv::circle(mnotedimg, Fourchipspt[Fourchipspt.size() - 1], 3, Scalar::all(180), -1);

			rectangle(mnotedimg, Fourchipsbdbox[Fourchipsbdbox.size() - 1],
				Scalar::all(170), 5, 8, 0); //mark		
			Mat Nzeroimg = CropIMG(Reqgray, Fourchipsbdbox[Fourchipsbdbox.size() - 1]);

			Nzeronum = countNonZero(Nzeroimg);

		}

		catch (std::exception& e) { // exception should be caught by reference

			Nzeronum = 0;
			cout << "exception: " << e.what() << "\n";
		}

		if (Nzeronum > 5)
		{
			Fourchipsstate[i] = 1;
		}
		else
		{
			Fourchipsstate[i] = 0;
		}

		std::cout << "Nzeronum is : " << Nzeronum << endl;
		std::cout << " current state: " << Fourchipsstate[i] << endl;
		std::cout << "*************************************" << endl;

	}

	if (std::accumulate(Fourchipsstate.begin(), Fourchipsstate.end(), 0) == 0)
	{
		bolResultstate = 9;
	}
	else
	{
		bolResultstate = 10;
	}

	std::cout << "bolResultstate;; " << bolResultstate << " / ( " << std::accumulate(Fourchipsstate.begin(), Fourchipsstate.end(), 0) << " )" << endl;

	return{ Fourchipsstate,Fourchipsbdbox ,Fourchipspt ,bolResultstate };
}


#pragma endregion generaloperation

#pragma region chipalgorithm

std::tuple<int, Mat, Point, Mat, vector<Point> >Uchip_singlephaseDownV3(int flag, Mat stIMG, thresP thresParm, SettingP chipsetting, sizeTD target, Point2f creteriaPoint, Point IMGoffset, ImgP imageParm)
{
	auto t_start = std::chrono::high_resolution_clock::now();

	//output parm:::::
	Mat Reqcomthres = Mat::zeros(stIMG.rows, stIMG.cols, CV_8UC1);
	Point crossCenter;
	Mat marksize;
	Mat Rotmarkpic = Mat::ones(stIMG.rows, stIMG.cols, CV_8UC3);
	Mat Rotnew = Mat::ones(stIMG.rows, stIMG.cols, CV_8UC3);
	Mat thresRot;
	Point crossCenternew;
	vector<int> Fourchipsstate;
	vector<Rect> Fourchipsbdbox;
	vector<Point> Fourchipspt;




	/////Step.1-Define Bright area::: 
	Mat Gimg, gauGimh;
	Mat gauBGR;


	vector<vector<Point>>  contH, contRot; // Vector for storing contour
	vector<Vec4i> hierH, hierRot;
	vector<vector<Point>> reqConH;

	Mat adptThres;
	Mat HIGHthres;
	Mat comthresIMG;
	int adaptWsize = 3;
	int adaptKsize = 2;

	vector<vector<Point>>  contours, REQcont; // Vector for storing contour
	Rect retCOMP;
	vector<Rect> Rectlist;
	vector<Point2f> center;
	vector<double> distance;
	Point2f piccenter;
	int minIndex;
	vector<Point> approx;
	vector< double> approxList;
	double areacomthres;

	//stIMG.copyTo(marksize);

	if (stIMG.channels() == 3)
	{

		stIMG.copyTo(marksize);

		stIMG.convertTo(stIMG, -1, 1.2, 0);//1.5:pic1 // 1.2:pic5
		cv::GaussianBlur(stIMG, gauBGR, Size(0, 0), 13);//15
		cv::addWeighted(stIMG, 1.5, gauBGR, -0.7, 0.0, stIMG); //(1.5, -0.7) //1.6,-0.7


		cv::cvtColor(stIMG, Gimg, COLOR_RGB2GRAY);
		cv::fastNlMeansDenoising(Gimg, gauGimh, 3, 7, 21);
	}
	else if (stIMG.channels() == 4)
	{
		stIMG.copyTo(marksize);
		stIMG.convertTo(stIMG, -1, 1.2, 0);//1.5:pic1 // 1.2:pic5
		cv::GaussianBlur(stIMG, gauBGR, Size(0, 0), 13);//15
		cv::addWeighted(stIMG, 1.5, gauBGR, -0.7, 0.0, stIMG); //(1.5, -0.7) //1.6,-0.7


		cv::cvtColor(stIMG, Gimg, COLOR_RGBA2GRAY);
		cv::fastNlMeansDenoising(Gimg, gauGimh, 3, 7, 21);
	}
	else
	{

		stIMG.convertTo(stIMG, -1, 1.2, 0);//1.5:pic1 // 1.2:pic5
		cv::GaussianBlur(stIMG, gauBGR, Size(0, 0), 13);//15
		cv::addWeighted(stIMG, 1.5, gauBGR, -0.7, 0.0, stIMG); //(1.5, -0.7) //1.6,-0.7
		stIMG.copyTo(Gimg);

		cv::cvtColor(stIMG, marksize, COLOR_GRAY2RGBA);
		cv::fastNlMeansDenoising(Gimg, gauGimh, 3, 7, 21);
	}



	//define thres-high area::	
	if (thresParm.thresmode == 0)
	{
		Scalar maxthres = Scalar(thresParm.fgmax[imageParm.PICmode], thresParm.fgmax[imageParm.PICmode], thresParm.fgmax[imageParm.PICmode]);
		Scalar minthres = Scalar(thresParm.fgmin[imageParm.PICmode], thresParm.fgmin[imageParm.PICmode], thresParm.fgmin[imageParm.PICmode]);
		cv::inRange(gauGimh, minthres, maxthres, HIGHthres);
		cv::medianBlur(HIGHthres, comthresIMG, 17);
		Mat Kcomclose = Mat::ones(Size(5, 5), CV_8UC1);  //Size(10,5)
		cv::morphologyEx(comthresIMG, comthresIMG, cv::MORPH_CLOSE, Kcomclose, Point(-1, -1), 1);//1 //2
	}
	else if (thresParm.thresmode == 3)
	{
		if (thresParm.bgmax[imageParm.PICmode] & 1)
		{
			adaptWsize = thresParm.bgmax[imageParm.PICmode];
			adaptKsize = thresParm.fgmax[imageParm.PICmode];
		}
		else
		{
			adaptWsize = thresParm.bgmax[imageParm.PICmode] + 1;
			adaptKsize = thresParm.fgmax[imageParm.PICmode];
		}
		adaptiveThreshold(gauGimh, adptThres, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, adaptWsize, adaptKsize);//55,1 //ADAPTIVE_THRESH_MEAN_C

		cv::medianBlur(adptThres, comthresIMG, 7);
		Mat Kcomclose = Mat::ones(Size(5, 5), CV_8UC1);  //Size(10,5)
		cv::morphologyEx(comthresIMG, comthresIMG, cv::MORPH_CLOSE, Kcomclose, Point(-1, -1), 1);//1 //2
	}

	else if (thresParm.thresmode == 4)
	{
		if (thresParm.bgmax[imageParm.PICmode] & 1)
		{
			adaptWsize = thresParm.bgmax[imageParm.PICmode];
			adaptKsize = thresParm.fgmax[imageParm.PICmode];
		}
		else
		{
			adaptWsize = thresParm.bgmax[imageParm.PICmode] + 1;
			adaptKsize = thresParm.fgmax[imageParm.PICmode];
		}
		adaptiveThreshold(gauGimh, adptThres, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, adaptWsize, adaptKsize);//55,1 //ADAPTIVE_THRESH_MEAN_C
		cv::medianBlur(adptThres, comthresIMG, 7);
		Mat Kcomclose = Mat::ones(Size(5, 5), CV_8UC1);  //Size(10,5)
		cv::morphologyEx(comthresIMG, comthresIMG, cv::MORPH_CLOSE, Kcomclose, Point(-1, -1), 1);//1 //2
	}



	Mat finescanIMG = Mat::zeros(stIMG.rows, stIMG.cols, CV_8UC1);
	Rect drawrect;
	Rect fineRect;
	Point centerTD;







	cv::findContours(comthresIMG, contH, hierH,
		cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());

	cv::drawContours(Reqcomthres, contH, -1, Scalar(255, 255, 255), -1);


	try
	{

		if (contH.size() == 0)
		{
			flag = 1;
			throw "something wrong::threshold value issue";
		}

		else
		{
			if (thresParm.thresmode == 3 || 4)
			{


				for (int i = 0; i < contH.size(); i++)
				{

					retCOMP = cv::boundingRect(contH[i]);
					areacomthres = cv::contourArea(contH[i]);
					cv::approxPolyDP(contH[i], approx, 15, true); //30,15
					if (retCOMP.width > target.TDwidth * target.TDminW
						&& retCOMP.height > target.TDheight * target.TDminH
						&& retCOMP.width < target.TDwidth * target.TDmaxW
						&& retCOMP.height < target.TDheight * target.TDmaxH
						)

					{
						Moments M = (moments(contH[i], false));
						center.push_back((Point2f((M.m10 / M.m00), (M.m01 / M.m00))));
						piccenter = find_piccenter(comthresIMG);
						distance.push_back(norm((creteriaPoint)-center[center.size() - 1])); // get Euclidian distance
						Rectlist.push_back(retCOMP);
						approxList.push_back(approx.size());
						REQcont.push_back(contH[i]);
						cv::rectangle(marksize, retCOMP, Scalar(255, 255, 255), 4);
						cv::rectangle(Reqcomthres, retCOMP, Scalar(255, 255, 255), -1);


					}

				} //for-loop: contours



			}//if-loop: (thresParm.thresmode == 3 || 4)

			else //(thresParm.thresmode == 0 || 1|| 2)
			{
				for (int i = 0; i < contH.size(); i++)
				{

					retCOMP = cv::boundingRect(contH[i]);
					areacomthres = cv::contourArea(contH[i]);
					cv::approxPolyDP(contH[i], approx, 15, true); //30,15

					//retCOMP.width * retCOMP.height > 0.6 * sizeParm.TDminW * sizeParm.TDminH && retCOMP.width * retCOMP.height < sizeParm.TDmaxW * sizeParm.TDmaxH * 1.2
					if (areacomthres > target.TDwidth * target.TDminW * target.TDheight * target.TDminH && areacomthres < target.TDwidth * target.TDmaxW * target.TDheight * target.TDmaxH)

					{
						Moments M = (moments(contH[i], false));
						center.push_back((Point2f((M.m10 / M.m00), (M.m01 / M.m00))));
						piccenter = find_piccenter(comthresIMG);
						distance.push_back(norm((creteriaPoint)-center[center.size() - 1])); // get Euclidian distance
						Rectlist.push_back(retCOMP);
						approxList.push_back(approx.size());
						REQcont.push_back(contH[i]);
						cv::rectangle(marksize, retCOMP, Scalar(255, 255, 255), 4);
						cv::rectangle(Reqcomthres, retCOMP, Scalar(255, 255, 255), -1);


					}

				} //for-loop: contours
			} //if-loop: (thresParm.thresmode == 0 || 1|| 2)


			//draw pic center:: 
			cv::circle(marksize,
				Point2i(piccenter), //coordinate
				9, //radius
				Scalar(0, 255, 255),  //color
				FILLED,
				LINE_AA);

			if (center.size() == 0)
			{
				flag = 2;
				throw "something wrong::potential object doesn't fit suitable dimension";
			}
			else
			{


				//Find a LED coordinate with the shortest distance to the pic center
				auto it = std::min_element(distance.begin(), distance.end());
				minIndex = std::distance(distance.begin(), it);
				//minvalue = *it;

				if (distance[minIndex] > chipsetting.xpitch[0])
				{
					flag = 6;
					throw "something wrong::potential object doesn't fit suitable dimension";
				}
				else
				{
					std::wcout << "check approx  size main: " << approxList[minIndex] << endl;


					if (approxList[minIndex] < 9) //==4
					{
						crossCenter = center[minIndex] + Point2f(chipsetting.carx, chipsetting.cary);
						drawrect = Rect(Rectlist[minIndex].x,
							Rectlist[minIndex].y, //rectangle ini y
							Rectlist[minIndex].width, //rectangle width
							Rectlist[minIndex].height); //rectangle height


					}
					else /*fine define chip centers*/
					{
						std::cout << "start fine define...." << endl;
						cv::drawContours(finescanIMG, REQcont, minIndex, Scalar::all(255), -1);

						tie(fineRect, centerTD) = FindMaxInnerRect(finescanIMG, stIMG, target, center[minIndex]);

						crossCenter = Point2f(centerTD) + Point2f(chipsetting.carx, chipsetting.cary);
						drawrect = fineRect;

					}

					cv::circle(marksize,
						(Point2i(crossCenter)), //coordinate
						6, //radius
						Scalar(255, 0, 255),  //color
						FILLED,
						LINE_AA);

					//cv::rectangle(marksize, drawrect, cv::Scalar(255, 255, 255), 1);
					/*cv::line(marksize, Point(0, crossCenter.y - chipsetting.cary), Point(marksize.size[1], crossCenter.y - chipsetting.cary), Scalar(255, 255, 255), 1, 8);
					cv::line(marksize, Point(crossCenter.x - chipsetting.carx, 0), Point(crossCenter.x - chipsetting.carx, marksize.size[0]), Scalar(255, 255, 255), 1, 8);*/


					cv::line(marksize, Point(0, crossCenter.y), Point(marksize.size[1], crossCenter.y), Scalar(255, 255, 255), 1, 8);
					cv::line(marksize, Point(crossCenter.x, 0), Point(crossCenter.x, marksize.size[0]), Scalar(255, 255, 255), 1, 8);

					/*check vacancy ::::*/

					int flag_test;

					std::cout << "+++++++++++fineRect : " << fineRect << endl;
					std::cout << "+++++++++++drawrect : " << drawrect << endl;

					std::tie(Fourchipsstate, Fourchipsbdbox, Fourchipspt, flag_test) = sectionalCheckFunction(Reqcomthres, Point(crossCenter), drawrect, chipsetting);



					if (flag_test == 10)
					{
						for (int i = 0; i < Fourchipsbdbox.size(); i++)
						{
							rectangle(marksize, Fourchipsbdbox[i], Scalar(130, 0, 255), 15);
						}


					}
					else
					{
						vector<Point> Fdirection = { Point(-1,-1), Point(1,-1), Point(-1,1), Point(1,1) };
						for (int i = 0; i < chipsetting.interval[0] + 1; i++)
						{
							Point drawpt = (Point(crossCenter.x + Fdirection[i].x * chipsetting.interval[0] * chipsetting.interval[1],
								crossCenter.y + Fdirection[i].y * chipsetting.interval[0] * chipsetting.interval[2]));

							//std::cout << "Fpoint;;" << Fpoint[Fpoint.size()-1] << endl;
							/*mark onto picture:: */
							cv::circle(marksize, drawpt, 15, Scalar(255, 0, 255), -1);
						}
					}


					flag = flag_test;


					if (imageParm.correctTheta != 0)
					{
						cv::circle(Rotnew,
							(Point2i(crossCenter)), //coordinate
							6, //radius
							Scalar(180, 180, 180),  //color
							FILLED,
							LINE_AA);
						Rotmarkpic = RotatecorrectImg(-1 * imageParm.correctTheta, Rotnew);
						marksize = RotatecorrectImg(-1 * imageParm.correctTheta, marksize);
						cv::inRange(Rotmarkpic, Scalar(175, 175, 175), Scalar(185, 185, 185), thresRot);
						cv::findContours(thresRot, contRot, hierRot, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());
						Moments Mans = (moments(contRot[0], false));
						crossCenternew = Point2i((Point2f((Mans.m10 / Mans.m00), (Mans.m01 / Mans.m00)))) + IMGoffset;
						std::cout << "check chip crossCenternew is: [ " << crossCenternew << " ]" << endl;
						std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
					}
					else
					{
						crossCenternew = crossCenter + IMGoffset;
						std::cout << "check chip crossCenternew is: [ " << crossCenternew << " ]" << endl;
						std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
					}

				}


			}


		}// if-loop:contH fits suitable area 



	} //try-loop
	catch (const char* message)
	{
		std::cout << message << std::endl;

	}







	//////////////////////////////////////////////////////////output//////////////////////////////////
	auto t_end = std::chrono::high_resolution_clock::now();
	double elapsed_time_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
	std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	std::cout << "check chip center is: [ " << crossCenter << " ]" << endl;
	std::cout << "result flag is :: " << flag << endl;
	std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	std::cout << "calculate color-filter time is:: " << elapsed_time_ms << endl;






	std::cout << "fini" << endl;
	return { flag, Reqcomthres, crossCenternew, marksize,Fourchipspt };
}












std::tuple<int, Mat, Point, Mat, vector<Point>>Uchip_dualphaseV2(int flag, Mat stIMG, thresP thresParm, SettingP chipsetting, sizeTD target, Point2f creteriaPoint, Point IMGoffset, ImgP imageParm)
{
	auto t_start = std::chrono::high_resolution_clock::now();

	//output parm:::::
	Mat Rotmarkpic = Mat::ones(stIMG.rows, stIMG.cols, CV_8UC3);
	Mat Rotnew = Mat::ones(stIMG.rows, stIMG.cols, CV_8UC3);
	Mat thresRot;
	Point crossCenternew;
	vector<int> Fourchipsstate;
	vector<Point> Fourchipspt;
	vector<Rect> Fourchipsbdbox;

	Mat Reqcomthres = Mat::zeros(stIMG.rows, stIMG.cols, CV_8UC1);
	Point crossCenter;
	Mat marksize;


	Size S_kermask, S_kernelLOW2, S_Kcomclose;

	if (target.TDheight < target.TDwidth)
	{


		S_kermask = Size(10, 1);
		S_kernelLOW2 = Size(1, 7);
		S_Kcomclose = Size(10, 5);
	}
	else
	{


		S_kermask = Size(1, 10);
		S_kernelLOW2 = Size(7, 1);
		S_Kcomclose = Size(5, 10);
	}


	/////Step.1-Define Bright area::: 
	Mat Gimg, gauGimh;
	if (stIMG.channels() == 3)
	{
		stIMG.copyTo(marksize);
		cv::cvtColor(stIMG, Gimg, COLOR_BGR2GRAY);
		cv::fastNlMeansDenoising(Gimg, gauGimh, 3, 7, 21);
	}
	else if (stIMG.channels() == 4)
	{
		stIMG.copyTo(marksize);
		cv::cvtColor(stIMG, Gimg, COLOR_BGR2GRAY);
		cv::fastNlMeansDenoising(Gimg, gauGimh, 3, 7, 21);
	}
	else
	{
		stIMG.copyTo(Gimg);

		cv::cvtColor(stIMG, marksize, COLOR_GRAY2RGBA);
		cv::fastNlMeansDenoising(Gimg, gauGimh, 3, 7, 21);

	}




	vector<vector<Point>>  contH; // Vector for storing contour
	vector<Vec4i> hierH;
	vector<vector<Point>> reqConH;
	Mat ReqH = Mat::zeros(stIMG.rows, stIMG.cols, CV_8UC1);
	vector<double> AreaHigh;



	Mat LOWimg = Mat::zeros(stIMG.rows, stIMG.cols, CV_8UC1);
	Mat BGRmask, BGRclose;



	Scalar maxHigh = Scalar(thresParm.fgmax[0], thresParm.fgmax[0], thresParm.fgmax[0]);
	Scalar minHigh = Scalar(thresParm.fgmin[0], thresParm.fgmin[0], thresParm.fgmin[0]);

	Scalar maxLow = Scalar(thresParm.bgmax[0], thresParm.bgmax[0], thresParm.bgmax[0]);
	Scalar minLow = Scalar(thresParm.bgmin[0], thresParm.bgmin[0], thresParm.bgmin[0]);











	Mat HIGHthres;
	//define thres-high area::

	cv::inRange(gauGimh, minHigh, maxHigh, HIGHthres);

	/*cv::threshold(gauGimh, HIGHthres, thresHIGHVal, 255, THRESH_BINARY);*/




	vector<vector<Point>>  contL; // Vector for storing contour
	vector<Vec4i> hierL;
	vector<vector<Point>> reqConL;
	Rect retlow;
	vector<Point> approx;



	double areaLow;

	RotatedRect newtst;
	vector<Point> hull;

	vector<vector<Point>> contLLow, contSLow;
	Mat picLLow = Mat::zeros(stIMG.rows, stIMG.cols, CV_8UC1);
	Mat picSLow = Mat::zeros(stIMG.rows, stIMG.cols, CV_8UC1);

	Mat BGRmask_OPEN;
	Mat comthresIMG;

	vector<vector<Point>>  contours; // Vector for storing contour
	vector<Vec4i> hierarchy;
	Rect retCOMP;
	vector<Rect> Rectlist;
	vector<Point2f> center;
	vector<double> distance;
	Point2f piccenter;
	int minIndex;
	double areacomthres;
	vector<vector<Point>> contRot;



	cv::findContours(HIGHthres, contH, hierH,
		cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());
	try
	{

		if (contH.size() == 0)
		{
			flag = 3;
			throw "something wrong::there is not any chip on screen";
		}



		else
		{
			cv::drawContours(ReqH, contH, -1, Scalar(255, 255, 255), -1); //*bright area time=300 ms*/





			/////Step.2-Define Dark area::: 


			gauGimh.copyTo(LOWimg);
			cv::inRange(LOWimg, minLow, maxLow, BGRmask);

			//cv::threshold(LOWimg, BGRmask, thresLowVal, 255, THRESH_BINARY_INV);

			cv::medianBlur(BGRmask, BGRmask, 11); //15
			cv::rectangle(BGRmask, Rect(0, 0, BGRmask.rows, BGRmask.cols), Scalar(0, 0, 0), 2); //draw a rect-boundary onto Lowthres image



			Mat kermask = Mat::ones(S_kermask, CV_8UC1);
			cv::morphologyEx(BGRmask, BGRmask, cv::MORPH_CLOSE, kermask, Point(-1, -1), 1);//1 //2




			cv::findContours(BGRmask, contL, hierL,
				cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());

			if (contL.size() == 0)
			{
				flag = 4;
				throw "something wrong:: Low-thres value issue";
			}
			else
			{

				for (int i = 0; i < contL.size(); i++)
				{


					areaLow = cv::contourArea(contL[i]);
					cv::approxPolyDP(contL[i], approx, 15, true); //30,15
					cv::convexHull(contL[i], hull, false, false);

					if (areaLow < target.TDheight * target.TDmaxH * target.TDwidth * target.TDmaxW / 3) //small area:::::areaLow < sizeParm.TDmaxH * sizeParm.TDmaxW / 3 * 0.8
					{
						if (approx.size() >= 3 && hull.size() > 10 && approx.size() <= 13) //4,20,13
						{
							contSLow.push_back(contL[i]);

						}


					}



				} //for-loop: contL size



				Mat kernelLOW2 = Mat::ones(S_kernelLOW2, CV_8UC1);  //Size(1,7)
				cv::morphologyEx(BGRmask, BGRmask_OPEN, cv::MORPH_OPEN, kernelLOW2, Point(-1, -1), 2);//1 //2
				cv::findContours(BGRmask_OPEN, contL, hierL,
					cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());

				for (int i = 0; i < contL.size(); i++)
				{


					areaLow = cv::contourArea(contL[i]);
					cv::approxPolyDP(contL[i], approx, 15, true); //30,15
					cv::convexHull(contL[i], hull, false, false);

					if (areaLow < target.TDheight * target.TDmaxH * target.TDwidth * target.TDmaxW / 3
						&& areaLow>target.TDheight * target.TDminH * target.TDwidth * target.TDminW / 9) //small area:::::
					{
						if (approx.size() >= 3 && hull.size() >= 9 && approx.size() <= 13) //4,20,13
						{



							contLLow.push_back(contL[i]);

						}




					}




				}
				cv::drawContours(picSLow, contSLow, -1, Scalar(255, 255, 255), -1);
				cv::drawContours(picLLow, contLLow, -1, Scalar(255, 255, 255), -1);



				comthresIMG = ReqH + picLLow + picSLow;
				Mat Kcomclose = Mat::ones(S_Kcomclose, CV_8UC1);  //Size(15,5)
				cv::morphologyEx(comthresIMG, comthresIMG, cv::MORPH_CLOSE, Kcomclose, Point(-1, -1), 2);//1 //2






				/*start to define req target::: */

				cv::findContours(comthresIMG, contours, hierarchy,
					cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point()); //RETR_LIST /RETR_EXTERNAL



				if (contours.size() == 0)
				{
					flag = 5;
					throw "something wrong::combine issue";
				}
				else
				{
					for (int i = 0; i < contours.size(); i++)
					{

						retCOMP = cv::boundingRect(contours[i]);
						areacomthres = cv::contourArea(contours[i]);
						cv::approxPolyDP(contours[i], approx, 15, true); //30,15

						if (retCOMP.width > target.TDwidth * target.TDminW
							&& retCOMP.height > target.TDheight * target.TDminH
							&& retCOMP.width < target.TDwidth * target.TDmaxW
							&& retCOMP.height < target.TDheight * target.TDmaxH)

						{
							center.push_back(Point2f(retCOMP.width * 0.5 + retCOMP.x, retCOMP.y + retCOMP.height * 0.5));
							piccenter = find_piccenter(comthresIMG);
							distance.push_back(norm((piccenter)-center[center.size() - 1])); // get Euclidian distance
							Rectlist.push_back(retCOMP);
							cv::rectangle(marksize, retCOMP, Scalar(255, 255, 255), 1);
							cv::rectangle(Reqcomthres, retCOMP, Scalar(255, 255, 255), -1);

						}


					} //for-loop: contours
					cv::circle(marksize,
						Point2i(piccenter), //coordinate
						9, //radius
						Scalar(0, 255, 255),  //color
						FILLED,
						LINE_AA);

					if (center.size() == 0)
					{
						flag = 2;
						throw "something wrong::potential object doesn't fit suitable dimension";
					}
					else
					{


						//Find a LED coordinate with the shortest distance to the pic center
						auto it = std::min_element(distance.begin(), distance.end());
						minIndex = std::distance(distance.begin(), it);
						//minvalue = *it;


						if (distance[minIndex] > chipsetting.xpitch[0])
						{
							flag = 6;
							throw "something wrong::potential object doesn't fit suitable dimension";
						}
						else
						{


							crossCenter = center[minIndex] + Point2f(chipsetting.carx, chipsetting.cary);

							std::cout << "check center of chip is :: " << center[minIndex] << endl;

							cv::circle(marksize,
								(Point2i(center[minIndex])), //coordinate
								6, //radius
								Scalar(255, 0, 255),  //color
								FILLED,
								LINE_AA);

							Rect drawrect = Rect(int(crossCenter.x - chipsetting.carx - 0.5 * target.TDwidth),
								int(crossCenter.y - chipsetting.cary - 0.5 * target.TDheight), //rectangle ini y
								Rectlist[minIndex].width, //rectangle width
								Rectlist[minIndex].height); //rectangle height


							//cv::rectangle(marksize, drawrect, cv::Scalar(255, 0, 255), 1, 4);
							cv::line(marksize, Point(0, crossCenter.y), Point(marksize.size[1], crossCenter.y), Scalar(255, 255, 255), 1, 8);
							cv::line(marksize, Point(crossCenter.x, 0), Point(crossCenter.x, marksize.size[0]), Scalar(255, 255, 255), 1, 8);




							int flag_test;

							std::tie(Fourchipsstate, Fourchipsbdbox, Fourchipspt, flag_test) = sectionalCheckFunction(Reqcomthres, Point(crossCenter), drawrect, chipsetting);

							if (flag_test == 10)
							{
								for (int i = 0; i < Fourchipsbdbox.size(); i++)
								{
									rectangle(marksize, Fourchipsbdbox[i], Scalar(130, 0, 255), 15);
								}


							}
							else
							{
								vector<Point> Fdirection = { Point(-1,-1), Point(1,-1), Point(-1,1), Point(1,1) };
								for (int i = 0; i < chipsetting.interval[0] + 1; i++)
								{
									Point drawpt = (Point(crossCenter.x + Fdirection[i].x * chipsetting.interval[0] * chipsetting.interval[1],
										crossCenter.y + Fdirection[i].y * chipsetting.interval[0] * chipsetting.interval[2]));
									cv::circle(marksize, drawpt, 15, Scalar(255, 0, 255), -1);
								}
							}


							flag = flag_test;

							if (imageParm.correctTheta != 0)
							{
								cv::circle(Rotnew,
									(Point2i(crossCenter)), //coordinate
									6, //radius
									Scalar(180, 180, 180),  //color
									FILLED,
									LINE_AA);
								Rotmarkpic = RotatecorrectImg(-1 * imageParm.correctTheta, Rotnew);
								marksize = RotatecorrectImg(-1 * imageParm.correctTheta, marksize);
								cv::inRange(Rotmarkpic, Scalar(175, 175, 175), Scalar(185, 185, 185), thresRot);
								cv::findContours(thresRot, contRot, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());
								Moments Mans = (moments(contRot[0], false));
								crossCenternew = Point2i((Point2f((Mans.m10 / Mans.m00), (Mans.m01 / Mans.m00)))) + IMGoffset;
								std::cout << "check chip crossCenternew is: [ " << crossCenternew << " ]" << endl;
								std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
							}
							else
							{
								crossCenternew = crossCenter + IMGoffset;
								std::cout << "check chip crossCenternew is: [ " << crossCenternew << " ]" << endl;
								std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
							}
						}

					}
				}




			}// if-loop:calculate contL size!=0


		}// if-loop:contH fits suitable area 



	} //try-loop
	catch (const char* message)
	{
		std::cout << message << std::endl;

	}














	//////////////////////////////////////////////////////////output//////////////////////////////////
	auto t_end = std::chrono::high_resolution_clock::now();
	double elapsed_time_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
	std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	std::cout << "check chip center is: [ " << crossCenter << " ]" << endl;
	std::cout << "result flag is :: " << flag << endl;
	std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	std::cout << "calculate color-filter time is:: " << elapsed_time_ms << endl;

	std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;



	return { flag, Reqcomthres, crossCenternew, marksize , Fourchipspt };
}






#pragma endregion chipalgorithm






