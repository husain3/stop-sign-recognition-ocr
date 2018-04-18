#include <iostream>
#include <vector>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <tesseract/baseapi.h>


using namespace cv;

const std::string windowName1 = "Stop Sign";

struct cvRectCorners {
	Point pt1;
	Point pt2;
};

/*Will have to optimize for multithreading*/
void findBlobs(std::vector<std::vector<Point> > &foundContours, std::vector<cvRectCorners> &foundBlobs, const Mat origImg, std::vector<Mat> &extractedImgs
) {
	int foundObject = 2;

	int minX, maxX;
	int minY, maxY;
	std::vector<std::vector<Point> >::iterator outer;
	std::vector<Point>::iterator inner;

	cvRectCorners current;

	for(outer = foundContours.begin(); outer != foundContours.end(); outer++)
	{	
		minX = maxX = outer->at(0).x;
		minY = maxY = outer->at(0).y;

		for(inner = outer->begin() + 1; inner != outer->end(); inner++)
		{
			//std::cout << inner[0] << '\n';
			//Checking for minimax X values
			if(inner[0].x < minX)
			{
				minX = inner[0].x;
			} else if (inner[0].x > maxX) {
				maxX = inner[0].x;
			}

			//Checking for minimax Y values
			if(inner[0].y < minY)
			{
				minY = inner[0].y;
			} else if (inner[0].y > maxY) {
				maxY = inner[0].y;
			}
		}
		// std::cout << "Min X: " << minX << '\n';
		// std::cout << "Max X: " << maxX << '\n';

		// std::cout << "Min Y: " << minY << '\n';
		// std::cout << "Max Y: " << maxY << '\n';
		

		Point lowerLeft(minX, minY);
		Point upperRight(maxX, maxY);

		//Extracting image using above two Points (make into Rect)
		Rect matRect(lowerLeft, upperRight);
		Mat picExtract(origImg, matRect);

		Size s = picExtract.size();
		s.height = s.height * 6;
		s.width = s.width * 6;
		resize(picExtract, picExtract, s);

		extractedImgs.push_back(picExtract);

		//Packing points into cvRectCorners struct
		current.pt1 = lowerLeft;
		current.pt2 = upperRight;
		
		foundBlobs.push_back(current);
	}
	

}

void morphOps(Mat &thresh) {
    //create structuring element that will be used to "dilate" and "erode" image.
    
    //the element chosen here is a 3px by 3px rectangle
    Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3,3));
    
    //dilate with larger element so make sure object is nicely visible
    Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8,8));
    
    erode(thresh, thresh, erodeElement);
    erode(thresh, thresh, erodeElement);
    
    dilate(thresh, thresh, dilateElement);
    dilate(thresh, thresh, dilateElement);
}


int main(int argc, const char * argv[]) {
	Mat img;
	Mat HSV, masked;

	//For findContours
	std::vector<std::vector<Point> > contours;
    std::vector<Vec4i> heirarchy;
    




	/* Upper/Lower Red shade bounds from color palette*/
    //Scalar lowerRedBound = Scalar(0, 255, 70);
    //Scalar upperRedBound = Scalar(0, 255, 223);


	/* Upper/Lower Red shade bounds from New Zealand dashboard cam for stop signs*/
    //Scalar lowerRedBound = Scalar(0, 0, 0);
    //Scalar upperRedBound = Scalar(256, 256, 85);
    
    /* Upper/Lower Red shade bounds from New Zealand dashboard cam for stop signs*/
    Scalar lowerRedBound = Scalar(171, 0, 0);
    Scalar upperRedBound = Scalar(256, 256, 103);


	img = imread("signsigndash.png");
	namedWindow(windowName1, 1);

	//imshow("NZ Stop Signs", img);

	cvtColor(img, HSV, CV_BGR2HSV);
	inRange(HSV, lowerRedBound, upperRedBound, masked);

	//"Opening" (Image Processing Procedure). To make red blobs visible
	morphOps(masked);



	//Find contours (searching for redblobs)
	findContours(masked, contours, heirarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	/* THIS IS WHERE WE GET THE AREAS WHERE THE RED BLOBS ARE AND EXTRACT THEM FROM THE ORIGINAL IMAGE*/
	//Need to create function to get min/max x and y values

	std::vector<Mat> extractedImgs;

	std::vector<cvRectCorners> redBlobLocations;
	std::vector<cvRectCorners>::iterator rectCoor;


	findBlobs(contours, redBlobLocations, img, extractedImgs);



	for(rectCoor = redBlobLocations.begin(); rectCoor != redBlobLocations.end(); rectCoor++) {
		rectangle(img, rectCoor->pt1, rectCoor->pt2, Scalar(0, 255, 0), 2);

	}

	
	Mat extractedMasked;
	
	inRange(extractedImgs[1], lowerRedBound, upperRedBound, extractedMasked);



	/*Resizing*/
	//InputArray hello(extractedImgs[1]);

	//std::cout << "Size " << extractedImgs[2].size().height << '\n';

	Mat greyMat, thresholdMat;
	double thresh = 50;
	double maxValue = 255;

	cvtColor(extractedImgs[1], greyMat, CV_BGR2GRAY);

	threshold(greyMat, thresholdMat, thresh, maxValue, THRESH_BINARY);
	

	imshow("GREY", masked);
	imshow("THRESHOLD", thresholdMat);


	waitKey(0);


	return 0;

}
