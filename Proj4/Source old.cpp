#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\nonfree\nonfree.hpp>
#include <opencv2\nonfree\features2d.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <opencv2\legacy\legacy.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

int main(int argc,char* argv[])
{
	
	if(argc != 4 && argc!= 6)
	{
		printf("usage: %s [sobel_op] [l2grad] [target]\nor\nusage: %s [thresh_low] [thresh_high] [sobel_op] [l2grad] [target]",argv[0],argv[0]);

		return 0;
	}

	Mat input = imread(argc==4 ? argv[3] : argv[5],1);
	Mat edges;
	
	//use mean and sdev and lower and upper threshold values
	Scalar mean,sdev;
	meanStdDev(input,mean,sdev);
	double mu = mean[0];
	double sd = sdev[0];

	double t1 = argc==4 ? (mu - sd) : atof(argv[1]);
	double t2 = argc==4 ? (mu + sd) : atof(argv[2]);

	printf("\nt1 = %f\nt2 = %f",t1,t2);
	int s = argc==4 ? atoi(argv[1]) : atoi(argv[3]);
	int l2 = argc==4 ? atoi(argv[2]) : atoi(argv[4]);

	printf("\ns = %d\nl2 = %d",s,l2);

	Canny(input,edges,t1,t2,s,l2);
	
	namedWindow("window",1);
	imshow("window",edges);

	
	//write result image to new file with c_ prefix on original filename
	string output_name(argc==4 ? argv[3]:argv[5]);
	output_name.insert(0,"c_");
	const char* out_name = output_name.c_str();
	printf("\n%s",out_name);
	imwrite(out_name,edges);
	
	waitKey(0);

	return 0;
}
