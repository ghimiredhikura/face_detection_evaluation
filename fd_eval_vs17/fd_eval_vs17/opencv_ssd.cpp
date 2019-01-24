#include "pch.h"
#include "opencv_ssd.h"

opencv_ssd::opencv_ssd()
{
	caffeConfigFile = "data/opencv_ssd_face_fp16_deploy.prototxt";
	caffeWeightFile = "data/opencv_ssd_face_fp16.caffemodel";

	inWidth = 300;
	inHeight = 300;
	inScaleFactor = 1.0;
	confidenceThreshold = 0.05;
	meanVal = cv::Scalar(104.0, 177.0, 123.0);

	net = cv::dnn::readNetFromCaffe(caffeConfigFile, caffeWeightFile);
}

opencv_ssd::~opencv_ssd()
{
}

bool opencv_ssd::detectFace(Mat &frameOpenCVDNN, std::vector<DetectionResult>& rfaces)
{
	int frameHeight = frameOpenCVDNN.rows;
	int frameWidth = frameOpenCVDNN.cols;
	cv::Mat inputBlob = cv::dnn::blobFromImage(frameOpenCVDNN, inScaleFactor, cv::Size(inWidth, inHeight), meanVal, false, false);

	net.setInput(inputBlob, "data");
	cv::Mat detection = net.forward("detection_out");

	cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());
		
	for (int i = 0; i < detectionMat.rows; i++)
	{
		float confidence = detectionMat.at<float>(i, 2);
		if (confidence > confidenceThreshold)
		{
			DetectionResult rface;
			int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * frameWidth);
			int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * frameHeight);
			int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * frameWidth);
			int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * frameHeight);

			cv::Rect r(cv::Point(x1, y1), cv::Point(x2, y2));
			rface.r = r;
			rface.score = confidence;

			rfaces.push_back(rface);

			if (confidence > 0.2) {
				cv::rectangle(frameOpenCVDNN, r, CV_RGB(255, 0, 0), 2);
			}
		}
	}

	if (!rfaces.size())
		return false;

	return true;
}
