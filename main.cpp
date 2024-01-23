#include <iostream>
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/opencv.hpp>

class Project_2 {

	cv::Mat img;

public:

	Project_2() {};
	Project_2(const std::string& path) {

		cv::Mat img_og = cv::imread(path);
		if (img_og.empty()) {
			std::cerr << "Error: Cannot load a picture\n";
			exit(EXIT_FAILURE);
		}

		resize_img(img_og);

	}

	void resize_img(cv::Mat img_og) {

		cv::resize(img_og, img, cv::Size(1024, 1024));

	}
	
	void red_circle() {

		cv::imshow("Original Image", img);
		cv::waitKey(0);

		//BGR to HSV
		cv::Mat hsv;
		cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);
		cv::imshow("HSV", hsv);
		cv::waitKey(0);

		//cv::Scalar lower_threshold1 = cv::Scalar(0, 100, 20);
		//cv::Scalar upper_threshold1 = cv::Scalar(10, 255, 255);
		cv::Scalar lower_threshold2 = cv::Scalar(150, 40, 0);
		cv::Scalar upper_threshold2 = cv::Scalar(179, 255, 255);

		//mask for red color
		cv::Mat red_mask;
		//cv::inRange(hsv, lower_threshold1, upper_threshold1, red_mask1);
		cv::inRange(hsv, lower_threshold2, upper_threshold2, red_mask);
		//cv::Mat red_mask = red_mask2; | red_mask2;

		//red color = 255, rest = 0
		cv::Mat binarization_red;
		cv::threshold(red_mask, binarization_red, 0, 255, cv::THRESH_BINARY);
		cv::imshow("BIN", binarization_red);
		cv::waitKey(0);


		//dilate to rid off holes in sign
		cv::Mat dilate_mat;
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
		cv::dilate(binarization_red, dilate_mat, kernel);
		cv::imshow("Dilate", dilate_mat);
		cv::waitKey(0);

		//find all contours
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(dilate_mat, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
		
		int max_area_of_children = 0;
		int index_of_max_area_of_children = 0;
		for (size_t i = 0; i < contours.size(); i++) {
			std::vector<cv::Point> approx;
			float epsilon = 0.02 * cv::arcLength(contours[i], true);
			cv::approxPolyDP(contours[i], approx, epsilon, true);
			std::cout << approx.size() << std::endl;
			int child = hierarchy[i][2];
			//find contours that are circles
			if (approx.size() >= 8 && cv::contourArea(contours[i]) > 200) {
				
				int number_of_children = 0;

				while (child != -1) {
					number_of_children++;
					child = hierarchy[child][0];
				}
				//find circles with only one contour that is the biggest rectangle
				if (number_of_children == 1) {
					child = hierarchy[i][2];
					std::vector<cv::Point> approxchild;
					float epsilonchild = 0.02 * cv::arcLength(contours[i], true);
					cv::approxPolyDP(contours[child], approxchild, epsilonchild, true);
					std::cout << approxchild.size() << std::endl;
					if (approxchild.size() == 4) {
						float area_children = cv::contourArea(contours[child]);
						if (area_children > max_area_of_children) {
							max_area_of_children = area_children;
							index_of_max_area_of_children = i;
						}
						
					}
					
				}
				
			}
			
			//cv::drawContours(img, contours, child, cv::Scalar(0, 0, 0), 2);
		}
		cv::drawContours(img, contours, index_of_max_area_of_children, cv::Scalar(0, 255, 0), 2);
		cv::Moments moment = cv::moments(contours[index_of_max_area_of_children]);
		int center_x = moment.m10 / moment.m00;
		int center_y = moment.m01 / moment.m00;
		cv::circle(img, cv::Point(center_x, center_y), 5, cv::Scalar(0, 255, 0), -1);
		std::stringstream stream;
		stream << "center: " << "(" << center_x << ", " << center_y << ")";
		cv::putText(img, stream.str(), cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		
		cv::imshow("Detected Zakaz wjazdu", img);
		cv::waitKey(0);
	}


	void yellow_triangle() {

		cv::imshow("Original Image", img);
		cv::waitKey(0);

		//BGR to HSV
		cv::Mat hsv;
		cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);
		cv::imshow("HSV", hsv);
		cv::waitKey(0);

		cv::Scalar lower_threshold = cv::Scalar(20, 100, 100);
		cv::Scalar upper_threshold = cv::Scalar(30, 255, 255);

		//mask for yellow color
		cv::Mat yellow_mask;
		cv::inRange(hsv, lower_threshold, upper_threshold, yellow_mask);

		//Yellow color = 255, rest = 0
		cv::Mat binarization_yellow;
		cv::threshold(yellow_mask, binarization_yellow, 0, 255, cv::THRESH_BINARY);
		cv::imshow("BIN", binarization_yellow);
		cv::waitKey(0);

		//erode to get better shape and rid off noise
		cv::Mat erode_mat;
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(1, 1));
		cv::erode(binarization_yellow, erode_mat, kernel);
		cv::imshow("Erode", erode_mat);
		cv::waitKey(0);
		
		//find all contours
		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(erode_mat, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		//find contour with the biggest amount of yellow pixels (pixel > 255)
		int max_amount_of_white_pixels = 0;
		int max_index = 0;


		for (size_t i = 0; i < contours.size(); i++) {

			//additional check if the contour is a triangle
			std::vector<cv::Point> approx;
			float epsilon = 0.02 * cv::arcLength(contours[i], true);
			cv::approxPolyDP(contours[i], approx, epsilon, true);


			if (approx.size() == 3) {
				cv::Rect roi_rect = cv::boundingRect(contours[i]);
				cv::Mat roi = erode_mat(roi_rect);

				int white_pixels = cv::countNonZero(roi);

				if (white_pixels > max_amount_of_white_pixels) {
					max_index = i;
					max_amount_of_white_pixels = white_pixels;
				}
			}
			
		}
		cv::drawContours(img, contours, max_index, cv::Scalar(0, 255, 0), 2);

		cv::Moments moment = cv::moments(contours[max_index]);
		int center_x = moment.m10 / moment.m00;
		int center_y = moment.m01 / moment.m00;

		cv::circle(img, cv::Point(center_x, center_y), 5, cv::Scalar(0, 255, 0), -1);
		std::stringstream stream;
		stream << "center: " << "(" << center_x << ", " << center_y << ")";
		cv::putText(img, stream.str(), cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);

		cv::imshow("Detected Ust¹p pierszeñstwa", img);
		cv::waitKey(0);
	}
	};

						


int main() {
	
	std::string path = "C:/Users/marcin/Desktop/Projekt2_Marcin_Szymczak_314910/zakaz3.jpg";
	Project_2 result(path);
	result.yellow_triangle();
	//result.red_circle();
	
	return 0;
}