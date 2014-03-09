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

	Point CiratefiData::ValidImageRange(Point& position, cv::Mat& image)
	{
		position.x=(position.x>=image.rows)?image.rows-1:((position.x<0)?0:position.x);
		position.y=(position.y>=image.cols)?image.cols-1:((position.y<0)?0:position.y);
		return position;
	}

	double CiratefiData::CircularSample(Mat& image, int row, int col, int radius)
	{ 
		int row2=0; int col2=radius; double sum=0; double count=0;
		while (col2>0) 
		{
			sum=sum+image.at<uchar>(ValidImageRange(Point(row+row2, col+col2), image))
				+image.at<uchar>(ValidImageRange(Point(row-row2, col-col2), image))
				+image.at<uchar>(ValidImageRange(Point(row+col2, col-row2), image))
				+image.at<uchar>(ValidImageRange(Point(row-col2, col+row2), image));
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

	Mat CiratefiData::quadradaimpar(Mat& image)
	{
		int length=min(image.rows,image.cols);
		if (length%2==0) 
		{
			length--;
		}
		Mat tempRoi = image(Rect((image.cols-1)/2-length/2, (image.rows-1)/2-length/2, length, length));
		Mat roi(tempRoi.clone());
		return roi;
	}

	void CiratefiData::Cissq(Mat& templateImage)
	{
		_cq.resize(_scaleNum*_circleNum);
		for (int f=0; f<_scaleNum; f++) 
		{
			double scaleRatio=scale(f);

			int length=ceil(scaleRatio*templateImage.rows);
			Mat resizedTemplate(length, length, templateImage.type());
			resize(templateImage, resizedTemplate, Size(length, length));
			int resizedCircleNum=min((int)floor(scaleRatio/scale(_scaleNum-1)*(double)_circleNum),_circleNum);
			for (int c=0; c<resizedCircleNum; c++) 
			{
				_cq[f*_circleNum+c]=CircularSample(resizedTemplate,(resizedTemplate.rows-1)/2,(resizedTemplate.cols-1)/2,round((double)c*_circleDistance+_initialRadius));
			}
			for (int c=resizedCircleNum; c<_circleNum; c++) _cq[f*_circleNum+c]=1.0;
		}
	}

	void CiratefiData::Cifi(cv::Mat& sourceImage, cv::Mat& templateImage)
	{
		vector<vector<double> > cqi(_scaleNum);
		vector<double> cqi2(_scaleNum);
		for (int s=0; s<_scaleNum; s++) 
		{
			int resizedCircleNum=_circleNum-1;
			while (_cq[s*_circleNum+resizedCircleNum]==1.0 && 0<=resizedCircleNum) resizedCircleNum--;
			resizedCircleNum++;
			if (resizedCircleNum<3) MessageBox(NULL, "Query.mat has a row with less than 3 columns", "Error", MB_ICONERROR | MB_OK);
			cqi[s].resize(resizedCircleNum);
			double meanCqi=0;
			for (int c=0; c<resizedCircleNum; c++) 
			{
				cqi[s][c]=_cq[s*_circleNum+c];
				meanCqi+=cqi[s][c];
			}
			meanCqi/=(double)resizedCircleNum;
			cqi2[s]=0;
			for(vector<double>::iterator i=cqi[s].begin(); i!=cqi[s].end(); i++)
			{
				(*i)-=meanCqi;
				cqi2[s]+=(*i)*(*i);
			}
		}

		_cis.clear();
		for (int row=0; row<sourceImage.rows; row++) 
		{
			for (int col=0; col<sourceImage.cols; col++) 
			{
				double maxCoef=-2;
				int maxScale=0;
				for (int s=0; s<_scaleNum; s++) 
				{
					vector<double>& x=cqi[s];
					double x2=cqi2[s];
					vector<double> y(cqi[s].size());
					double meanY=0;
					double y2=0;
					for (int k=0; k<y.size(); k++)
					{
						y[k]=_ca[k*sourceImage.rows*sourceImage.cols+row*sourceImage.cols+col];
						meanY+=y[k];
					}
					meanY/=(double)y.size();
					for (int k=0; k<y.size(); k++)
					{
						y[k]-=meanY;
						y2+=y[k]*y[k];
					}

					double coef=0;
					for(int i=0;i<x.size();i++)
					{
						coef+=x[i]*y[i];
					}
					coef/=(sqrt(x2)*sqrt(y2));
					if (_isMatchNegative==true) coef=abs(coef);
					if (coef>maxCoef) 
					{
						maxCoef=coef;
						maxScale=s;
					}
				}
				if (maxCoef>_scaleThreshold) 
				{
					_cis.push_back(CorrData(row, col, maxScale, -1, maxCoef));
				}
			}
		}
	}

	Mat CiratefiData::DrawCifiResult(Mat& sourceImage)
	{
		Mat cifiResult;
		cvtColor(sourceImage, cifiResult, CV_GRAY2BGR);
		for(vector<CorrData>::iterator i=_cis.begin(); i!=_cis.end(); i++)
		{
			int row=i->GetRow();
			int col=i->GetCol();
			if(row>=cifiResult.rows && row<0 && col>=cifiResult.cols && col<0)
			{
				MessageBox(NULL, "DrawCifiResult: out of range", "Error", MB_ICONERROR | MB_OK);
				return Mat();
			}
			cifiResult.at<Vec3b>(row, col)[2]=(uchar)(i->GetCoefficient()*255.0);
		}
		
		return cifiResult;
	}
}

