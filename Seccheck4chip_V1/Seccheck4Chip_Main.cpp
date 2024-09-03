


#include "Seccheck4chip_lib_V1.h"



int main()
{
	SettingP_ chipsetting;
	thresP_ thresParm;
	ImgP_ imageParm;
	sizeTD_ target;



	imageParm.imgcols = 5320; 
	imageParm.imgrows = 4600;

	chipsetting.interval[0] = 3; 
	


	imageParm.Outputmode = 0; //0:center coord ; 1: multiple mode
	imageParm.PICmode = 0;  // 0=B or L¡B1=G¡B2=R
	//chipsetting.interval[0] = 0; //2
	chipsetting.xpitch[0] = 250; //400
	chipsetting.carx = 0;
	chipsetting.cary = 0;

	//Tell AOI how many angles should rotate : positive: counterclockwise   /negative:clockwise
	//imageParm.correctTheta = 2.6; //8280402
	imageParm.correctTheta = 0;
	//imageParm.correctTheta = -0.247; //L4_1020
	/////////////////////////////////////////
	Mat rawimg, cropedRImg;
	int picorder;
	Point piccenter;
	Point IMGoffset;
	Point2f creteriaPoint;


	//output parameters::
	Mat ReqIMG, marksize;
	Point crossCenter;
	int boolflag = 0;//11
	

	//operating mode
	int mode = 1;
	if (mode == 1)
	{
		rawimg = imread("C:\\Git\\Vancy4chips\\Pic\\111005.bmp");
		int picorder = 111005;
	
		target.TDmaxW = 1.5;
		target.TDminW = 0.7;

		target.TDmaxH = 1.5;
		target.TDminH = 0.7;

		if (picorder > 24052200 && picorder < 24052299)
		{
			chipsetting.interval[1] = 490; //490
			chipsetting.interval[2] = 273; //273 
			target.TDwidth =300;
			target.TDheight = 153;
			thresParm = { 3,{280,99999,99999},{99999,99999,99999} ,{9,99999,99999}, {99999,99999,99999} };//pic24052202
		}
		else if (picorder > 111000 && picorder < 111099)
		{
			chipsetting.interval[1] = 176;
			chipsetting.interval[2] = 114; 
			target.TDwidth = 133;
			target.TDheight = 71;
			thresParm = { 3,{280,99999,99999},{99999,99999,99999} ,{9,99999,99999}, {99999,99999,99999} };//pic24052202
		}
		else if (picorder > 204008 && picorder < 204015)
		{
			chipsetting.interval[1] = 391;
			chipsetting.interval[2] = 240;
			target.TDwidth = 290;
			target.TDheight = 132;
			thresParm = { 0,{90,99999,99999},{0,99999,99999} ,{255,9,9}, {120,0,0} };//pic2040
		}
		else if (picorder > 204000 && picorder < 204007)
		{
			chipsetting.interval[1] = 228;
			chipsetting.interval[2] = 138;
			target.TDwidth = 164;
			target.TDheight = 84;
			thresParm = { 0,{90,99999,99999},{0,99999,99999} ,{255,9,9}, {120,0,0} };//pic2040
		}


		
		//{mode,bgmax,bgmin,fgmax,fgmin}
		//thresParm = { 0,{90,99999,99999},{0,99999,99999} ,{255,9,9}, {120,0,0} };//2040
		
		


	
		

		if ((chipsetting.interval[0] + 1) * chipsetting.interval[1] > rawimg.cols || (chipsetting.interval[0] + 1) * chipsetting.interval[2] > rawimg.rows)
		{
			boolflag == 7;
		}

		if (boolflag == 0)
		{
			/*Automatically crop image via pitch setting*/
			piccenter = find_piccenter(rawimg);				
			IMGoffset.x = piccenter.x - int((chipsetting.interval[0] + 1) * chipsetting.interval[1]);
			IMGoffset.y = piccenter.y - int((chipsetting.interval[0] + 1) * chipsetting.interval[2]);
			Rect Cregion(IMGoffset.x, IMGoffset.y, int((chipsetting.interval[0] + 1) * chipsetting.interval[1]) * 2, int((chipsetting.interval[0] + 1) * chipsetting.interval[2]) * 2);			
			cropedRImg = CropIMG(rawimg, Cregion);


			/*Resize image to speed up*/
			chipsetting.interval[1] = chipsetting.interval[1]/2; //490
			chipsetting.interval[2] = chipsetting.interval[2]/2; //273 
			target.TDwidth = target.TDwidth/2;
			target.TDheight = target.TDheight / 2;
			cv::resize(cropedRImg, cropedRImg, Size(int(cropedRImg.cols/2), int(cropedRImg.rows / 2)), INTER_LINEAR);

			

			/*Rotate picture::: */
			if (imageParm.correctTheta != 0)
			{
				cropedRImg = RotatecorrectImg(imageParm.correctTheta, cropedRImg);

			}
			/*rotate end----------------*/

			///*///*image without CROP  process :::*/
			//sizeParm.CsizeW = rawimg.size[0];
			//sizeParm.CsizeH = sizeParm.CsizeW;
			//rawimg.copyTo(cropedRImg);


			//start to ISP-negative::
			creteriaPoint = find_piccenter(cropedRImg); //dirt8280312
			
			vector<Point> Fourchipspt;
			
			
			

			//version2.1
			if (thresParm.fgmin[imageParm.PICmode] != 99999 && thresParm.bgmax[imageParm.PICmode] != 99999 && thresParm.thresmode == 0)
			{
				std::cout << "start to dual phase detection...." << endl;
				std::tie(boolflag, ReqIMG, crossCenter, marksize, Fourchipspt) = Uchip_dualphaseV2(boolflag, cropedRImg, thresParm, chipsetting, target, creteriaPoint, IMGoffset, imageParm);

			}

			else
			{
				std::cout << "start to single phase detection...." << endl;

				std::tie(boolflag, ReqIMG, crossCenter, marksize, Fourchipspt) = Uchip_singlephaseDownV3(boolflag, cropedRImg, thresParm, chipsetting, target, creteriaPoint, IMGoffset, imageParm);
			}

			
			cv::resize(marksize, marksize, Size(1200, 800), INTER_LINEAR);
			cv::resize(ReqIMG, ReqIMG, Size(1200, 800), INTER_LINEAR);


		}






		std::cout << "check img state:: " << boolflag << endl;
		std::cout << "check center is ::" << crossCenter << endl;
	}


	rawimg.release();
	cropedRImg.release();

	return 0;
}

























