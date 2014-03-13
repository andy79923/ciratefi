#include "stdafx.h"
#include "ciratefiAlgorithm.h"

using namespace std;
using namespace cv;

namespace Ciratefi
{
	void CiratefiData::CountParameter(Mat& templateImage)
	{
		if (_scaleNum>1) _passoesc=exp(log(_finalScale/_initialScale)/_scaleNum); else _passoesc=1.0;
		_angleDegree=360.0/_angleNum;
		_angleRadian = _angleDegree * M_PI / 180.0;
		_finalRadius=scale(_scaleNum-1)*(templateImage.rows/2);
		_templateRadius=templateImage.rows/2;
		if (_circleNum>1) _circleDistance=(_finalRadius-_initialRadius)/(_circleNum-1); else _circleDistance=0.0;
	}

	double CiratefiData::CircularSample(Mat& image, int y, int x, int radius)
	{ 
		int y2=0; int x2=radius; int sum=0; int count=0;
		int r2=radius*radius;
		while (x2>0) 
		{
			sum+=*(image.data+image.step[0]*(y+y2)+image.step[1]*(x+x2));
			sum+=*(image.data+image.step[0]*(y-y2)+image.step[1]*(x-x2));
			sum+=*(image.data+image.step[0]*(y+x2)+image.step[1]*(x-y2));
			sum+=*(image.data+image.step[0]*(y-x2)+image.step[1]*(x+y2));

			count=count+4;

			int mh=abs((y2+1)*(y2+1)+x2*x2-r2);
			int md=abs((y2+1)*(y2+1)+(x2-1)*(x2-1)-r2);
			int mv=abs(y2*y2+(x2-1)*(x2-1)-r2);
			int m=min(min(mh, md), mv);
			if (m==mh) y2++;
			else if (m==md) { y2++; x2--; }
			else x2--;
		}
		if (count>0)
		{
			return clip(((double)sum+(double)count/2.0)/(double)count, 0.0, 255.0);
		}
		return *(image.data+y*image.step[0]+x*image.step[1]);
	}

	void CiratefiData::Cisssa(Mat& sourceImage)
	{
		_ca.resize(_circleNum*sourceImage.rows*sourceImage.cols,-1.0);
		int n=sourceImage.rows*sourceImage.cols;
		int smallestRadius=ceil(scale(0)*_templateRadius);
		int lastRow=sourceImage.rows-smallestRadius;
		int lastCol=sourceImage.cols-smallestRadius;

		for (int s=0; s<_circleNum; s++) 
		{
			int sn=s*n;
			int radius=round(_circleDistance*s+_initialRadius);
			for (int y=smallestRadius; y<lastRow; y++)
			{
				int rn=y*sourceImage.cols;
				for (int x=smallestRadius; x<lastCol; x++) 
				{
					if(y+radius<sourceImage.rows && y-radius>=0 && x+radius<sourceImage.cols && x-radius>=0)
					{
						_ca[sn+rn+x]=CircularSample(sourceImage, y, x, radius);
					}					
				}
			}
		}
	}

	Mat CiratefiData::quadradaimpar(Mat& image)
	{
		int length=min(image.rows,image.cols);
		if (length%2==0) length--;
		Mat tempRoi = image(Rect((image.cols-1)/2-length/2, (image.rows-1)/2-length/2, length, length));
		Mat roi(tempRoi.clone());
		return roi;
	}

	void CiratefiData::Cissq(Mat& templateImage)
	{
		_cq.resize(_scaleNum*_circleNum, -1.0);
		Mat resizedTemplate;
		for (int s=0; s<_scaleNum; s++) 
		{
			int sn=s*_circleNum;
			double scaleRatio=scale(s);
			int length=ceil(scale(s)*templateImage.rows);

			resize(templateImage, resizedTemplate, Size(length, length));
			int resizedCircleNum=min((int)floor(scaleRatio/scale(_scaleNum-1)*_circleNum),_circleNum);
			int templateRowCenter=(resizedTemplate.rows-1)/2;
			int templateColCenter=(resizedTemplate.cols-1)/2;
			for (int c=0; c<resizedCircleNum; c++) 
			{
				_cq[sn+c]=CircularSample(resizedTemplate,templateRowCenter,templateColCenter,round(c*_circleDistance+_initialRadius));
			}
		}
	}

	void CiratefiData::Cifi(Mat& sourceImage)
	{
		vector<vector<double> > cqi(_scaleNum);
		vector<double> cqi2(_scaleNum,0);
		for (int s=0; s<_scaleNum; s++) 
		{
			int resizedCircleNum=_circleNum-1;
			int sn=s*_circleNum;
			while (_cq[sn+resizedCircleNum]<0.0 && 0<=resizedCircleNum) resizedCircleNum--;
			resizedCircleNum++;
			if (resizedCircleNum<3) MessageBox(NULL, "There has a resizedTemplate with less than 3 circle", "Error", MB_ICONERROR | MB_OK);
			cqi[s].resize(resizedCircleNum);
			double meanCqi=0;
			for(int i=0; i< resizedCircleNum; i++)
			{
				cqi[s][i]=_cq[sn+i];
				meanCqi+=_cq[sn+i];

			}
			meanCqi/=(double)resizedCircleNum;
			for(int i=0;i<resizedCircleNum;i++)
			{
				cqi[s][i]-=meanCqi;
				cqi2[s]+=cqi[s][i]*cqi[s][i];
			}
		}

		_cis.clear();
		int n=sourceImage.rows*sourceImage.cols;
		int smallestRadius=ceil(scale(0)*_templateRadius);
		int lastRow=sourceImage.rows-smallestRadius;
		int lastCol=sourceImage.cols-smallestRadius;
		_cis.reserve((lastRow-smallestRadius)*(lastCol-smallestRadius)*_scaleNum);
		vector<double> Y;
		for (int y=smallestRadius; y<lastRow; y++) 
		{
			int rn=y*sourceImage.cols;
			for (int x=smallestRadius; x<lastCol; x++) 
			{
				double maxCoef=-2;
				int fitScale;
				for (int s=0; s<_scaleNum; s++) 
				{
					vector<double>& X=cqi[s];
					double X2=cqi2[s];
					Y.resize(cqi[s].size());
					double meanY=0;
					double Y2=0;
					for (int i=Y.size()-1; i>=0; i--)
					{
						Y[i]=_ca[i*n+rn+x];
						if(Y[i]<0.0)
						{
							meanY=-1;
							break;
						}
						meanY+=Y[i];
					}
					if(meanY<0) continue;
					meanY/=(double)Y.size();
					for(int i=0;i<Y.size();i++)
					{
						Y[i]-=meanY;
						Y2+=Y[i]*Y[i];
					}

					double coef=0;
					for(int i=0; i<X.size(); i++)
					{
						coef+=X[i]*Y[i];
					}
					coef/=sqrt(X2*Y2);
					if (_isMatchNegative==true) coef=abs(coef);
					if (coef>maxCoef) 
					{
						maxCoef=coef;
						fitScale=s;
					}
				}
				if (maxCoef>_scaleThreshold) 
				{
					_cis.push_back(CorrData(y, x, fitScale, -1, maxCoef));
				}
			}
		}
	}

	Mat CiratefiData::DrawCifiResult(Mat& sourceImage)
	{
		Mat cifiResult;
		cvtColor(sourceImage, cifiResult, CV_GRAY2BGR);
		for(int i=0; i<_cis.size(); i++)
		{
			int row=_cis[i].GetRow();
			int col=_cis[i].GetCol();
			if(row>=cifiResult.rows && row<0 && col>=cifiResult.cols && col<0)
			{
				MessageBox(NULL, "DrawCifiResult: out of range", "Error", MB_ICONERROR | MB_OK);
				return Mat();
			}
			cifiResult.at<Vec3b>(row, col)=Vec3b((uchar)(_cis[i].GetCoefficient()*255.0),_cis[i].GetScale(), 255);
		}		
		return cifiResult;
	}

	double CiratefiData::RadialSample(Mat& image, int centerY, int centerX, double angle, double radius)
	{
		//把圓切成8等份計算
		int sum=0; int count=0;
		int x,y,dx,dy,a,err,dx2,dy2,sobe;
		int x1=centerX; int x2=centerX+round(cos(angle)*radius);
		int y1=centerY; int y2=centerY-round(sin(angle)*radius);
		dx=x2-x1; dy=y2-y1;

		if (abs(dx)>=abs(dy))//成立代表位於圓形的1, 4, 5, 8區
		{ 
			if (dx<0) 
			{ swap(x1,x2); swap(y1,y2); dx=-dx; dy=-dy; }
			a=abs(2*dy); err=0; dx2=dx*2;
			sobe=(dy==0)?0:((dy>0)?1:-1); y=y1;
			for (x=x1; x<=x2; x++) 
			{
				sum+=*(image.data+y*image.step[0]+x*image.step[1]); count++; err=err+a;
				if (err>=dx) { y=y+sobe; err=err-dx2; }
			}
		} 
		else//代表位於圓形的2, 3, 6, 7區
		{
			if (dy<0) { swap(x1,x2); swap(y1,y2); dx=-dx; dy=-dy; }
			a=abs(2*dx); err=0; dy2=dy*2;
			sobe=(dx==0)?0:((dx>0)?1:-1); x=x1;
			for (y=y1; y<=y2; y++) 
			{
				sum+=*(image.data+y*image.step[0]+x*image.step[1]); count++; err=err+a;
				if (err>=dy) { x=x+sobe; err=err-dy2; }
			}
		}
		if (count>0)
		{
			return clip(((double)sum+(double)count/2.0)/(double)count, 0.0, 255.0);
		}
		return *(image.data+centerY*image.step[0]+centerX*image.step[1]);
	}

	void CiratefiData::Rassq(Mat& templateImage)
	{
		_rq.resize(_angleNum);
		for (int a=0; a<_angleNum; a++)
		{
			_rq[a]=RadialSample(templateImage, _templateRadius, _templateRadius, _angleRadian*a,_templateRadius);
		}
	}

	void CiratefiData::Rafi(Mat& sourceImage)
	{
		vector<double> X(_angleNum);
		for(int i=0; i<_angleNum; i++) X[i]=_rq[i];
		double meanX=0;
		double X2=0;
		for(int i=0;i<_angleNum;i++)
		{
			meanX+=X[i];
		}
		meanX/=(double)_angleNum;
		for(int i=0;i<_angleNum;i++)
		{
			X[i]-=meanX;
			X2+=X[i]*X[i];
		}

		_ras.clear();
		_ras.reserve(_cis.size()*_angleNum);
		vector<double> Y(_angleNum);
		for (int i=0; i<_cis.size(); i++) 
		{
			CorrData& candidate=_cis[i];

			double scaleRatio=scale(candidate.GetScale());
			double angleRange=2.0*M_PI/_angleNum;
			int y=candidate.GetRow();
			int x=candidate.GetCol();
			double maxCoef=-2; int fitAngle=0;

			double meanY=0;
			for (int a=0; a<_angleNum; a++)
			{
				Y[a]=RadialSample(sourceImage, y, x, a*angleRange, _templateRadius*scaleRatio);
				meanY+=Y[a];

			}
			meanY/=(double)_angleNum;
			double Y2=0;
			for(int a=0; a<_angleNum; a++)
			{
				Y[a]-=meanY;
				Y2+=Y[a]*Y[a];
			}

			for (int j=0; j<_angleNum; j++) 
			{
				double coef=0;
				for(int k=0; k<_angleNum; k++)
				{
					coef+=X[k]*Y[k];
				}
				coef/=sqrt(X2*Y2);

				if (_isMatchNegative) coef=abs(coef);
				if (coef>maxCoef) 
				{
					maxCoef=coef; fitAngle=j;
				}
				rotate(X.rbegin(),X.rbegin()+1,X.rend());
			}
			if (maxCoef>_angleThreshold) 
			{
				_ras.push_back(CorrData(y, x, candidate.GetScale(), fitAngle, maxCoef));
			}
		}
	}

	Mat CiratefiData::DrawRafiResult(Mat& sourceImage)
	{
		Mat rafiResult;
		cvtColor(sourceImage, rafiResult, CV_GRAY2BGR);
		for(int i=0; i<_ras.size(); i++)
		{
			int row=_ras[i].GetRow();
			int col=_ras[i].GetCol();
			if(row>=rafiResult.rows && row<0 && col>=rafiResult.cols && col<0)
			{
				MessageBox(NULL, "DrawRafiResult: out of range", "Error", MB_ICONERROR | MB_OK);
				return Mat();
			}
			rafiResult.at<Vec3b>(row, col)=Vec3b((uchar)(_ras[i].GetCoefficient()*255.0),_ras[i].GetAngle(), 255);
		}		
		return rafiResult;
	}

	void CiratefiData::Tefi(Mat& sourceImage, Mat& templateImage)
	{
		vector< vector<bool> > possibleCheck(_scaleNum*_angleNum);
		vector< vector<double> > possibleTemplateX(_scaleNum*_angleNum);
		vector<double> possibleTemplateX2(_scaleNum*_angleNum, -1.0);

		_tes.clear();
		_tes.reserve(_ras.size()*(4*_tefiTolerance+2*_tefiTolerance+1));//_ras.size()*total scale number*total angle number
		vector<double> Y;
		Y.reserve(pow(round(scale(_scaleNum-1)*_templateRadius),2));

		for(int i=0; i<_ras.size(); i++)
		{
			CorrData& candidate=_ras[i];
			int y=candidate.GetRow();
			int x=candidate.GetCol();
			double maxCoef=-2;
			int initialScale=clip(candidate.GetScale()-_tefiTolerance, 0, _scaleNum);
			int finalScale=clip(candidate.GetScale()+_tefiTolerance, 0, _scaleNum);
			int initialAngle=candidate.GetAngle()-_tefiTolerance;
			int finalAngle=candidate.GetAngle()+_tefiTolerance+1;
			int fitScale=0, fitAngle=0;
			for(int s=initialScale; s<finalScale;s++)
			{
				int sn=s*_angleNum;
				for(int a=initialAngle; a<finalAngle; a++)
				{
					int a2=a%_angleNum;
					if(a2<0) a2+=_angleNum;
					double coef=0;
					double scaleRatio=scale(s);
					int length=round(scaleRatio*templateImage.rows);
					int radius=(length-1)/2;

					if(possibleTemplateX2[sn+a2]<0.0)
					{
						possibleTemplateX2[sn+a2]=0;
						double meanX=0;
						Mat possibleTemplate;

						double angle=a2*_angleDegree;
						possibleTemplateX[sn+a2].reserve(length*length);
						possibleCheck[sn+a2].resize(length*length, true);
						resize(templateImage,possibleTemplate, Size(length, length));
						Mat affine=getRotationMatrix2D(Point2f(radius, radius), angle, 1.0);
						warpAffine(possibleTemplate, possibleTemplate, affine, possibleTemplate.size());

						for (int y=0; y<length; y++)
						{
							int yn=y*length;
							for (int x=0; x<length; x++) 
							{
								if(round(sqrt(double((x-radius)*(x-radius)+(y-radius)*(y-radius))))<=radius)
								{
									possibleTemplateX[sn+a2].push_back(*(possibleTemplate.data+y*possibleTemplate.step[0]+x*possibleTemplate.step[1]));
									meanX+=possibleTemplateX[sn+a2].back();
									continue;
								}
								*(possibleTemplate.data+y*possibleTemplate.step[0]+x*possibleTemplate.step[1])=0;
								possibleCheck[sn+a2][yn+x]=false;

							}
						}
						meanX/=(double)possibleTemplateX[sn+a2].size();
						for(int j=0; j<possibleTemplateX[sn+a2].size(); j++)
						{
							possibleTemplateX[sn+a2][j]-=meanX;
							possibleTemplateX2[sn+a2]+=possibleTemplateX[sn+a2][j]*possibleTemplateX[sn+a2][j];
						}
					}

					vector<double>& X=possibleTemplateX[sn+a2];

					int c1=x-radius, c2=c1+length;
					int r1=y-radius, r2=r1+length;
					double meanY=0;
					Y.clear();
					for(int row=r1; row<r2; row++)
					{
						int yn=(row-r1)*length;
						for(int col=c1; col<c2; col++)
						{
							if(possibleCheck[sn+a2][yn+col-c1]==true)
							{
								Y.push_back(*(sourceImage.data+row*sourceImage.step[0]+col*sourceImage.step[1]));
								meanY+=Y.back();
							}
						}
					}
					meanY/=(double)Y.size();

					if(X.size()!=Y.size())
					{
						MessageBox(NULL, "Tefi: size is not the same", "Error", MB_ICONERROR | MB_OK);
					}

					double Y2=0;
					for(int j=0; j<Y.size(); j++)
					{
						Y[j]-=meanY;
						Y2+=Y[j]*Y[j];
						coef+=X[j]*Y[j];
					}

					coef/=sqrt(possibleTemplateX2[sn+a2]*Y2);


					if (_isMatchNegative) coef=abs(coef);
					if (coef>maxCoef) 
					{
						maxCoef=coef;
						fitScale=s;
						fitAngle=a2;
					}
				}
			}
			if (maxCoef>_nccThreshold)
			{
				_tes.push_back(CorrData(y, x, fitScale, fitAngle, maxCoef));
			}
		}
		}
}

