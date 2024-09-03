#include "Seccheck4chip_lib_V1.h"


std::tuple<int, Mat, Point, Mat>Uchip_singlephaseDownV2_1(int flag, Mat stIMG, thresP_ thresParm, SettingP_ chipsetting, sizeTD_ target, Point2f creteriaPoint, Point IMGoffset, ImgP_ imageParm)
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
	stIMG.copyTo(marksize);

	/////Step.1-Define Bright area::: 
	Mat Gimg, gauGimh;
	Mat gauBGR;
	/*
	//define grayimg area::
	vector<int> stp, enp;
	Mat defgry = CropIMG(gauGimh, Rect(716,491,136,70));
	std::tie(stp, enp)=UserTuneVal(defgry,0);
	//////////////////////////////////////////
	*/

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



	stIMG.convertTo(stIMG, -1, 1.2, 0);//1.5:pic1 // 1.2:pic5
	cv::GaussianBlur(stIMG, gauBGR, Size(0, 0), 13);//15
	cv::addWeighted(stIMG, 1.5, gauBGR, -0.7, 0.0, stIMG); //(1.5, -0.7) //1.6,-0.7


	cv::cvtColor(stIMG, Gimg, COLOR_RGB2GRAY);
	cv::fastNlMeansDenoising(Gimg, gauGimh, 3, 7, 21);

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


	/*cv::bitwise_not()
	gray.threshold(upper, 255, cv.THRESH_BINARY).bitwiseNot(gray.threshold(lower, 255, cv.THRESH_BINARY))*/
	//cv::threshold(gauGimh, HIGHthres, thresHIGHVal, 255, THRESH_BINARY);

	//comthresIMG.copyTo(Reqcomthres);




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


					if (approxList[minIndex] == 4)
					{
						crossCenter = center[minIndex] + Point2f(chipsetting.carx, chipsetting.cary);
						drawrect = Rect(Rectlist[minIndex].x,
							Rectlist[minIndex].y, //rectangle ini y
							Rectlist[minIndex].width, //rectangle width
							Rectlist[minIndex].height); //rectangle height


					}
					else
					{
						std::cout << "start fine define...." << endl;
						cv::drawContours(finescanIMG, REQcont, minIndex, Scalar(255, 255, 255), -1);

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

					

					flag = 9;


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


	/*Mat Rotmark;
	Rotmark = RotatecorrectImg(-2.6, marksize);*/




	//////////////////////////////////////////////////////////output//////////////////////////////////
	auto t_end = std::chrono::high_resolution_clock::now();
	double elapsed_time_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
	std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	std::cout << "check chip center is: [ " << crossCenter << " ]" << endl;
	std::cout << "result flag is :: " << flag << endl;
	std::cout << "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" << endl;
	std::cout << "calculate color-filter time is:: " << elapsed_time_ms << endl;






	std::cout << "fini" << endl;
	return { flag, Reqcomthres, crossCenternew, marksize };
}




std::tuple<int, Mat, Point, Mat>Uchip_dualphase(int flag, Mat stIMG, thresP_ thresParm, SettingP_ chipsetting, sizeTD_ target, Point2f creteriaPoint, Point IMGoffset, ImgP_ imageParm)
{
	auto t_start = std::chrono::high_resolution_clock::now();

	//output parm:::::
	Mat Rotmarkpic = Mat::ones(stIMG.rows, stIMG.cols, CV_8UC3);
	Mat Rotnew = Mat::ones(stIMG.rows, stIMG.cols, CV_8UC3);
	Mat thresRot;
	Point crossCenternew;

	Mat Reqcomthres = Mat::zeros(stIMG.rows, stIMG.cols, CV_8UC1);
	Point crossCenter;
	Mat marksize;
	stIMG.copyTo(marksize);

	Size S_kermask, S_kernelLOW2, S_Kcomclose;

	if (target.TDheight < target.TDwidth)
	{
		/*S_kermask = Size(10, 1);
		S_kernelLOW2 = Size(1, 7);
		S_Kcomclose = Size(15, 5);*/

		S_kermask = Size(10, 1);
		S_kernelLOW2 = Size(1, 7);
		S_Kcomclose = Size(10, 5);
	}
	else
	{
		/*S_kermask = Size(1, 10);
		S_kernelLOW2 = Size(7, 1);
		S_Kcomclose = Size(5, 15);*/

		S_kermask = Size(1, 10);
		S_kernelLOW2 = Size(7, 1);
		S_Kcomclose = Size(5, 10);
	}


	/////Step.1-Define Bright area::: 
	Mat Gimg, gauGimh;
	cv::cvtColor(stIMG, Gimg, COLOR_BGR2GRAY);
	cv::fastNlMeansDenoising(Gimg, gauGimh, 3, 7, 21);

	//Mat sharpIMG, gauBGR;
	//cv::GaussianBlur(gauGimh, gauBGR, Size(11, 11), 0);
	//gauGimh = (gauGimh - gauBGR) * 8 + gauGimh;//raw-smooth=sharp

	vector<vector<Point>>  contH; // Vector for storing contour
	vector<Vec4i> hierH;
	vector<vector<Point>> reqConH;
	Mat ReqH = Mat::zeros(stIMG.rows, stIMG.cols, CV_8UC1);
	vector<double> AreaHigh;



	Mat LOWimg = Mat::zeros(stIMG.rows, stIMG.cols, CV_8UC1);
	Mat BGRmask, BGRclose;

	/*int thresLowVal = thresParm.fgmin[0];
	int thresHIGHVal = thresParm.fgmax[0];*/


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
							/*std::cout << "check hull is :: " << hull.size() << endl;
							std::cout << "check approx is :: " << approx.size() << endl;
							std::cout << "============================================ "  << endl;*/
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
							/*std::cout << "check hull is :: " << hull.size() << endl;
							std::cout << "check approx is :: " << approx.size() << endl;
							std::cout << "============================================ "  << endl;*/


							contLLow.push_back(contL[i]);

						}

						/*else
						{
							std::cout << "check hull is :: " << hull.size() << endl;
							std::cout << "check approx is :: " << approx.size() << endl;
							std::cout << "============================================ "  << endl;
							contLLow.push_back(contL[i]);
						}*/


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



						/*std::cout << "area is :: " << retCOMP.width * retCOMP.height << endl;
						std::cout << "minarea:: " << 0.5 * sizeParm.TDminW * sizeParm.TDminH<<",, "<< sizeParm.TDmaxW * sizeParm.TDmaxH << endl;*/

						//areacomthres > 0.6 * sizeParm.TDminW * sizeParm.TDminH && areacomthres < sizeParm.TDmaxW * sizeParm.TDmaxH * 1.2

						//if (areacomthres > target.TDwidth*target.TDminW * target.TDheight * target.TDminH
						//	&& areacomthres < target.TDwidth * target.TDmaxW * target.TDheight *target.TDmaxH
						//	&& approx.size()>3 && approx.size() < 14)//<14


						if (retCOMP.width > target.TDwidth * target.TDminW
							&& retCOMP.height > target.TDheight * target.TDminH
							&& retCOMP.width < target.TDwidth * target.TDmaxW
							&& retCOMP.height < target.TDheight * target.TDmaxH)

						{


							//M = (moments(contours[i], false));
							//center.push_back(Point2i(M.m10 / M.m00, M.m01 / M.m00));

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




							//Rect drawrect = Rect(Rectlist[minIndex].x,
							//	Rectlist[minIndex].y, //rectangle ini y
							//	Rectlist[minIndex].width, //rectangle width
							//	Rectlist[minIndex].height); //rectangle height

							Rect drawrect = Rect(int(crossCenter.x - chipsetting.carx - 0.5 * target.TDwidth),
								int(crossCenter.y - chipsetting.cary - 0.5 * target.TDheight), //rectangle ini y
								Rectlist[minIndex].width, //rectangle width
								Rectlist[minIndex].height); //rectangle height


							//cv::rectangle(marksize, drawrect, cv::Scalar(255, 0, 255), 1, 4);
							cv::line(marksize, Point(0, crossCenter.y), Point(marksize.size[1], crossCenter.y), Scalar(255, 255, 255), 1, 8);
							cv::line(marksize, Point(crossCenter.x, 0), Point(crossCenter.x, marksize.size[0]), Scalar(255, 255, 255), 1, 8);




							flag = 9;

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



	return { flag, Reqcomthres, crossCenternew, marksize };
}