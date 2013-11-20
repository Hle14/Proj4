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

void getFeatures(Mat& image,vector<KeyPoint>& keypoints,SurfFeatureDetector& det);

//void _tmain(int argc, TCHAR *argv[])
void _tmain(int argc, char *argv[])
{

	
	//prepare a file for writing data to
	//std::ofstream outFile("features.bin", ios::out | ios::binary | ios::app);
	std::ofstream outFile("features.txt", ios::out | ios::app);
	
	/*
	//prepare to read command line argument and convert to string
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	*/
	if( argc != 3 )
	{
		_tprintf(TEXT("Usage: %s [target_file]\n"), argv[0]);
		return;
	}

	_tprintf (TEXT("Target file is %s\n and \n%s"), argv[1], argv[2]);
	//hFind = FindFirstFile(argv[1], &FindFileData);
	/*
	if (hFind == INVALID_HANDLE_VALUE) 
	{
		printf ("FindFirstFile failed (%d)\n", GetLastError());
		waitKey(0);
		return;
	} else
	{*/
		//initialize necessary opencv modules
		initModule_nonfree();
		initModule_features2d();
		SurfFeatureDetector detector(500); //create feature detector object
		SurfDescriptorExtractor extractor();

		//SiftFeatureDetector detector(50,3,0.04,10.0,1.6);
	
		vector<KeyPoint> keypoints1, keypoints2;
		Mat imG1_features;
		Mat img2_features;

		/*
		//following 4 lines were copypasta'd from example on reading command line arguments and converting to string type
		char input_string[60];
		char DefChar = ' ';
		WideCharToMultiByte(CP_ACP,0,FindFileData.cFileName,-1, input_string,60,&DefChar, NULL);
    
		//A std:string  using the char* constructor.
		std::string ss(input_string);

		printf("\n%d: %s",0,ss);
		*/

		Mat img1 = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE); //load image
		Mat img2 = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

		//detector.detect(image,keypoints); //find keypoints and store in a vector
		//drawKeypoints(image,keypoints,image_features,Scalar::all(-1),DrawMatchesFlags::DRAW_RICH_KEYPOINTS); //create copy of image w/ keypoints drawn in

		getFeatures(img1,keypoints1,detector);
		getFeatures(img2,keypoints2,detector);
		
		int i = 1;

		char buffer[200];
		int n;
		
		for(int i=0;i<keypoints.size();i++)
		{
			n = sprintf(buffer, "%f %f %f %f %f %d %d\n",keypoints[i].pt.x,keypoints[i].pt.y,keypoints[i].size,keypoints[i].angle,keypoints[i].response,keypoints[i].octave,keypoints[i].class_id);
			outFile.write(buffer,n);
		}

		char ID[10];
		int IDsize;
		IDsize = sprintf(ID,"#%d\n",i);
		outFile.write(ID,IDsize);
		/*
		char marker[3] = "#\n";
		outFile.write(marker,sizeof(marker));
		*/

		waitKey(0);
		destroyAllWindows(); //attempt to free up memory
		
		int filecount = 1;
		
		//while loop to continue feature finding for the rest of the images in directory
		while(FindNextFile(hFind, &FindFileData) && filecount<100)
		{
			//following 2 commands convert filename to proper format
			WideCharToMultiByte(CP_ACP,0,FindFileData.cFileName,-1, input_string,260,&DefChar, NULL);
    		//A std:string  using the char* constructor.
			std::string ss(input_string);
			
			printf("\n%d: %s",i,input_string);
			
			image = imread(ss,-1); //load the image from the converted filename
			if (image.data == NULL) //if imread fails to load, attempt to use cvLoadImage instead
			{
				IplImage* pImg = cvLoadImage(input_string,-1);
				Mat temp(pImg);
				if(temp.data == NULL)
				{
					printf("DID NOT WORK!");
					n = sprintf(buffer,">>%s",ss);
					outFile.write(buffer,n);
					return;
				} else
				{
					printf("WORKED!!");
					image = temp;
				}

			}
			//process the image, assuming it was loaded
			detector.detect(image,keypoints);
			drawKeypoints(image,keypoints,image_features,Scalar::all(-1),DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
			namedWindow("TheWindow",CV_WINDOW_AUTOSIZE);
			imshow("TheWindow",image_features);
			
			for(int i=0;i<keypoints.size();i++)
			{
				n = sprintf(buffer, "%f %f %f %f %f %d %d\n",keypoints[i].pt.x,keypoints[i].pt.y,keypoints[i].size,keypoints[i].angle,keypoints[i].response,keypoints[i].octave,keypoints[i].class_id);
				outFile.write(buffer,n);
			}

			filecount++;

			char ID[5];
			int IDsize;
			IDsize = sprintf(ID,"#%d\n",filecount);
			outFile.write(ID,IDsize);

			waitKey(0);
			
		}
		
	FindClose(hFind);
   //}
}

void getFeatures(Mat& image,vector<KeyPoint>& keypoints,SurfFeatureDetector& det)
{
	Mat image_features;
	//initModule_nonfree();
	//initModule_features2d();
	det.detect(image,keypoints);
	drawKeypoints((image),(keypoints),image_features,Scalar::all(-1),DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	namedWindow("window",CV_WINDOW_AUTOSIZE);
	imshow("window",image_features);
	waitKey(0);
}

