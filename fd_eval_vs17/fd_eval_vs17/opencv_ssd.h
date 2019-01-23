#pragma once
class opencv_ssd
{
public:
	opencv_ssd();
	~opencv_ssd();

	bool detectFace(Mat &frameOpenCVDNN, std::vector<DetectionResult>& rfaces);

	size_t inWidth;
	size_t inHeight;
	double inScaleFactor;
	float confidenceThreshold;
	cv::Scalar meanVal;
	
	std::string caffeConfigFile;
	std::string caffeWeightFile;
	
	Net net;
};

