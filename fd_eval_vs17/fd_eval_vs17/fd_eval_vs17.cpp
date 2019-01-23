// opencv_ssd_vs17.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include "opencv_ssd.h"

enum DATASET { AFW, PASCAL, FDDB, WIDER_TEST, WIDER_VAL, UFDD };
enum MODE { WEBCAM, BENCHMARK_EVALUATION, IMAGE, IMAGE_LIST };

opencv_ssd m_detect;

void run_afw_pascal_fddb(DATASET m_data, std::string dataset_path);
void run_wider(DATASET m_data, std::string dataset_path, std::string result_dir);
void run_ufdd(std::string dataset_path);
void run_webcam(int webcam_id);
void run_image(std::string image_path);
void run_images(std::string image_path);

cv::String keys =
"{ help h 		| | Print help message. }"
"{ mode m 		| 0 | Select running mode: "
"0: WEBCAM - get image streams from webcam, "
"1: IMAGE - detect faces in single image, "
"2: IMAGE_LIST - detect faces in set of images, "
"3: BENCHMARK_EVALUATION - benchmark evaluation, results will be stored in 'detections' }"
"{ webcam i 	| 0 | webcam id, if mode is 0 }"
"{ dataset d 	| AFW | select dataset, if mode is 3:"
"AFW: afw dataset, "
"PASCAL: pascal dataset, "
"FDDB: fddb dataset, "
"WIDER_VAL: wider validation set, "
"WIDER_TEST: wider test set, "
"UFDD: ufdd valudation set }"
"{ path p 		| | Path to image file or image list dir or benchmark dataset }";

int main(int argc, char** argv)
{
	cv::CommandLineParser parser(argc, argv, keys);

	if (argc == 1 || parser.has("help"))
	{
		parser.printMessage();
		return 0;
	}

	MODE m_mode;
	int mode = parser.get<int>("mode");

	if (mode == 1) m_mode = IMAGE;
	else if (mode == 2) m_mode = IMAGE_LIST;
	else if (mode == 3) m_mode = BENCHMARK_EVALUATION;
	else m_mode = WEBCAM;

	if (m_mode == WEBCAM)
	{
		int webcam_id = parser.get<int>("webcam");
		run_webcam(webcam_id);
	}
	else if (m_mode == IMAGE)
	{
		std::string image_path = parser.get<String>("path");
		run_image(image_path);
	}
	else if (m_mode == IMAGE_LIST)
	{
		std::string image_path = parser.get<String>("path");
		run_images(image_path);
	}
	else if (m_mode == BENCHMARK_EVALUATION)
	{
		DATASET m_data;
		std::string dataset_name = parser.get<String>("dataset");
		cout << dataset_name << endl;
		if (dataset_name == "PASCAL") m_data = PASCAL;
		else if (dataset_name == "FDDB") m_data = FDDB;
		else if (dataset_name == "WIDER_VAL") m_data = WIDER_VAL;
		else if (dataset_name == "WIDER_TEST") m_data = WIDER_TEST;
		else if (dataset_name == "UFDD") m_data = UFDD;
		else if (dataset_name == "AFW") m_data = AFW;

		std::string dataset_path = parser.get<String>("path");

		if (m_data == AFW || m_data == PASCAL || m_data == FDDB)
			run_afw_pascal_fddb(m_data, dataset_path);
		else if (m_data == WIDER_VAL || m_data == WIDER_TEST) {
			std::string result_dir;
			if (m_data == WIDER_TEST) result_dir = "detections/WIDER/ocvssd_test/";
			else result_dir = "detections/WIDER/ocvssd_val/";
			run_wider(m_data, dataset_path, result_dir);
		}
		else if (m_data == UFDD)
			run_ufdd(dataset_path);
	}

	return 1;
}

void run_webcam(int webcam_id)
{
	VideoCapture cap(webcam_id);
	if (!cap.isOpened()) {
		cout << "fail to open webcam!" << endl;
		return;
	}

	cv::Mat image;
	while (true) {

		cap >> image;

		if (image.empty()) break;

		std::vector<DetectionResult> results;
		m_detect.detectFace(image, results);

		imshow("result", image);
		if (waitKey(1) >= 0) break;
	}
}

void run_image(std::string image_path)
{

	cv::Mat image = cv::imread(image_path.c_str());

	if (image.empty()) {
		cout << "Image not exist in the specified dir!";
		return;
	}

	std::vector<DetectionResult> results;
	m_detect.detectFace(image, results);

	imshow("result", image);
	waitKey(1);
}

void run_images(std::string image_path)
{
	std::vector<cv::String> img_list;
	glob(image_path + "*.jpg", img_list, false);

	size_t count = img_list.size();
	for (size_t i = 0; i < count; i++)
	{
		cv::Mat image = cv::imread(img_list[i].c_str());
		if (image.empty()) {
			cout << "Image not exist in the specified dir!";
			return;
		}

		std::vector<DetectionResult> results;
		m_detect.detectFace(image, results);

		imshow("result", image);
		waitKey(1);
	}
}

void run_afw_pascal_fddb(DATASET m_data, std::string dataset_path)
{
	cout << m_data << endl;
	cout << dataset_path << endl;

	std::string str_img_file;
	if (m_data == AFW) str_img_file = "detections/AFW/afw_img_list.txt";
	else if (m_data == PASCAL) str_img_file = "detections/PASCAL/pascal_img_list.txt";
	else if (m_data == FDDB) str_img_file = "detections/FDDB/fddb_img_list.txt";
	else return;

	cout << str_img_file << endl;

	std::ifstream inFile(str_img_file.c_str(), std::ifstream::in);

	std::vector<string> image_list;
	std::string imname;
	while (std::getline(inFile, imname))
	{
		image_list.push_back(imname);
	}

	std::string str_out_file;
	if (m_data == AFW) str_out_file = "detections/AFW/ocvssd_afw_dets.txt";
	else if (m_data == PASCAL) str_out_file = "detections/PASCAL/ocvssd_pascal_dets.txt";
	else if (m_data == FDDB) str_out_file = "detections/FDDB/ocvssd_fddb_dets.txt";

	std::ofstream outFile(str_out_file.c_str());
	// process each image one by one

	for (int i = 0; i < image_list.size(); i++)
	{
		std::string imname = image_list[i];
		std::string tempname = imname;

		if (m_data == AFW) imname = dataset_path + imname + ".jpg";
		else if (m_data == PASCAL) imname = dataset_path + imname;
		else if (m_data == FDDB) imname = dataset_path + imname + ".jpg";

		cout << "processing image " << i + 1 << "/" << image_list.size() << " [" << imname.c_str() << "]" << endl;

		cv::Mat image = cv::imread(imname);

		std::vector<DetectionResult> results;
		m_detect.detectFace(image, results);

		if (m_data != FDDB)
		{
			for (int j = 0; j < results.size(); j++) {
				outFile << tempname << " " << results[j].score << " " << results[j].r.x << " "
					<< results[j].r.y << " " << results[j].r.x + results[j].r.width << " "
					<< results[j].r.y + results[j].r.height << endl;
			}
		}
		else {

			outFile << tempname << "\n";
			outFile << results.size() << "\n";
			for (int j = 0; j < results.size(); j++) {
				outFile << results[j].r.x << " " << results[j].r.y << " "
					<< results[j].r.width << " " << results[j].r.height << " " << results[j].score << "\n";
			}
		}

		imshow("test", image);

		waitKey(1);
	}
	outFile.close();
}

void run_wider(DATASET m_data, std::string dataset_path, std::string result_dir)
{
	std::string str_img_file;
	if (m_data == WIDER_TEST) str_img_file = "detections/WIDER/wider_test_list.txt";
	else if (m_data == WIDER_VAL) str_img_file = "detections/WIDER/wider_val_list.txt";
	else return;

	std::ifstream inFile(str_img_file.c_str(), std::ifstream::in);

	std::vector<string> image_list;
	while (!inFile.eof())
	{
		std::string str;
		inFile >> str;
		if (str == "") continue;
		image_list.push_back(str);
	}

	std::string wider_path = dataset_path;

	for (int i = 0; i < image_list.size(); i++)
	{
		std::string imname = wider_path + image_list[i];
		//cout << imname << endl;
		cout << "Processing image " << i + 1 << "/" << image_list.size() << " [" << image_list[i].c_str() << "]" << endl;

		cv::Mat image = cv::imread(imname);

		std::vector<DetectionResult> results;
		m_detect.detectFace(image, results);

		std::string txt_name = image_list[i];
		txt_name.replace(txt_name.end() - 4, txt_name.end(), ".txt");
		txt_name = result_dir + txt_name;

		std::ofstream out_txt(txt_name.c_str());
		out_txt << image_list[i] << "\n";
		out_txt << results.size() << "\n";
		for (int j = 0; j < results.size(); j++) {
			out_txt << results[j].r.x << " " << results[j].r.y << " " << results[j].r.width << " "
				<< results[j].r.height << " " << results[j].score << "\n";
		}
		out_txt.close();

		//imshow("OPENCV SSD", image);

		waitKey(1);
	}
}

void run_ufdd(std::string dataset_path)
{
	std::string result_dir;
	result_dir = "detections/UFDD/";

	std::string str_img_file;
	str_img_file = "detections/UFDD/ufdd_img_list.txt";

	std::ifstream inFile(str_img_file.c_str(), std::ifstream::in);

	std::vector<string> image_list;
	while (!inFile.eof())
	{
		std::string str;
		inFile >> str;
		if (str == "") continue;
		image_list.push_back(str);
	}

	std::string ufdd_path = dataset_path;

	for (int i = 0; i < image_list.size(); i++)
	{
		std::string imname = ufdd_path + image_list[i] + ".jpg";
		cout << imname << endl;
		cout << "processing image " << i + 1 << "/" << image_list.size() << " [" << image_list[i].c_str() << "]" << endl;

		cv::Mat image = cv::imread(imname);

		std::vector<DetectionResult> results;
		m_detect.detectFace(image, results);

		std::string txt_name = image_list[i] + ".txt";
		txt_name = result_dir + txt_name;

		std::ofstream out_txt(txt_name.c_str());
		out_txt << image_list[i] << "\n";
		out_txt << results.size() << "\n";
		for (int j = 0; j < results.size(); j++) {
			out_txt << results[j].r.x << " " << results[j].r.y << " " << results[j].r.width << " "
				<< results[j].r.height << " " << results[j].score << "\n";
		}
		out_txt.close();

		imshow("image", image);

		waitKey(1);
	}
}
