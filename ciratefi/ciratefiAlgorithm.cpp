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
}

