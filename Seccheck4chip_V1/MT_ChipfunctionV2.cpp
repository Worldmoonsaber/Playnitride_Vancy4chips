#include "Seccheck4chip_lib_V1.h"
#include "OpenCV_Extension_Tool.h"


std::tuple<int, Mat, Point, Mat, vector<Point> >Uchip_singlephaseDownV3(int flag, Mat stIMG, thresP_ thresParm, SettingP_ chipsetting, sizeTD_ target, Point2f creteriaPoint, Point IMGoffset, ImgP_ imageParm)
{
	auto t_start = std::chrono::high_resolution_clock::now();

	//output parm:::::
	Point crossCenter;
	Mat marksize;
	Mat Rotmarkpic = Mat::ones(stIMG.rows, stIMG.cols, CV_8UC3);
	Mat Rotnew = Mat::ones(stIMG.rows, stIMG.cols, CV_8UC3);
	Point crossCenternew;
	vector<int> Fourchipsstate;
	vector<Rect> Fourchipsbdbox;
	vector<Point> Fourchipspt;
	



	/////Step.1-Define Bright area::: 
	Mat comthresIMG;
	int adaptWsize = 3;
	int adaptKsize = 2;

	Point2f piccenter;
	stIMG.copyTo(marksize);

	funcThreshold(stIMG, comthresIMG, thresParm, imageParm, target);

	vector<BlobInfo> vChips = RegionPartition(comthresIMG, target.TDwidth *target.TDheight * 1.4, target.TDwidth * target.TDheight * 0.5);

	Rect drawrect;

	piccenter = find_piccenter(comthresIMG);

	try
	{

		if (vChips.size() == 0)
		{
			flag = 1;
			throw "something wrong::threshold value issue";
		}

		else
		{
			vector<vector<cv::Point>> vContour;
			vector<BlobInfo> blobRegionPossible;

			for (int i = 0; i < vChips.size(); i++)
			{
				if (vChips[i].Width() > target.TDwidth * target.TDmaxW)
					continue;

				if (vChips[i].Height() > target.TDheight * target.TDmaxH)
					continue;


				if (vChips[i].Rectangularity() < 0.7)
					continue;


				if (vChips[i].Width() > target.TDwidth * target.TDminW
					&& vChips[i].Height() > target.TDheight * target.TDminH
					&& vChips[i].Width() < target.TDwidth * target.TDmaxW
					&& vChips[i].Height() < target.TDheight * target.TDmaxH
					)
				{
					blobRegionPossible.push_back(vChips[i]);
					cv::rectangle(marksize, Point(vChips[i].Xmin(), vChips[i].Ymin()), Point(vChips[i].Xmax(), vChips[i].Ymax()), Scalar(255, 255, 255), 1);
					cv::rectangle(comthresIMG, Point(vChips[i].Xmin(), vChips[i].Ymin()), Point(vChips[i].Xmax(), vChips[i].Ymax()), Scalar(255, 255, 255), -1);
				}
				else
					vContour.push_back(vChips[i].Points());

			}

			//draw pic center:: 
			cv::circle(marksize,
				Point2i(piccenter), //coordinate
				9, //radius
				Scalar(0, 255, 255),  //color
				FILLED,
				LINE_AA);

			if (blobRegionPossible.size() == 0)
			{
				flag = 2;
				throw "something wrong::potential object doesn't fit suitable dimension";
			}
			else
			{

				std::sort(blobRegionPossible.begin(), blobRegionPossible.end(), [&, piccenter](BlobInfo& a, BlobInfo& b)
					{
						norm(a.Center() - piccenter);
						return norm(a.Center() - piccenter) < norm(b.Center() - piccenter);
					});

				float dist = norm(blobRegionPossible[0].Center() - piccenter);


				if (dist > chipsetting.xpitch[0])
				{
					flag = 6;
					throw "something wrong::potential object doesn't fit suitable dimension";
				}
				else
				{
					drawrect = Rect(blobRegionPossible[0].Xmin(), blobRegionPossible[0].Ymin(), blobRegionPossible[0].Width(), blobRegionPossible[0].Height());
					crossCenter = blobRegionPossible[0].Center() + Point2f(chipsetting.carx, chipsetting.cary);

					cv::circle(marksize,
						(Point2i(crossCenter)), //coordinate
						6, //radius
						Scalar(255, 0, 255),  //color
						FILLED,
						LINE_AA);


					cv::line(marksize, Point(0, crossCenter.y), Point(marksize.size[1], crossCenter.y), Scalar(255, 255, 255), 1, 8);
					cv::line(marksize, Point(crossCenter.x, 0), Point(crossCenter.x, marksize.size[0]), Scalar(255, 255, 255), 1, 8);

					/*check vacancy ::::*/

					int flag_test;
					
					std::cout << "+++++++++++drawrect : " << drawrect << endl;

					std::tie(Fourchipsstate, Fourchipsbdbox, Fourchipspt, flag_test)=sectionalCheckFunction(comthresIMG, Point(crossCenter), drawrect, chipsetting);

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
						vector<Point> vPt;
						vPt.push_back(crossCenter);

						vector<Point> vPtOut;
						funcRotatePoint(vPt, vPtOut, marksize, imageParm.correctTheta, IMGoffset);

						if (vPtOut.size() > 0)
							crossCenternew = vPtOut[0];

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

	stIMG.release();
	Rotnew.release();
	Rotmarkpic.release();

	std::cout << "fini" << endl;
	return { flag, comthresIMG, crossCenternew, marksize,Fourchipspt };
}
