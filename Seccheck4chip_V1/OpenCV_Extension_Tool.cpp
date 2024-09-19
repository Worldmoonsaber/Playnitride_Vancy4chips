
#include "OpenCV_Extension_Tool.h"

#pragma region Region �аO �\�� ������k

//void RegionFloodFill(uchar* ptr, int x, int y, vector<Point>& vectorPoint, vector<Point>& vContour, int maxArea, bool& isOverSizeExtension,int width,int height)
//{
//	if (maxArea < vectorPoint.size())
//		return;
//
//	uchar tagOverSize = 10;
//	uchar tagIdx = 101;
//
//	if (ptr[x + width * y] == 255)
//	{
//		ptr[x + width * y] = tagIdx;
//		vectorPoint.push_back(Point(x, y));
//	}
//	else if (ptr[x + width * y] == tagIdx)
//		return;
//
//	uchar edgesSides = 0; //���C���ڪŶ�
//
//#pragma region ��@Region ���j�� �e���X�{��ư�n���~ �M���ݩʭn�վ�
//
//	for (int j = y - 1; j <= y + 1; j++)
//		for (int i = x - 1; i <= x + 1; i++)
//		{
//			if (i == x && y == j)
//				continue;
//
//			if (i < 0 || j < 0)
//			{
//				edgesSides++;
//				continue;
//			}
//
//			if (i >= width || j >= height)
//			{
//				edgesSides++;
//				continue;
//			}
//
//			if (ptr[i + width * j] == 0)
//			{
//				edgesSides++;
//				continue;
//			}
//
//			if (ptr[i + width * j] == 255)
//				RegionFloodFill(ptr, i, j, vectorPoint, vContour, maxArea, isOverSizeExtension, width, height);
//			else if (ptr[i + width * j] == tagOverSize)
//				isOverSizeExtension = true;
//			else if (ptr[i + width * j] == tagIdx)
//				continue;
//
//		}
//
//#pragma endregion
//
//	if (edgesSides > 1 && edgesSides < 8)
//		vContour.push_back(Point(x, y));
//}
//
//void RegionPaint(uchar* ptr, vector<Point> vPoint, uchar PaintIdx,int imgWidth)
//{
//	//<---���B�[�J����B��ݭn�B�~���禡�w, UI�ݹq�����Ҥ���,������@�������e
//	for (int i = 0; i < vPoint.size(); i++)
//		ptr[vPoint[i].y * imgWidth + vPoint[i].x] = PaintIdx;
//}

#pragma endregion

#pragma region BlobInfo ����
BlobInfo::BlobInfo(vector<Point> vArea, vector<Point> vContour)
{
	CaculateBlob(vArea, vContour);
}

BlobInfo::BlobInfo()
{
}

BlobInfo::BlobInfo(Mat ImgRegion)
{
	vector<vector<Point>> vContourArr;
	findContours(ImgRegion, vContourArr, RETR_LIST, CHAIN_APPROX_NONE);
	vector<Point> vContour;
	
	for (int i = 0; i < vContourArr.size(); i++)
		vContour.insert(vContour.end(), vContourArr[i].begin(), vContourArr[i].end());

	vector<Point> vArea;
	findNonZero(ImgRegion, vArea);
	CaculateBlob(vArea, vContour);

	ImgRegion.release();
}

BlobInfo::BlobInfo(vector<Point> vContour)
{
	_contourMain = vContour;
	_contourHollow.clear();
	//retCOMP = cv::boundingRect(contH[i]);
	double areacomthres = cv::contourArea(vContour);

	_contour = vContour;
	//_points = vArea;
	_area = areacomthres;

	Moments M = (moments(vContour, false));
	_center = Point2f((M.m10 / M.m00), (M.m01 / M.m00));

	_XminBound = 99999;
	_YminBound = 99999;
	_XmaxBound = -1;
	_YmaxBound = -1;

	for (int i = 0; i < vContour.size(); i++)
	{
		if (vContour[i].x > _XmaxBound)
			_XmaxBound = vContour[i].x;

		if (vContour[i].y > _YmaxBound)
			_YmaxBound = vContour[i].y;

		if (vContour[i].x < _XminBound)
			_XminBound = vContour[i].x;

		if (vContour[i].y < _YminBound)
			_YminBound = vContour[i].y;

	}

	_Width = _XmaxBound - _XminBound + 1;
	_Height = _YmaxBound - _YminBound + 1;


	float min_len = _area;
	float max_len = 0;

	for (int j = 0; j < vContour.size(); j++)
	{
		float len = norm(_center - (Point2f)vContour[j]);

		if (len > max_len)
			max_len = len;

		if (len < min_len)
			min_len = len;
	}

	_circularity = _area / (max_len * max_len * CV_PI);

	if (vContour.size() == 0)
		return;

	RotatedRect minrect = minAreaRect(vContour);
	_Angle = minrect.angle;

	while (_Angle < 0 && _Angle>180)
	{
		if (_Angle < 0)
			_Angle += 180;

		if (_Angle > 180)
			_Angle -= 180;
	}

	float minArea = (minrect.size.height + 1) * (minrect.size.width + 1);//���X���G���O���Y�� �ҥH�n+1
	_minRectHeight = minrect.size.height + 1;
	_minRectWidth = minrect.size.width + 1;

	if (_minRectHeight > _minRectWidth)
	{
		_AspectRatio = _minRectHeight / _minRectWidth;
		_Ra = _minRectHeight;
		_Rb = _minRectWidth;
	}
	else
	{
		_AspectRatio = _minRectWidth / _minRectHeight;
		_Rb = _minRectHeight;
		_Ra = _minRectWidth;
	}

	_bulkiness = CV_PI * _Ra / 2 * _Rb / 2 / _area * 1.0;

	if (minArea < _area)
		_rectangularity = minArea / _area;
	else
		_rectangularity = _area / minArea;

	_rectangularity = abs(_rectangularity);
	_compactness = (1.0 * _contour.size()) * (1.0 * _contour.size()) / (4.0 * CV_PI * _area);

	// _compactness ����
	//
	// 
	//  _compactness= (�P��)^2/ (4*PI*���n)
	//
	//  �i�H����X �z�פW �ؼЪ��� _compactness �ƭ� �b�Φ��ƭȶi��L�o
	//
	//

	// _roundness;
	// 

	if (_contour.size() > 0)
	{
		float distance = 0;

		for (int i = 0; i < _contour.size(); i++)
		{
			float d = norm(_center - (Point2f)_contour[i]);
			distance += d;
		}

		distance /= _contour.size();
		float sigma;
		float diff = 0;

		for (int i = 0; i < _contour.size(); i++)
		{
			float d = norm(_center - (Point2f)_contour[i]);
			diff += (d - distance) * (d - distance);
		}

		diff = sqrt(diff);
		sigma = diff / sqrt(_contour.size() * 1.0);
		_roundness = 1 - sigma / distance;
		_sides = (float)1.411 * pow((distance / sigma), (0.4724));
	}

	// Moments openCV�w�g�s�b��@ �S�����n�[�J�����S�x ���ݭn�b�I�s�Y�i

}

BlobInfo::BlobInfo(vector<Point> vMainContour, vector<vector<Point>> vHollowContour)
{
	_contourMain = vMainContour;
	_contourHollow = vHollowContour;

	_area = cv::contourArea(vMainContour);
	Moments M = (moments(vMainContour, false));
	_center = Point2f((M.m10 / M.m00)*_area, (M.m01 / M.m00) * _area);

	for (int i = 0; i < vHollowContour.size(); i++)
	{
		M = (moments(vHollowContour[i], false));
		int sub_Area= contourArea(vHollowContour[i]);
		_center-= Point2f((M.m10 / M.m00) * sub_Area, (M.m01 / M.m00) * sub_Area);
		_area -= sub_Area;
	}

	_center /= _area;

	RotatedRect minrect = minAreaRect(vMainContour);
	_Angle = minrect.angle;

	while (_Angle < 0 && _Angle>180)
	{
		if (_Angle < 0)
			_Angle += 180;

		if (_Angle > 180)
			_Angle -= 180;
	}

	float minArea = (minrect.size.height + 1) * (minrect.size.width + 1);//���X���G���O���Y�� �ҥH�n+1
	_minRectHeight = minrect.size.height + 1;
	_minRectWidth = minrect.size.width + 1;

	if (_minRectHeight > _minRectWidth)
	{
		_AspectRatio = _minRectHeight / _minRectWidth;
		_Ra = _minRectHeight;
		_Rb = _minRectWidth;
	}
	else
	{
		_AspectRatio = _minRectWidth / _minRectHeight;
		_Rb = _minRectHeight;
		_Ra = _minRectWidth;
	}

	_XminBound = 99999;
	_YminBound = 99999;
	_XmaxBound = -1;
	_YmaxBound = -1;

	for (int i = 0; i < vMainContour.size(); i++)
	{
		if (vMainContour[i].x > _XmaxBound)
			_XmaxBound = vMainContour[i].x;

		if (vMainContour[i].y > _YmaxBound)
			_YmaxBound = vMainContour[i].y;

		if (vMainContour[i].x < _XminBound)
			_XminBound = vMainContour[i].x;

		if (vMainContour[i].y < _YminBound)
			_YminBound = vMainContour[i].y;
	}

	_Width = _XmaxBound - _XminBound + 1;
	_Height = _YmaxBound - _YminBound + 1;

	_contour.clear();

	_contour.insert(_contour.end(), vMainContour.begin(), vMainContour.end());

	for (size_t i = 0; i < vHollowContour.size(); i++)
		_contour.insert(_contour.end(), vHollowContour[i].begin(), vHollowContour[i].end());

	vector<vector<Point>> vPoint;

	vPoint.push_back(vMainContour);

	for (int i = 0; i < vHollowContour.size(); i++)
		vPoint.push_back(vHollowContour[i]);

	float min_len = _area;
	float max_len = 0;

	for (int j = 0; j < _contour.size(); j++)
	{
		float len = norm(_center - (Point2f)_contour[j]);

		if (len > max_len)
			max_len = len;

		if (len < min_len)
			min_len = len;
	}

	_circularity = _area / (max_len * max_len * CV_PI);

	if (_contour.size() == 0)
		return;

	_bulkiness = CV_PI * _Ra / 2 * _Rb / 2 / _area * 1.0;

	if (minArea < _area)
		_rectangularity = minArea / _area;
	else
		_rectangularity = _area / minArea;

	_rectangularity = abs(_rectangularity);
	_compactness = (1.0 * _contour.size()) * (1.0 * _contour.size()) / (4.0 * CV_PI * _area);

	if (_contour.size() > 0)
	{
		float distance = 0;

		for (int i = 0; i < _contour.size(); i++)
		{
			float d = norm(_center - (Point2f)_contour[i]);
			distance += d;
		}

		distance /= _contour.size();
		float sigma;
		float diff = 0;

		for (int i = 0; i < _contour.size(); i++)
		{
			float d = norm(_center - (Point2f)_contour[i]);
			diff += (d - distance) * (d - distance);
		}

		diff = sqrt(diff);
		sigma = diff / sqrt(_contour.size() * 1.0);
		_roundness = 1 - sigma / distance;
		_sides = (float)1.411 * pow((distance / sigma), (0.4724));
	}


}

void BlobInfo::CaculateBlob(vector<Point> vArea, vector<Point> vContour)
{
	_contour = vContour;
	_points = vArea;
	_area = vArea.size();

	float x_sum = 0, y_sum = 0;

	_XminBound = 99999;
	_YminBound = 99999;
	_XmaxBound = -1;
	_YmaxBound = -1;

	for (int i = 0; i < vArea.size(); i++)
	{
		x_sum += vArea[i].x;
		y_sum += vArea[i].y;

		if (vArea[i].x > _XmaxBound)
			_XmaxBound = vArea[i].x;

		if (vArea[i].y > _YmaxBound)
			_YmaxBound = vArea[i].y;

		if (vArea[i].x < _XminBound)
			_XminBound = vArea[i].x;

		if (vArea[i].y < _YminBound)
			_YminBound = vArea[i].y;

	}

	_Width = _XmaxBound - _XminBound + 1;
	_Height = _YmaxBound - _YminBound + 1;
	_center = Point2f(x_sum / vArea.size(), y_sum / vArea.size());

	float min_len = _area;
	float max_len = 0;

	for (int j = 0; j < vContour.size(); j++)
	{
		float len = norm(_center - (Point2f)vContour[j]);

		if (len > max_len)
			max_len = len;

		if (len < min_len)
			min_len = len;
	}

	_circularity = _area / (max_len * max_len * CV_PI);

	if (vContour.size() == 0)
		return;

	RotatedRect minrect = minAreaRect(vContour);
	_Angle = minrect.angle;

	while (_Angle < 0 && _Angle>180)
	{
		if (_Angle < 0)
			_Angle += 180;

		if (_Angle > 180)
			_Angle -= 180;
	}

	float minArea = (minrect.size.height + 1) * (minrect.size.width + 1);//���X���G���O���Y�� �ҥH�n+1
	_minRectHeight = minrect.size.height + 1;
	_minRectWidth = minrect.size.width + 1;

	if (_minRectHeight > _minRectWidth)
	{
		_AspectRatio = _minRectHeight / _minRectWidth;
		_Ra = _minRectHeight;
		_Rb = _minRectWidth;
	}
	else
	{
		_AspectRatio = _minRectWidth / _minRectHeight;
		_Rb = _minRectHeight;
		_Ra = _minRectWidth;
	}

	_bulkiness = CV_PI * _Ra / 2 * _Rb / 2 / _area * 1.0;

	if (minArea < _area)
		_rectangularity = minArea / _area;
	else
		_rectangularity = _area / minArea;

	_rectangularity = abs(_rectangularity);
	_compactness = (1.0 * _contour.size()) * (1.0 * _contour.size()) / (4.0 * CV_PI * _area);

	// _compactness ����
	//
	// 
	//  _compactness= (�P��)^2/ (4*PI*���n)
	//
	//  �i�H����X �z�פW �ؼЪ��� _compactness �ƭ� �b�Φ��ƭȶi��L�o
	//
	//

	// _roundness;
	// 

	if (_contour.size() > 0)
	{
		float distance = 0;

		for (int i = 0; i < _contour.size(); i++)
		{
			float d = norm(_center - (Point2f)_contour[i]);
			distance += d;
		}

		distance /= _contour.size();
		float sigma;
		float diff = 0;

		for (int i = 0; i < _contour.size(); i++)
		{
			float d = norm(_center - (Point2f)_contour[i]);
			diff += (d - distance) * (d - distance);
		}

		diff = sqrt(diff);
		sigma = diff / sqrt(_contour.size() * 1.0);
		_roundness = 1 - sigma / distance;
		_sides = (float)1.411 * pow((distance / sigma), (0.4724));
	}

	// Moments openCV�w�g�s�b��@ �S�����n�[�J�����S�x ���ݭn�b�I�s�Y�i
}

void BlobInfo::Release()
{
	_contour.clear();
	_points.clear();
	_area = -1;
	_circularity = -1;
	_rectangularity = -1;
	_XminBound = -1;
	_YminBound = -1;
	_XmaxBound = -1;
	_YmaxBound = -1;
	_minRectWidth = -1;
	_minRectHeight = -1;
	_Angle = -1;
	_AspectRatio = -1;
	_Ra = -1;
	_Rb = -1;
	_bulkiness = -1;
	_compactness = -1;
	_roundness = -1;
	_sides = -1;
}

int BlobInfo::Area()
{
	return _area;
}

float BlobInfo::Circularity()
{
	return _circularity;
}

Point2f BlobInfo::Center()
{
	return _center;
}

float BlobInfo::Rectangularity()
{
	return _rectangularity;
}

float BlobInfo::minRectHeight()
{
	return _minRectHeight;
}

float BlobInfo::minRectWidth()
{
	return _minRectWidth;
}

float BlobInfo::Angle()
{
	return _Angle;
}

float BlobInfo::AspectRatio()
{
	return _AspectRatio;
}

vector<Point> BlobInfo::Points()
{
	return _points;
}

vector<Point> BlobInfo::contour()
{
	return _contour;
}

/// <summary>
/// ���b
/// </summary>
/// <returns></returns>
float BlobInfo::Ra()
{
	return _Ra;
}

/// <summary>
/// �u�b
/// </summary>
/// <returns></returns>
float BlobInfo::Rb()
{
	return _Rb;
}

int BlobInfo::Xmin()
{
	return _XminBound;
}

int BlobInfo::Ymin()
{
	return _YminBound;
}

int BlobInfo::Xmax()
{
	return _XmaxBound;
}

int BlobInfo::Ymax()
{
	return _YmaxBound;
}

int BlobInfo::Width()
{
	return _Width;
}

int BlobInfo::Height()
{
	return _Height;
}

float BlobInfo::Bulkiness()
{
	return _bulkiness;
}

float BlobInfo::Compactness()
{
	return _compactness;
}

float BlobInfo::Roundness()
{
	return _roundness;
}

float BlobInfo::Sides()
{
	return _sides;
}

vector<vector<Point>> BlobInfo::contourHollow()
{
	return _contourHollow;
}

vector<Point> BlobInfo::contourMain()
{
	return _contourMain;
}

#pragma endregion

#pragma region BlobFilter ����

BlobFilter::BlobFilter()
{
	FilterCondition condition1;
	condition1.FeatureName = "area";
	condition1.Enable = false;
	condition1.MaximumValue = INT16_MAX;
	condition1.MinimumValue = INT16_MIN;

	FilterCondition condition2;
	condition2.FeatureName = "xBound";
	condition2.Enable = false;
	condition2.MaximumValue = INT16_MAX;
	condition2.MinimumValue = INT16_MIN;

	FilterCondition condition3;
	condition3.FeatureName = "yBound";
	condition3.Enable = false;
	condition3.MaximumValue = INT16_MAX;
	condition3.MinimumValue = INT16_MIN;

	FilterCondition condition4;
	condition4.FeatureName = "grayLevel";
	condition4.Enable = false;
	condition4.MaximumValue = 255;
	condition4.MinimumValue = 0;



	mapConditions.insert(std::pair<string, FilterCondition>(condition1.FeatureName, condition1));
	mapConditions.insert(std::pair<string, FilterCondition>(condition2.FeatureName, condition2));
	mapConditions.insert(std::pair<string, FilterCondition>(condition3.FeatureName, condition3));
	mapBool.insert(std::pair<string, bool>("SubRegion", true));
}

BlobFilter::~BlobFilter()
{
	mapConditions.clear();
}

void BlobFilter::_setMaxPokaYoke(string title, float value)
{
	if (mapConditions[title].MinimumValue < value)
		mapConditions[title].MaximumValue = value;
	else
		mapConditions[title].MaximumValue = mapConditions[title].MinimumValue;
}

void BlobFilter::_setMinPokaYoke(string title, float value)
{
	if (mapConditions[title].MaximumValue > value)
		mapConditions[title].MinimumValue = value;
	else
		mapConditions[title].MinimumValue = mapConditions[title].MaximumValue;
}

bool BlobFilter::IsEnableArea()
{
	return mapConditions["area"].Enable;
}

float BlobFilter::MaxArea()
{
	return mapConditions["area"].MaximumValue;
}

float BlobFilter::MinArea()
{
	return mapConditions["area"].MinimumValue;
}

bool BlobFilter::IsEnableXbound()
{
	return mapConditions["xBound"].Enable;
}

float BlobFilter::MaxXbound()
{
	return mapConditions["xBound"].MaximumValue;
}

float BlobFilter::MinXbound()
{
	return mapConditions["xBound"].MinimumValue;
}

bool BlobFilter::IsEnableYbound()
{
	return mapConditions["yBound"].Enable;
}

float BlobFilter::MaxYbound()
{
	return mapConditions["yBound"].MaximumValue;
}

float BlobFilter::MinYbound()
{
	return mapConditions["yBound"].MinimumValue;
}

bool BlobFilter::IsEnableSubRegion()
{
	return mapBool["SubRegion"];
}

void BlobFilter::SetEnableArea(bool enable)
{
	mapConditions["area"].Enable = enable;
}

void BlobFilter::SetMaxArea(float value)
{
	_setMaxPokaYoke("area", value);
}

void BlobFilter::SetMinArea(float value)
{
	_setMinPokaYoke("area", value);
}

void BlobFilter::SetEnableXbound(bool enable)
{
	mapConditions["xBound"].Enable = enable;
}

void BlobFilter::SetMaxXbound(float value)
{
	_setMaxPokaYoke("xBound", value);
}

void BlobFilter::SetMinXbound(float value)
{
	_setMinPokaYoke("xBound", value);
}

void BlobFilter::SetEnableYbound(bool enable)
{
	mapConditions["yBound"].Enable = enable;
}

void BlobFilter::SetMaxYbound(float value)
{
	_setMaxPokaYoke("yBound", value);
}

void BlobFilter::SetMinYbound(float value)
{
	_setMinPokaYoke("yBound", value);
}

void BlobFilter::SetEnableGrayLevel(bool enable)
{
	mapConditions["grayLevel"].Enable = enable;
}

void BlobFilter::SetMaxGrayLevel(float value)
{
	if (value >= 0 && value <= 255 && value > mapConditions["grayLevel"].MinimumValue)
		mapConditions["grayLevel"].MaximumValue = (int)value;
	else
		mapConditions["grayLevel"].MaximumValue = mapConditions["grayLevel"].MinimumValue;
}

void BlobFilter::SetMinGrayLevel(float value)
{
	if (value >= 0 && value <= 255 && value < mapConditions["grayLevel"].MaximumValue)
		mapConditions["grayLevel"].MinimumValue = (int)value;
	else
		mapConditions["grayLevel"].MinimumValue = mapConditions["grayLevel"].MaximumValue;
}

void BlobFilter::SetEnableSubRegion(bool enable)
{
	mapBool["SubRegion"] = enable;
}

#pragma endregion

#pragma region BlobInfoThreadObject ����

/// <summary>
/// �Ω�h���B�z BlobInfo���� ���ɮĲv��
/// </summary>
class BlobInfoThreadObject
{
public:
	BlobInfoThreadObject();
	void Initialize();
	void AddObject(vector<Point> vArea, vector<Point> vContour);
	void WaitWorkDone();
	vector<BlobInfo> GetObj();
	~BlobInfoThreadObject();
private:
	static void thread_WorkContent(queue <tuple<vector<Point>, vector<Point>>>* queue, bool* _isFinished, vector<BlobInfo>* vResult, mutex* mutex);
	thread thread_Work;
	queue <tuple<vector<Point>, vector<Point>>> _QueueObj;
	vector<BlobInfo> _result;
	bool _isProcessWaitToFinished = false;
	mutex mu;
};

BlobInfoThreadObject::BlobInfoThreadObject()
{
	_isProcessWaitToFinished = false;
}

void BlobInfoThreadObject::Initialize()
{
	thread_Work = thread(BlobInfoThreadObject::thread_WorkContent,&_QueueObj,&_isProcessWaitToFinished,&_result,&mu);
}

void BlobInfoThreadObject::AddObject(vector<Point> vArea, vector<Point> vContour)
{
	mu.lock();
	_QueueObj.push(tuple<vector<Point>, vector<Point>>(vArea, vContour));
	mu.unlock();
}

void BlobInfoThreadObject::WaitWorkDone()
{
	_isProcessWaitToFinished = true;
	thread_Work.join();
}

vector<BlobInfo> BlobInfoThreadObject::GetObj()
{
	return _result;
}

BlobInfoThreadObject::~BlobInfoThreadObject()
{
	thread_Work.~thread();
}

void BlobInfoThreadObject::thread_WorkContent(queue <tuple<vector<Point>, vector<Point>>>* queue, bool* _isFinished, vector<BlobInfo>* vResult,mutex* mutex)
{
	while (true)
	{
		if (queue->size() != 0)
		{
			tuple<vector<Point>, vector<Point>> obj;

			mutex->lock();
			obj = queue->front();//���o���
			queue->pop();//�N��Ʊq_QueueObj���M��		
			mutex->unlock();

			vector<Point> vArea = std::get<0>(obj);
			vector<Point> vContour = std::get<1>(obj);
			BlobInfo regionInfo = BlobInfo(vArea, vContour);
			vResult->push_back(regionInfo);
		}

		if (_isFinished[0] == true && queue->size() == 0)
			break;//�u�@����
	}
}

#pragma endregion

//�Q�^�O���¤�k �t�ק�� ���O�����í�w
//vector<BlobInfo> RegionPartition(Mat ImgBinary,int maxArea, int minArea)
//{
//	vector<BlobInfo> lst;
//	lst.reserve(100);
//	uchar tagOverSize = 10;
//	Mat ImgTag = ImgBinary.clone();
//
//	uchar* _ptr = (uchar*)ImgTag.data;
//	int ww = ImgBinary.cols;
//	int hh = ImgBinary.rows;
//
//	BlobInfoThreadObject blobInfoThread;
//	blobInfoThread.Initialize();
//
//	for (int i = 0; i < ww; i++)
//		for (int j = 0; j < hh; j++)
//		{
//			uchar val = _ptr[ww * j + i];
//			bool isOverSizeExtension = false;
//
//			if (val == 255)
//			{
//				vector<Point> vArea;
//				vector<Point> vContour;
//				RegionFloodFill(_ptr, i, j, vArea, vContour, maxArea, isOverSizeExtension,ww,hh);
//
//				if (vArea.size() > maxArea || isOverSizeExtension)
//				{
//					RegionPaint(_ptr, vArea, tagOverSize, ww);
//					continue;
//				}
//				else if (vArea.size() <= minArea)
//				{
//					RegionPaint(_ptr, vArea, 0, ww);
//					continue;
//				}
//				blobInfoThread.AddObject(vArea, vContour);
//				RegionPaint(_ptr, vArea, 0, ww);
//			}
//		}
//
//	blobInfoThread.WaitWorkDone();
//	lst = blobInfoThread.GetObj();
//
//	ImgTag.release();
//	ImgBinary.release();
//	return lst;
//
//}

//vector<BlobInfo> RegionPartition(Mat ImgBinary)
//{
//	return RegionPartition(ImgBinary, INT16_MAX, 0);
//}

/// <summary>
/// 
/// </summary>
/// <param name="ImgBinary">������J�G�ȤƼv��</param>
/// <param name="filter">�w���L�o���󤧫�Q��|�̷ӻݨD����W�[</param>
/// <returns></returns>
//vector<BlobInfo> RegionPartition(Mat ImgBinary, BlobFilter filter)
//{
//	float maxArea = INT_MAX-2;
//	float minArea = 0;
//
//	bool isEnable = filter.IsEnableArea();
//	if (filter.IsEnableArea())
//	{
//		maxArea = filter.MaxArea();
//		minArea = filter.MinArea();
//	}
//
//	float Xmin = 0;
//	float Xmax = ImgBinary.cols;
//
//	if (filter.IsEnableXbound())
//	{
//		Xmax = filter.MaxXbound();
//		Xmin = filter.MinXbound();
//	}
//
//	float Ymin = 0;
//	float Ymax = ImgBinary.rows;
//
//	if (filter.IsEnableYbound())
//	{
//		Ymax = filter.MaxYbound();
//		Ymin = filter.MinYbound();
//	}
//
//	vector<BlobInfo> lst;
//	uchar tagOverSize = 10;
//
//	Mat ImgTag = ImgBinary.clone();
//
//	uchar* _ptr = (uchar*)ImgTag.data;
//	int ww = ImgBinary.cols;
//	int hh = ImgBinary.rows;
//
//	BlobInfoThreadObject blobInfoThread;
//	blobInfoThread.Initialize();
//
//	for (int i = Xmin; i < Xmax; i++)
//		for (int j = Ymin; j < Ymax; j++)
//		{
//			uchar val = _ptr[ww * j + i];
//			bool isOverSizeExtension = false;
//
//			if (val == 255)
//			{
//				vector<Point> vArea;
//				vector<Point> vContour;
//				RegionFloodFill(_ptr, i, j, vArea, vContour, maxArea, isOverSizeExtension, ww, hh);
//
//				if (vArea.size() > maxArea || isOverSizeExtension)
//				{
//					RegionPaint(_ptr, vArea, tagOverSize, ww);
//					continue;
//				}
//				else if (vArea.size() <= minArea)
//				{
//					RegionPaint(_ptr, vArea, 0, ww);
//					continue;
//				}
//				blobInfoThread.AddObject(vArea, vContour);
//				RegionPaint(_ptr, vArea, 0, ww);
//			}
//		}
//
//	blobInfoThread.WaitWorkDone();
//	lst = blobInfoThread.GetObj();
//
//	ImgTag.release();
//	ImgBinary.release();
//	return lst;
//}

void RegionPartitionTopologySubLayerAnalysis(int layer,int curIndex, vector<vector<Point>> vContour, vector<Vec4i> vhi,vector<BlobInfo>& lstBlob)
{
	int type = layer % 2;
	//--- 0 ���h��Region
	//--- 1 ���h�����Ű�

	if (type == 0)
	{
		//----�S���l���h
		vector<Point> mainContour = vContour[curIndex];
		vector<vector<Point>> vHollowContour;

		//---�R���l���h
		int idx = vhi[curIndex].val[2];
		vector<int> subIndx;

		if (idx != -1)
		{
			while (true)
			{
				vHollowContour.push_back(vContour[idx]);

				if (vhi[idx].val[2] != -1)
					subIndx.push_back(vhi[idx].val[2]);//--��Region
				if (vhi[idx].val[0] == -1)
					break;

				idx = vhi[idx].val[0];
			}
		}

		BlobInfo blob = BlobInfo(mainContour, vHollowContour);
		lstBlob.push_back(blob);

		for (int i = 0; i < subIndx.size(); i++)
			RegionPartitionTopologySubLayerAnalysis(layer + 1, subIndx[i], vContour, vhi, lstBlob);
	}
	else
	{
		//---���Űϰ� �[��O�_�s�b �l�ϰ�

		int idx = vhi[curIndex].val[2];
		vector<int> subIndx;

		if (idx != -1)
		{
			while (true)
			{
				if (vhi[idx].val[2] != -1)
					subIndx.push_back(vhi[idx].val[2]);

				if (vhi[idx].val[0] == -1)
					break;

				idx = vhi[idx].val[0];
			}
		}

		for (int i = 0; i < subIndx.size(); i++)
			RegionPartitionTopologySubLayerAnalysis(layer + 1, subIndx[i], vContour, vhi, lstBlob);

	}
}

/// <summary>
///  ������G����C (�z�פW���ӭn�����) ����M
/// </summary>
/// <param name="ImgBinary"></param>
/// <param name="filter"></param>
/// <returns></returns>
vector<BlobInfo> RegionPartitionTopology(Mat ImgBinary)
{
	vector<BlobInfo> vRes;
	//https://blog.csdn.net/qinglingLS/article/details/106270095
	// �ǳƥΩݾ몺�覡���c��k

	vector<vector<Point>> vContour;
	vector<Vec4i> vhi;
	//
	//  [�U�@��,�W�@��,�l�h,���h]
	//
	findContours(ImgBinary, vContour, vhi, RETR_CCOMP, CHAIN_APPROX_NONE);

	int layer = 0;
	int i = 0;

	if (vhi.size() > 0)
	{
		while (true)
		{
			if (vhi[i].val[2] == -1)
			{
				//----�S���l���h
				BlobInfo obj = BlobInfo(vContour[i]);
				vRes.push_back(obj);
			}
			else
			{
				//----�����h �ݦ����|�}�ϰ�
				RegionPartitionTopologySubLayerAnalysis(0, i, vContour, vhi, vRes);
			}

			if (vhi[i].val[0] == -1)
				break;

			i = vhi[i].val[0];
		}
	}
	return vRes;
}

//void thread_Content(Mat* img, int maxArea, int minArea, int starY, int endY , vector<BlobInfo>* vResult, vector<BlobInfo>* vEdge)
//{
//	//vector<BlobInfo> lst;
//	uchar tagOverSize = 10;
//	Mat ImgTag = img->clone();
//
//	uchar* _ptr = (uchar*)ImgTag.data;
//	int ww = img->cols;
//	int hh = img->rows;
//
//	BlobInfoThreadObject blobInfoThread;
//	blobInfoThread.Initialize();
//
//	for (int i = 0; i < ww; i++)
//		for (int j = starY; j < endY; j++)
//		{
//			uchar val = _ptr[ww * j + i];
//			bool isOverSizeExtension = false;
//
//			if (val == 255)
//			{
//				vector<Point> vArea;
//				vector<Point> vContour;
//				RegionFloodFill(_ptr, i, j, vArea, vContour, maxArea, isOverSizeExtension, ww, hh);
//
//				if (vArea.size() > maxArea || isOverSizeExtension)
//				{
//					RegionPaint(_ptr, vArea, tagOverSize, ww);
//					continue;
//				}
//				else if (vArea.size() <= minArea)
//				{
//					RegionPaint(_ptr, vArea, 0, ww);
//					continue;
//				}
//				blobInfoThread.AddObject(vArea, vContour);
//				RegionPaint(_ptr, vArea, 0, ww);
//			}
//		}
//
//	blobInfoThread.WaitWorkDone();
//	vResult[0] = blobInfoThread.GetObj();
//
//}


vector<BlobInfo> RegionPartition(Mat ImgBinary, int maxArea, int minArea)
{
	vector<BlobInfo> lst = RegionPartitionTopology(ImgBinary);
	vector<BlobInfo> lstOut;

	for (int i = 0; i < lst.size(); i++)
	{
		if (lst[i].Area() > maxArea)
			continue;

		if (lst[i].Area() < minArea)
			continue;

		lstOut.push_back(lst[i]);
	}

	ImgBinary.release();
	return lstOut;
}
