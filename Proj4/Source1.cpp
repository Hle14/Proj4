#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <opencv2\legacy\legacy.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\nonfree\nonfree.hpp>
#include <opencv2\nonfree\features2d.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <fstream>
//#include <sys/stat.h> //for getting size of file


using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{

	if( argc != 3 )
	{
		_tprintf(TEXT("Usage: %s [target_file]\n"), argv[0]);
		return 0;
	}

	//_tprintf (TEXT("Target file is %s\n and \n%s"), argv[1], argv[2]);
	/*
	Mat img1 = imread(argv[1],CV_LOAD_IMAGE_GRAYSCALE);
	Mat img2 = imread(argv[2],CV_LOAD_IMAGE_GRAYSCALE);

	vector<KeyPoint> keypoints1, keypoints2;
	Mat img1_features, img2_features;
	Mat img1_descriptors, img2_descriptors;
	vector<DMatch> matches;
	Mat img_matches;

	initModule_nonfree();
	initModule_features2d();
	SurfFeatureDetector detector(500); //create feature detector object
	SurfDescriptorExtractor extractor;
	BruteForceMatcher<L2<float>> matcher;

	detector.detect(img1,keypoints1);
	detector.detect(img2,keypoints2);

	extractor.compute(img1,keypoints1,img1_descriptors);
	extractor.compute(img2,keypoints2,img2_descriptors);

	matcher.match(img1_descriptors,img2_descriptors,matches);


	namedWindow("matches", CV_WINDOW_AUTOSIZE);
	drawMatches(img1, keypoints1, img2, keypoints2, matches, img_matches);
	imshow("matches", img_matches);
	waitKey(0);
	*/

	Mat img1 = imread(argv[1], 1);
	Mat img2 = imread(argv[2], 1);

	if(img1.empty() || img2.empty())
	{
		printf("Can't read one of the images\n");
		return -1;
	}

	// detecting keypoints
	SurfFeatureDetector detector(400);
	vector<KeyPoint> keypoints1, keypoints2;
	detector.detect(img1, keypoints1);
	detector.detect(img2, keypoints2);

	// computing descriptors
	SurfDescriptorExtractor extractor;
	Mat descriptors1, descriptors2;
	extractor.compute(img1, keypoints1, descriptors1);
	extractor.compute(img2, keypoints2, descriptors2);

	// matching descriptors
	BruteForceMatcher<L2<float> > matcher;
	vector<DMatch> matches;
	matcher.match(descriptors1, descriptors2, matches);

	// drawing the results
	namedWindow("matches", 0);
	resizeWindow("matches",(img1.cols + img2.cols),max(img1.rows,img2.rows));
	Mat img_matches;
	drawMatches(img1, keypoints1, img2, keypoints2, matches, img_matches);
	imshow("matches", img_matches);
	waitKey(0);

	vector<Point2f> pts_img1,pts_img2;

	for(int i=0; i < matches.size(); i++)
	{
		pts_img1.push_back(keypoints1[matches[i].queryIdx].pt);
		pts_img2.push_back(keypoints2[matches[i].trainIdx].pt);
	}

	Mat homography = findHomography(pts_img1,pts_img2,CV_RANSAC,10);

	cout << "H  = " << endl << " " << homography  << endl << endl;

	waitKey(0);
	printf("homography\n\t.rows : %d\n\t.cols : %d",homography.rows,homography.cols);

	Mat img1_transformed;
	warpPerspective(img1,img1_transformed,homography,img1.size(),INTER_LINEAR,0,0);

	namedWindow("transformed",CV_WINDOW_AUTOSIZE);
	imshow("transformed",img1_transformed);

	//affine method//affine method//affine method//affine method//affine method
	
	//find 3 best matches and store matches-indices in an array
	DMatch tri_matches[3];

	float tri_match_dist[3];
	tri_match_dist[0] = 1000000;
	tri_match_dist[1] = 1000000;
	tri_match_dist[2] = 1000000;

	for(int i=0;i<matches.size();i++)
	{
		if(matches[i].distance < tri_match_dist[2])
		{
			if(matches[i].distance < tri_match_dist[1])
			{
				tri_match_dist[2] = tri_match_dist[1];
				tri_matches[2] = tri_matches[1];

				if(matches[i].distance < tri_match_dist[0])
				{
					tri_match_dist[1] = tri_match_dist[0];
					tri_matches[1] = tri_matches[0];
					tri_match_dist[0] = matches[i].distance;
					tri_matches[0] = matches[i];
				} else
				{
					tri_match_dist[1] = matches[i].distance;
					tri_matches[1] = matches[i];
				}
			} else
			{
				tri_match_dist[2] = matches[i].distance;
				tri_matches[2] = matches[i];
			}
		}
	}
	vector<DMatch> tri_vector;
	for(int i=0;i<3;i++)
	{
		tri_vector.push_back(tri_matches[i]);
	}

	Mat tri_force;
	drawMatches(img1, keypoints1, img2, keypoints2, tri_vector, tri_force);
	namedWindow("triforce",0);
	resizeWindow("triforce",1000,1000);
	imshow("triforce", tri_force);
	waitKey(0);

	Point2f tri1[3],tri2[3];

	//store x,y coordinates of triangle formed by best match points in 2 different arrays
	tri1[0] = keypoints1[tri_matches[0].queryIdx].pt;
	tri1[1] = keypoints1[tri_matches[1].queryIdx].pt;
	tri1[2] = keypoints1[tri_matches[2].queryIdx].pt;

	tri2[0] = keypoints2[tri_matches[0].trainIdx].pt;
	tri2[1] = keypoints2[tri_matches[1].trainIdx].pt;
	tri2[2] = keypoints2[tri_matches[2].trainIdx].pt;

	//calculate affine transformation of triangle1 to triangle2
	
	//GETTING THE AFFINE TRANSFORM BETWEEN THE IMAGES
	//Mat img1c = imread(argv[1],CV_32FC2);
	//Mat img2c = imread(argv[2],CV_32FC2);
	
	Mat affine_transform = getAffineTransform(tri1,tri2);

	cout << "\naffine_transform  = " << endl << " " << affine_transform << endl << endl;

	//apply the transform to img1

	warpAffine(img1,img1_transformed,affine_transform,img1.size());

	namedWindow("affine_transform",CV_WINDOW_AUTOSIZE);
	//resizeWindow("affine_transform",3000,img1_transformed.rows);
	imshow("affine_transform",img1_transformed);

	waitKey(0);
	return 0;
}


