#include "stdafx.h"
#include "ciratefiAlgorithm.h"

using namespace std;
using namespace cv;

namespace Ciratefi
{
	void CiratefiData::CountParameter(Mat& templateImage)
	{
		if (_scaleNum>1) _passoesc=exp(log(_finalScale/_initialScale)/_scaleNum); else _passoesc=1.0;
		_AngleDegree=360.0/_angleNum;
		_AngleRadian = _AngleDegree * M_PI / 180.0;
		if (_circleNum>1) _circleDistance=(_finalRadius-_initialRadius)/(_circleNum-1); else _circleDistance=0.0;
		if (_finalRadius<_initialRadius) _finalRadius=scale(_scaleNum-1)*((double)templateImage.rows/2);
		_templateRadius=(double)templateImage.rows/2;
		if (_circleNum>1) _circleDistance=(_finalRadius-_initialRadius)/((double)_circleNum-1); else _circleDistance=0.0;
	}

	double CiratefiData::round(double val, int precision=0)
	{
		double mul=pow(10, (double)precision);
		val*=mul;
		val=(val<0.0)?ceil(val-0.5):floor(val+0.5);
		return val/mul;
	}

	double CiratefiData::CircularSample(Mat& image, int row, int col, int radius)
	{ 
		int row2=0; int col2=radius; double sum=0; double count=0;
		while (col2>0) 
		{
			sum=sum+image.at<uchar>((row+row2<image.rows)?row+row2:image.rows-1, (col+col2<image.cols)?col+col2:image.cols-1)
				+image.at<uchar>((row-row2<0)?0:row-row2, (col-col2<0)?0:col-col2)
				+image.at<uchar>((row+col2<image.rows)?row+col2:image.rows-1, (col-row2<0)?0:col-row2)
				+image.at<uchar>((row-col2<0)?0:row-col2, (col+row2<image.cols)?col+row2:image.cols-1);
			count=count+4;

			int mh=abs((row2+1)*(row2+1)+col2*col2-radius*radius);
			int md=abs((row2+1)*(row2+1)+(col2-1)*(col2-1)-radius*radius);
			int mv=abs(row2*row2+(col2-1)*(col2-1)-radius*radius);
			int m=min(min(mh, md), mv);
			if (m==mh) row2++;
			else if (m==md) { row2++; col2--; }
			else col2--;
		}
		if (count>0)
		{
			return clip((sum+count/2.0)/count, 0.0, 255.0);
		}
		return image.at<uchar>(row,col);
	}

	void CiratefiData::Cisssa(Mat& sourceImage)
	{
		_ca.resize(_circleNum*sourceImage.rows*sourceImage.cols);
		for (int circleNO=_circleNum-1; circleNO>=0; circleNO--) 
		{
			int radius=round(_circleDistance*circleNO+_initialRadius);
			for (int row=0; row<sourceImage.rows; row++)
			{
				for (int col=0; col<sourceImage.cols; col++) 
				{
					_ca[circleNO*sourceImage.rows*sourceImage.cols+row*sourceImage.cols+col]=CircularSample(sourceImage, row, col, radius);
				}
			}
		}
	}
}

