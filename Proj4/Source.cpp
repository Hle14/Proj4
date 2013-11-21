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

#define AFFINE_CHOICE 0
/*
1	select 3 matches w/ least eror
0	uniformly select 3 matches below error threshold at random
*/

using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{

	if( argc != 3 )
	{
		_tprintf(TEXT("Usage: %s [target_file]\n"), argv[0]);
		return 0;
	}

	Mat img1 = imread(argv[1], 1);
	Mat img2 = imread(argv[2], 1);

	if(img1.empty() || img2.empty())
	{
		printf("Can't read one of the images\n");
		return -1;
	}

	//MATCHING PHASE// //MATCHING PHASE// //MATCHING PHASE//

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
	FlannBasedMatcher matcher;
	vector<DMatch> matches;
	matcher.match(descriptors1, descriptors2, matches);

	float min = 10000;

	for(int i=0; i<matches.size(); i++)
	{
		if(matches[i].distance < min)
		{
			min = matches[i].distance;
		}
	}
	vector<DMatch> good_matches;
	for(int i=0; i<matches.size(); i++)
	{
		if(matches[i].distance <= 2*min)
		{
			good_matches.push_back(matches[i]);
		}
	}
	
	// drawing the results
	namedWindow("matches", 0);
	resizeWindow("matches",1280,360);
	Mat img_matches;
	drawMatches(img1, keypoints1, img2, keypoints2, good_matches, img_matches);
	imshow("matches", img_matches);
	imwrite("matches.jpg", img_matches);
	
	waitKey(0);

	//END OF MATCHING PHASE// //END OF MATCHING PHASE// //END OF MATCHING PHASE//
	
	//HOMOGRAPHY CALCULATION// //HOMOGRAPHY CALCULATION// //HOMOGRAPHY CALCULATION//
	vector<Point2f> pts_img1,pts_img2;

	for(int i=0; i < matches.size(); i++)
	{
		pts_img1.push_back(keypoints1[matches[i].queryIdx].pt);
		pts_img2.push_back(keypoints2[matches[i].trainIdx].pt);
	}

	Mat homography = findHomography(pts_img1,pts_img2,CV_RANSAC,3);

	cout << "H  = " << endl << " " << homography  << endl << endl;

	waitKey(0);
	printf("homography\n\t.rows : %d\n\t.cols : %d",homography.rows,homography.cols);

	Mat img1_transformed;
	warpPerspective(img1,img1_transformed,homography,img1.size(),INTER_LINEAR,0,0);

	double alpha = 0.5;
	double beta = 1.0 - alpha;
	
	Mat shifted;
	Mat compared;
	addWeighted(img1,alpha,img1_transformed,beta,0.0,shifted);
	addWeighted(img2,alpha,img1_transformed,beta,0.0,compared);


	namedWindow("transformed",0);
	resizeWindow("transformed",960,540);
	imshow("transformed",img1_transformed);
	namedWindow("shifted",0);
	resizeWindow("shifted",960,540);
	imshow("shifted",shifted);
	namedWindow("compared",0);
	resizeWindow("compared",960,540);
	imshow("compared",compared);

	imwrite("blend_h.jpg",compared);
	waitKey(0);

	//AFFINE TRANSFORM CALCULATION// //AFFINE TRANSFORM CALCULATION// //AFFINE TRANSFORM CALCULATION//

	//find 3 best matches and store matches-indices in an array
	DMatch tri_matches[3];

#if AFFINE_CHOICE

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
#elif !AFFINE_CHOICE
	//determine min error out of all matches
	min = matches[0].distance;
	float max = matches[0].distance;
	for(int i=1; i<matches.size(); i++)
	{
		if(matches[i].distance < min)
		{
			min = matches[i].distance;
		}
		if(matches[i].distance > max)
		{
			max = matches[i].distance;
		}
	}

	printf("\nmin = %f\n",min);

	int i=0;
	srand(time(NULL));
	while(true)
	{
		//randomly choose a match by index of match vector
		int idx = (int)(rand()%(matches.size()-1));
		
		if(matches[idx].distance < 2*min) //if match error is w/in acceptable limits, then store it in array of matches to calc transform from
		{
			if(i == 0)
			{
				tri_matches[0] = matches[idx];
				i++;
			} else if(i == 1)
			{
				tri_matches[1] = matches[idx];
				i++;
			} else if(i == 2)
			{
				tri_matches[2] = matches[idx];
				i++;
			} else
			{
				break;
			}
		}
	}
#endif

	waitKey(0);

	//END OF TRANSFORM CALCULATION// //END OF TRANSFORM CALCULATION// //END OF TRANSFORM CALCULATION//
	
	//WARP-PHASE// //WARP-PHASE// //WARP-PHASE// //WARP-PHASE//

	//store the best 3 matches from array to a vector to pass to drawMatches
	vector<DMatch> tri_vector;
	for(int i=0;i<3;i++)
	{
		tri_vector.push_back(tri_matches[i]);
	}

	//draw chosen matches between the two images
	Mat tri_force;
	drawMatches(img1, keypoints1, img2, keypoints2, tri_vector, tri_force);
	namedWindow("chosen matches",0);
	resizeWindow("chosen matches",1280,360);
	imshow("chosen matches", tri_force);
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
	
	Mat affine_transform = getAffineTransform(tri1,tri2);

	cout << "\naffine_transform  = " << endl << " " << affine_transform << endl << endl;

	//apply the transform to img1
	warpAffine(img1,img1_transformed,affine_transform,img1.size());

	//display the warped image
	namedWindow("affine_transform",0);
	resizeWindow("affine_transform",640,360);
	imshow("affine_transform",img1_transformed);

	//blend the warped image with original and also with target image
	Mat affine_shifted;
	addWeighted(img1,alpha,img1_transformed,beta,0.0,affine_shifted);
	Mat affine_compared;
	addWeighted(img2,alpha,img1_transformed,beta,0.0,affine_compared);

	//display the warped image blended with the original
	namedWindow("affine_shifted",0);
	resizeWindow("affine_shifted",960,540);
	imshow("affine_shifted",affine_shifted);

	//dispaly the warped image blended with target image
	namedWindow("affine_compared",0);
	resizeWindow("affine_compared",960,540);
	imshow("affine_compared",affine_compared);

	imwrite("blend_a.jpg",affine_compared);

	waitKey(0);

	//WARP-PHASE COMPLETED// //WARP-PHASE COMPLETED// //WARP-PHASE COMPLETED//
	return 0;
	//TERMINATING PROGRAM//
}


