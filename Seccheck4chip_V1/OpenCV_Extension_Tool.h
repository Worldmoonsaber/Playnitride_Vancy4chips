#pragma once

#include<opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp> //mophorlogical operation
#include<opencv2/core.hpp>
#include<vector>

using namespace cv;
using namespace std;

struct FilterCondition
{
    string FeatureName;
    float  MaximumValue;
    float  MinimumValue;
    bool Enable;
};

class BlobInfo
{
public:
    BlobInfo(vector<Point> vArea, vector<Point> vContour);
    BlobInfo();
    BlobInfo(Mat ImgRegion);
    BlobInfo(vector<Point> vContour);
    BlobInfo(vector<Point> vMainContour,vector<vector<Point>> vHollowContour);

    void CaculateBlob(vector<Point> vArea, vector<Point> vContour);
    void Release();
    int Area();

    /// <summary>
    ///  1 ~ 0.0   1:�������
    /// </summary>
    /// <returns></returns>
    float Circularity();
    Point2f Center();
    /// <summary>
    ///  1 ~ 0.0   1:�����x��
    /// </summary>
    /// <returns></returns>
    float Rectangularity();
    float minRectHeight();
    float minRectWidth();
    float Angle();
    /// <summary>
    /// ���e��
    /// </summary>
    /// <returns></returns>
    float AspectRatio();
    vector<Point> Points();
    vector<Point> contour();

    /// <summary>
    /// ���b����
    /// </summary>
    /// <returns></returns>
    float Ra();

    /// <summary>
    /// �u�b����
    /// </summary>
    /// <returns></returns>
    float Rb();

    int Xmin();
    int Ymin();
    int Xmax();
    int Ymax();
    int Width();
    int Height();
    /// <summary>
    /// ���P��
    /// </summary>
    /// <returns></returns>
    float Bulkiness();

    /// <summary>
    /// ��o��
    /// </summary>
    /// <returns></returns>
    float Compactness();

    /// <summary>
    /// �P Circularity ���w�q�W���t�O �o���ݩʧ�A�X���� ���Ŷ��������� ������ Roundness�ͪ��1 Circularity 0.1 ���k
    /// </summary>
    /// <returns></returns>
    float Roundness();

    float Sides();


    /// <summary>
    /// Topology �~���Ϊ��ݩ�
    /// </summary>
    /// <returns></returns>
    vector<vector<Point>> contourHollow();

    /// <summary>
    /// Topology �~���Ϊ��ݩ�
    /// </summary>
    /// <returns></returns>
    vector<Point> contourMain();

private:

    int _area = -1;
    float _circularity = -1;
    float _rectangularity = -1;

    Point2f _center;

    vector<Point> _points;
    vector<Point> _contour;

    int _XminBound=-1;
    int _YminBound = -1;
    int _XmaxBound = -1;
    int _YmaxBound = -1;
    float _minRectWidth = -1;
    float _minRectHeight = -1;
    float _Angle = -1;
    float _AspectRatio = -1;
    float _Ra = -1;
    float _Rb = -1;
    float _bulkiness = -1;
    float _compactness = -1;
    float _roundness = -1;
    float _sides = -1;
    float _Width = -1;
    float _Height = -1;

    vector<Point> _contourMain;
    vector<vector<Point>> _contourHollow;

};

class BlobFilter
{
public:
    BlobFilter();
    ~BlobFilter();

    map<string, FilterCondition> DictionaryFilterCondition;

    bool IsEnableArea();
    float MaxArea();
    float MinArea();

    bool IsEnableXbound();
    float MaxXbound();
    float MinXbound();

    bool IsEnableYbound();
    float MaxYbound();
    float MinYbound();

    bool IsEnableSubRegion();


    void SetEnableArea(bool enable);
    void SetMaxArea(float value);
    void SetMinArea(float value);

    void SetEnableXbound(bool enable);
    void SetMaxXbound(float value);
    void SetMinXbound(float value);

    void SetEnableYbound(bool enable);
    void SetMaxYbound(float value);
    void SetMinYbound(float value);

    void SetEnableGrayLevel(bool enable);
    void SetMaxGrayLevel(float value);
    void SetMinGrayLevel(float value);

    void SetEnableSubRegion(bool enable);

private:
    map<string, FilterCondition> mapConditions;
    map<string, bool> mapBool;

    void _setMaxPokaYoke(string title, float value);
    void _setMinPokaYoke(string title, float value);
};

/// <summary>
/// 
/// </summary>
/// <param name="ImgBinary"></param>
/// <param name="maxArea">�O�@���I �p�G���ݭn�o��j��Region �i�H�b�o�����α����o�� �קK�O�����n���D����</param>
/// <returns></returns>
vector<BlobInfo> RegionPartition(Mat ImgBinary, int maxArea, int minArea);//; int maxArea = INT_MAX - 2, int minArea = -1);�ثe�����ϥΦY�w�]�Ȧn���|�XBug�קK���~�ϥΥ��]�_��
vector<BlobInfo> RegionPartition(Mat ImgBinary);
vector<BlobInfo> RegionPartition(Mat ImgBinary, BlobFilter filter);

/// <summary>
/// �t�׻P�O����ϥζq���b�i�����d�� ��ĳ�ϥγo�Ӥ�k
/// </summary>
/// <param name="ImgBinary"></param>
/// <returns></returns>
vector<BlobInfo> RegionPartitionTopology(Mat ImgBinary);
