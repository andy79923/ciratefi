#ifndef __CIRATEFIALGORITHM_H__
#define __CIRATEFIALGORITHM_H__



namespace Ciratefi
{
	class CiratefiData
	{
	public:
		CiratefiData():_scaleNum(5),_initialScale(0.5),_finalScale(1),_angleNum(36),_scaleThreshold(0.95),_angleThreshold(0.8),_nccThreshold(0.9)
			,_isMatchNegative(false),_circleNum(16),_initialRadius(0),_finalRadius(-1),_tefiTolerance(1){}
		void CountParameter(cv::Mat& templateImage);
		double scale(int s){ return _initialScale*pow(_passoesc,s);}
		void SetScaleNum(int scaleNum){_scaleNum=scaleNum;}
		void SetInitialScale(double initialScale){_initialScale=initialScale;}
		void SetFinalScale(double finalScale){_finalScale=finalScale;}
		void SetAngleNum(int angleNum){_angleNum=angleNum;}
		void SetScaleThreshold(double scaleThreshold){_scaleThreshold=scaleThreshold;}
		void SetAngleThreshold(double angleThreshold){_angleThreshold=angleThreshold;}
		void SetNccThreshold(double nccThreshold){_nccThreshold=nccThreshold;}
		void SetMatchNegative(bool isMatchNegative){_isMatchNegative=isMatchNegative;}
		void SetCircleNum(int circleNum){_circleNum=circleNum;}
		void SetInitialRadius(double initialRadius){_initialRadius=initialRadius;}
		void SetTefiTolerance(int tefiTolerance){_tefiTolerance=tefiTolerance;}
		
		double round(double val, int precision);
		template <class T>
		T clip(const T val, const T lower, const T upper){return std::max(lower, std::min(val, upper));}
		cv::Point ValidImageRange(cv::Point& position, cv::Mat& image);
		double CircularSample(cv::Mat& image, int row, int col, int radius);
		void Cisssa(cv::Mat& sourceImage);

	private:
		int _scaleNum;
		double _initialScale;
		double _finalScale;
		int _angleNum;
		double _scaleThreshold;
		double _angleThreshold;
		double _nccThreshold;
		bool _isMatchNegative;
		int _circleNum;
		double _initialRadius;
		double _finalRadius;
		int _tefiTolerance;
		double _circleDistance;
		double _passoesc;
		double _AngleDegree;
		double _AngleRadian;
		double _templateRadius;
		std::vector<double> _ca;
		std::vector<double> _cq;
		std::vector<double> _rq;
	};


}


#endif