// IC_AAM.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"

#include <iostream>
#include <fstream>

#include "cv.h"
#include "highgui.h"

#include "dirent.h"
#include "aam.h"
#include "delaunay.h"
//#include "aamTrainer.h"
#include "pca.h"
#include "pca_shape.h"

using namespace std;

#define SHAPE_SELECT_K_EIGENS 6		//	1~102

#define APPEARANCE_SELECT_K_EIGENS 92		//	1~105

//void aamTrainer::
void gendata()
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /*
	struct dirent *de=NULL;
    DIR *d=NULL;
    d=opendir("./images/");
    int k=0;

    while (de = readdir(d) )
    {
        if (strcmp(de->d_name+strlen(de->d_name)-3, "dat")==0 )
        {
            k++;
        }
    }
	*/

	FILE * fptr = 0; 
	char imgName[32];
	
	float x_point = 0, y_point = 0;
	char coma = 0;

	char* filename = "cctu_DB/TrainingData/norpoint.txt";
	char* filename_test = "cctu_DB/TestingData/norpoint.txt";

	// open the input file
	if (!(fptr = fopen(filename, "r"))) {
		fprintf(stderr, "Can't open file %s\n", filename);
	}

	//	load Images' count
	int k=0;//int nFaceImgs = 0;
	fscanf(fptr, "%d", &k);//fscanf(fptr, "%d", &nFaceImgs);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CvMat * meanXY = cvCreateMat(k,2, CV_64FC1 );
    CvMat * dataset = cvCreateMat(k,numberofpoints, CV_64FC2 );
    CvMat ** testWarpSet = (CvMat **)cvAlloc(k*sizeof(CvMat *));

    for (int i=0;i<k;i++)
        testWarpSet[i]=cvCreateMat(numberofpoints,2, CV_64FC1 );

    IplImage ** images = (IplImage **)cvAlloc(k*sizeof(IplImage *));

	int index=0;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*d=opendir("./images/");
    int index=0;
    while (de = readdir(d) )
    {

        if (strcmp(de->d_name+strlen(de->d_name)-3, "dat")==0 )
        {
            int j=0;
            int num=0;

            double meanx=0,meany=0;

            char path[200];
            sprintf(path,"./images/%s",de->d_name);
            char imagepath[200];
            sprintf(imagepath,"%s",path);
            imagepath[strlen(imagepath)-4]='\0';
            sprintf(imagepath,"%s.jpg",imagepath);

            FILE *fp,*wfp;
            float n,shape[totalnumberofpoints];
            double xavg=0,yavg=0;
            if (fp=fopen(path,"r"))
            {
                images[index]=cvLoadImage(imagepath,0);
                while ( ! feof( fp ) )
                {
                    CvScalar s;
                    s.val[0]=0;
                    s.val[1]=0;
                    s.val[2]=0;
                    fscanf(fp,"%f %f %f\n",&n,&shape[j],&shape[j+1]);	//	存取.dat檔的座標位置資料
                    s.val[0]=shape[j];
                    s.val[1]=shape[j+1];
                    meanx+=s.val[0];	// 把各個.dat檔的座標位置相加累計
                    meany+=s.val[1];
                    cvSet2D(  dataset, index,num, s);	//	製作單張 annotated face shape 影像



                    CvScalar t;
                    t.val[0]=s.val[0];
                    cvSet2D(testWarpSet[index],num,0,t);
                    t.val[0]=s.val[1];
                    cvSet2D(testWarpSet[index],num,1,t);

                    num++;
                    j+=2;



                }
                fclose(fp);

            }


            CvScalar s;
            s.val[0]=meanx/numberofpoints;
            cvSet2D(  meanXY, index,0, s);
            s.val[0]=meany/numberofpoints;
            cvSet2D(  meanXY, index,1, s);

            index++;

        }
    }
	*/

	// load the txt file
	//CvMat* dataSet = cvCreateMat(nFaceImgs, 51 * 2, CV_32FC1);	// total number of Sample = 51 

	double meanx, meany;

	for (int iFace = 0; iFace < k; iFace++) {

		meanx = 0, meany = 0;

		fscanf(fptr, "%s", imgName);

		images[iFace]=cvLoadImage(imgName,0);

		for(int iPoint = 0; iPoint < 51; iPoint ++){

			CvScalar s;
            s.val[0]=0;
            s.val[1]=0;
            s.val[2]=0;

			fscanf(fptr, "%f%c%f", &x_point, &coma, &y_point);

			s.val[0] = x_point;
            s.val[1] = y_point;

			meanx+=s.val[0];	// 把各個.dat檔的座標位置相加累計
            meany+=s.val[1];

			//printf("x = %f, y = %f\n", s.val[0], s.val[1]);
			cvSet2D(dataset, iFace, iPoint, s);

			CvScalar t;
            t.val[0] = s.val[0];
            cvSet2D(testWarpSet[iFace], iPoint, 0, t);
            t.val[0] = s.val[1];
            cvSet2D(testWarpSet[iFace], iPoint, 1, t);
		}
		//printf("==========face%d=============\n", iFace + 2);

		CvScalar s;
        s.val[0] = meanx / numberofpoints;
        cvSet2D(  meanXY, iFace, 0, s);
        s.val[0] = meany / numberofpoints;
        cvSet2D(  meanXY, iFace, 1, s);
	}

	fclose(fptr);

	/////////////////////////////////////////////////////////////////////////////////////////////
    double avNorm=0;
    for (index=0;index<k;index++)	//	preProcess (linear alignment : zero mean)
    {
        for (int i=0;i<numberofpoints;i++)
        {
            CvScalar s,m1,m2;
            s=    cvGet2D(dataset, index,i);
            m1=    cvGet2D(meanXY, index,0);
            m2=    cvGet2D(meanXY, index,1);

            s.val[0] -= m1.val[0];
            s.val[1] -= m2.val[0];
            cvSet2D(  dataset, index,i, s);
        }
    }

    CvMat * x1Tx2 = cvCreateMat(2,2, CV_64FC1 );
    CvMat * x1T = cvCreateMat(2,numberofpoints, CV_64FC1 );
    CvMat * x2 = cvCreateMat(numberofpoints,2, CV_64FC1 );
    CvMat * UT= cvCreateMat(2,2, CV_64FC1 );
    CvMat * V = cvCreateMat(2,2, CV_64FC1 );
    CvMat * tempW = cvCreateMat(2,2, CV_64FC1 );
    CvMat * VUT = cvCreateMat(2,2, CV_64FC1 );
    CvMat * average = cvCreateMat(numberofpoints,2, CV_64FC1 );


    for (int i=0;i<numberofpoints;i++)	// face1(已zero mean)的 land mark point
    {
        CvScalar s;
        s= cvGet2D(dataset,0,i);
        CvScalar m;
        m.val[0]=s.val[0];
        cvSet2D(  x2, i,0, m);
        m.val[0]=s.val[1];
        cvSet2D(  x2, i,1, m);
    }

    for (int iteration=0;iteration<5;iteration++)
    {
        for (index=0;index<k;index++)	// face1~k(已zero mean)的 land mark point
        {
			//printf("========Face%d=========\n", k);
            for (int i=0;i<numberofpoints;i++)	//	generate X1T (row:2 , col:63)
            {
                CvScalar s;
                s= cvGet2D(dataset,index,i);
				//printf("x = %lf, y = %lf\n", s.val[0], s.val[1]);
                CvScalar m;
                m.val[0]=s.val[0];
                cvSet2D(  x1T,0, i, m);
                m.val[0]=s.val[1];
                cvSet2D(  x1T,1, i, m);
            }

            cvMatMul(x1T,x2,x1Tx2);
            cvSVD( x1Tx2, tempW, UT, V,CV_SVD_U_T);
            cvMatMul(V,UT,VUT);

			//printf("\n\n\n\n");
            for (int i = 0; i < numberofpoints; i ++)
            {
                CvScalar s;
                s= cvGet2D(dataset,index,i);
				
                CvScalar v1,v2,v3,v4;
                v1= cvGet2D(VUT,0,0);
                v2= cvGet2D(VUT,0,1);
                v3= cvGet2D(VUT,1,0);
                v4= cvGet2D(VUT,1,1);
                double k1 = v1.val[0] * s.val[0] + v2.val[0] * s.val[1];
                double k2 = v3.val[0] * s.val[0] + v4.val[0] * s.val[1];
                s.val[0] = k1;
                s.val[1] = k2;
				//printf("x_k = %lf, y_k = %lf\n", s.val[0], s.val[1]);
                cvSet2D(dataset,index,i,s);

            }

        }
        cvSetZero(average);

        for (int i=0;i<numberofpoints;i++)
        {
            CvScalar s,avx,avy;
            s= cvGet2D(dataset,0,i);
            avx= cvGet2D(average,i,0);
            avy= cvGet2D(average,i,1);
            avx.val[0]+=s.val[0];
            avy.val[0]+=s.val[1];
            cvSet2D(average,i,0,avx);
            cvSet2D(average,i,1,avy);
        }
        for (index=1;index<k;index++)	//	共 k 張 的訓練影像
        {
            for (int i=0;i<numberofpoints;i++)
            {
                CvScalar s,avx,avy;
                s= cvGet2D(dataset,index,i);
                avx= cvGet2D(average,i,0);
                avy= cvGet2D(average,i,1);
                avx.val[0]= (avx.val[0]*index + s.val[0])/(index+1);
                avy.val[0]= (avy.val[0]*index + s.val[1])/(index+1);
                cvSet2D(average,i,0,avx);
                cvSet2D(average,i,1,avy);
            }
        }

    }

    CvFileStorage *fs;
    fs = cvOpenFileStorage( "./alignedset.aam", 0, CV_STORAGE_WRITE );
    cvWrite( fs, "shape",dataset, cvAttrList(0,0) );
    cvReleaseFileStorage( &fs );

    CvMat **shapeAlignedSet =(CvMat**)cvAlloc(sizeof(CvMat*) * k);	//	共 k 張 的訓練影像

    for (int i=0;i<k;i++)	//	共 k 張 的訓練影像
    {
        shapeAlignedSet[i]=cvCreateMat(1,totalnumberofpoints,CV_64FC1);

        for (int m=0;m<numberofpoints;m++)
        {
            CvScalar v;
            v= cvGet2D(dataset,i,m);

			CvScalar s1,s2;
            s1.val[0]=v.val[0];
            s2.val[0]=v.val[1];
            cvSet2D(shapeAlignedSet[i],0,2*m,s1);
            cvSet2D(shapeAlignedSet[i],0,(2*m)+1,s2);
        }
    }

    double norm=0;
    for (int i=0;i<numberofpoints;i++)	//	(sum of (entries^2))^0.5
    {
        CvScalar avx,avy;
        avx= cvGet2D(average,i,0);
        avy= cvGet2D(average,i,1);

        norm+=pow(avx.val[0],2)+pow(avy.val[0],2);

    }


    norm=sqrt(norm);

    for (int i=0;i<k;i++)
    {
        double newNorm=0;

        for (int m=0;m<numberofpoints;m++)
        {
            CvScalar avx,avy;

            avx=cvGet2D(shapeAlignedSet[i],0,2*m);
            avy=cvGet2D(shapeAlignedSet[i],0,(2*m)+1);

            newNorm+=pow(avx.val[0],2)+pow(avy.val[0],2);
        }

        newNorm=sqrt(newNorm);
        for (int m=0;m<numberofpoints;m++)
        {
            CvScalar avx,avy;

            avx= cvGet2D(shapeAlignedSet[i],0,2*m);
            avy=cvGet2D(shapeAlignedSet[i],0,(2*m)+1);
	            avx.val[0]*=norm/newNorm;
	            avy.val[0]*=norm/newNorm;
            cvSet2D(shapeAlignedSet[i],0,2*m,avx);
            cvSet2D(shapeAlignedSet[i],0,(2*m)+1,avy);
        }

    }
    IplImage *imgPoints =cvCreateImage(cvSize(120,170),IPL_DEPTH_8U,3);

    cvSetZero(imgPoints);

    for (int m=0;m<numberofpoints;m++)
    {
        CvScalar avx,avy;	

        avx= cvGet2D(average,m,0);
        avy= cvGet2D(average,m,1);
        cvCircle( imgPoints, cvPoint(avx.val[0]+100,avy.val[0]+100),1, CV_RGB(255,255,255),2 );

    }
#ifdef DEBUG
    //cvNamedWindow("warped1", CV_WINDOW_AUTOSIZE);
    //cvShowImage("warped1",imgPoints);
    //cvWaitKey(-1);	//int cvWaitKey( int delay=0 ) 返回值?int型，函?的???int型，?delay小于等于0的?候，如果?有??触?，?一直等待
    for (int i=0;i<k;i++)
    {
        double newNorm=0;
        cvSetZero(imgPoints);

        for (int m=0;m<numberofpoints;m++)
        {
            CvScalar avx,avy;

            avx=cvGet2D(shapeAlignedSet[i],0,2*m);
            avy=cvGet2D(shapeAlignedSet[i],0,(2*m)+1);

//            cvCircle( imgPoints, cvPoint(avx.val[0]+100,avy.val[0]+100),1, CV_RGB(255,255, 0),2 );//CV_RGB(255,255,i*20),2 );
			cvCircle( imgPoints, cvPoint(avx.val[0] + 58, avy.val[0] + 100),1, CV_RGB(255,255, 0),2 );//CV_RGB(255,255,i*20),2 );
        }

//        cvNamedWindow("warped1",1);
//        cvShowImage("warped1",imgPoints);
//        cvWaitKey(-1);
		
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		char shapeName[80];
		sprintf(shapeName, "Face Shape/%d.jpg",  i+1);
		cvSaveImage(shapeName, imgPoints);
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    }
#endif


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////// Shape Alignment For Testing Images ////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	FILE * fptr_test = 0; 
	char imgName_test[32];
	
	float x_point_test = 0, y_point_test = 0;
	char coma_test = 0;

//	char* filename = "cctu_DB/TrainingData/norpoint.txt";
	//char* filename_test = "cctu_DB/TestingData/norpoint.txt";
	// open the input file
	if (!(fptr_test = fopen(filename_test, "r"))) {
		printf("Can't open Testing Faces File %s\n", filename_test);
	}

	//	load Images' count
	int k_test = 0;//int nFaceImgs = 0;
	fscanf(fptr_test, "%d", &k_test);//fscanf(fptr, "%d", &nFaceImgs);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CvMat * meanXY_test = cvCreateMat(k_test, 2, CV_64FC1 );
    CvMat * dataset_test = cvCreateMat(k_test, numberofpoints, CV_64FC2 );
    CvMat ** testWarpSet_test = (CvMat **)cvAlloc(k_test * sizeof(CvMat *));

    for (int i = 0; i < k_test; i ++)
        testWarpSet_test[i] = cvCreateMat(numberofpoints, 2, CV_64FC1 );

    IplImage ** images_test = (IplImage **)cvAlloc(k_test * sizeof(IplImage *));

	int index_test = 0;

	// load the txt file
	//CvMat* dataSet = cvCreateMat(nFaceImgs, 51 * 2, CV_32FC1);	// total number of Sample = 51 

	double meanx_test, meany_test;

	for (int iFace = 0; iFace < k_test; iFace++) {

		meanx_test = 0, meany_test = 0;

		fscanf(fptr_test, "%s", imgName_test);

		images_test[iFace]=cvLoadImage(imgName_test,0);

		for(int iPoint = 0; iPoint < 51; iPoint ++){

			CvScalar s;
            s.val[0]=0;
            s.val[1]=0;
            s.val[2]=0;

			fscanf(fptr_test, "%f%c%f", &x_point_test, &coma_test, &y_point_test);

			s.val[0] = x_point_test;
            s.val[1] = y_point_test;

			meanx_test += s.val[0];	// 把各個.dat檔的座標位置相加累計
            meany_test += s.val[1];

			//printf("x = %f, y = %f\n", s.val[0], s.val[1]);
			cvSet2D(dataset_test, iFace, iPoint, s);

			CvScalar t;
            t.val[0] = s.val[0];
            cvSet2D(testWarpSet_test[iFace], iPoint, 0, t);
            t.val[0] = s.val[1];
            cvSet2D(testWarpSet_test[iFace], iPoint, 1, t);
		}
		//printf("==========face%d=============\n", iFace + 2);

		CvScalar s;
        s.val[0] = meanx_test / numberofpoints;
        cvSet2D(  meanXY_test, iFace, 0, s);
        s.val[0] = meany / numberofpoints;
        cvSet2D(  meanXY_test, iFace, 1, s);
	}

	fclose(fptr_test);

	/////////////////////////////////////////////////////////////////////////////////////////////
    double avNorm_test = 0;
    for (index = 0;index < k_test; index ++)	//	preProcess (linear alignment : zero mean)
    {
        for (int i = 0; i < numberofpoints; i ++)
        {
            CvScalar s,m1,m2;
            s =    cvGet2D(dataset_test, index, i);
            m1 =    cvGet2D(meanXY_test, index,0);
            m2 =    cvGet2D(meanXY_test, index,1);

            s.val[0] -= m1.val[0];
            s.val[1] -= m2.val[0];
            cvSet2D(  dataset_test, index,i, s);
        }
    }

    CvMat * x1Tx2_test = cvCreateMat(2,2, CV_64FC1 );
    CvMat * x1T_test = cvCreateMat(2,numberofpoints, CV_64FC1 );
    CvMat * x2_test = cvCreateMat(numberofpoints,2, CV_64FC1 );
    CvMat * UT_test= cvCreateMat(2,2, CV_64FC1 );
    CvMat * V_test = cvCreateMat(2,2, CV_64FC1 );
    CvMat * tempW_test = cvCreateMat(2,2, CV_64FC1 );
    CvMat * VUT_test = cvCreateMat(2,2, CV_64FC1 );
    CvMat * average_test = cvCreateMat(numberofpoints,2, CV_64FC1 );


    for (int i = 0; i < numberofpoints; i ++)	// face1(已zero mean)的 land mark point
    {
        CvScalar s;
        s= cvGet2D(dataset_test,0,i);
        CvScalar m;
        m.val[0]=s.val[0];
        cvSet2D(  x2_test, i,0, m);
        m.val[0]=s.val[1];
        cvSet2D(  x2_test, i,1, m);
    }

    for (int iteration = 0; iteration < 5; iteration ++)
    {
        for (index = 0; index < k_test; index ++)	// face1~k(已zero mean)的 land mark point
        {
			//printf("========Face%d=========\n", k);
            for (int i = 0; i < numberofpoints; i ++)	//	generate X1T (row:2 , col:63)
            {
                CvScalar s;
                s= cvGet2D(dataset_test, index, i);
				//printf("x = %lf, y = %lf\n", s.val[0], s.val[1]);
                CvScalar m;
                m.val[0] = s.val[0];
                cvSet2D(  x1T_test,0, i, m);
                m.val[0] = s.val[1];
                cvSet2D(  x1T_test,1, i, m);
            }

            cvMatMul(x1T_test, x2_test, x1Tx2_test);
            cvSVD( x1Tx2_test, tempW_test, UT_test, V_test,CV_SVD_U_T);
            cvMatMul(V_test, UT_test, VUT_test);

			//printf("\n\n\n\n");
            for (int i = 0; i < numberofpoints; i ++)
            {
                CvScalar s;
                s= cvGet2D(dataset_test, index, i);
				
                CvScalar v1,v2,v3,v4;
                v1= cvGet2D(VUT_test, 0, 0);
                v2= cvGet2D(VUT_test, 0, 1);
                v3= cvGet2D(VUT_test, 1, 0);
                v4= cvGet2D(VUT_test, 1, 1);
                double k1 = v1.val[0] * s.val[0] + v2.val[0] * s.val[1];
                double k2 = v3.val[0] * s.val[0] + v4.val[0] * s.val[1];
                s.val[0] = k1;
                s.val[1] = k2;
				//printf("x_k = %lf, y_k = %lf\n", s.val[0], s.val[1]);
                cvSet2D(dataset_test, index, i, s);

            }

        }
        cvSetZero(average_test);

        for (int i = 0; i < numberofpoints; i ++)
        {
            CvScalar s, avx, avy;
            s= cvGet2D(dataset_test, 0, i);
            avx= cvGet2D(average_test, i, 0);
            avy= cvGet2D(average_test, i, 1);
            avx.val[0]+=s.val[0];
            avy.val[0]+=s.val[1];
            cvSet2D(average_test, i, 0, avx);
            cvSet2D(average_test, i, 1, avy);
        }
        for (index = 1;index < k_test; index ++)	//	共 k 張 的訓練影像
        {
            for (int i = 0;i < numberofpoints; i ++)
            {
                CvScalar s, avx, avy;
                s = cvGet2D(dataset_test, index, i);
                avx = cvGet2D(average_test, i, 0);
                avy = cvGet2D(average_test, i, 1);
                avx.val[0]= (avx.val[0] * index + s.val[0]) / (index + 1);
                avy.val[0]= (avy.val[0] * index + s.val[1]) / (index + 1);
                cvSet2D(average_test, i, 0, avx);
                cvSet2D(average_test, i, 1, avy);
            }
        }

    }

    //CvFileStorage *fs_test;
    //fs_test = cvOpenFileStorage( "./alignedset.aam", 0, CV_STORAGE_WRITE );
    //cvWrite( fs_test, "shape",dataset_test, cvAttrList(0,0) );
    //cvReleaseFileStorage( &fs_test );

    CvMat **shapeAlignedSet_test =(CvMat**)cvAlloc(sizeof(CvMat*) * k_test);	//	共 k 張 的訓練影像

    for (int i = 0; i < k_test; i ++)	//	共 k 張 的訓練影像
    {
        shapeAlignedSet_test[i]=cvCreateMat(1,totalnumberofpoints,CV_64FC1);

        for (int m = 0; m < numberofpoints; m ++)
        {
            CvScalar v;
            v= cvGet2D(dataset_test,i,m);

			CvScalar s1,s2;
            s1.val[0]=v.val[0];
            s2.val[0]=v.val[1];
            cvSet2D(shapeAlignedSet_test[i],0,2*m,s1);
            cvSet2D(shapeAlignedSet_test[i],0,(2*m)+1,s2);
        }
    }

    double norm_test = 0;
    for (int i = 0 ; i < numberofpoints; i ++)	//	(sum of (entries^2))^0.5
    {
        CvScalar avx,avy;
        avx= cvGet2D(average_test, i, 0);
        avy= cvGet2D(average_test, i, 1);

        norm_test += pow(avx.val[0],2)+pow(avy.val[0],2);

    }


    norm_test = sqrt(norm_test);

    for (int i = 0; i < k_test; i ++)
    {
        double newNorm_test=0;

        for (int m=0;m<numberofpoints;m++)
        {
            CvScalar avx,avy;

            avx=cvGet2D(shapeAlignedSet_test[i],0,2*m);
            avy=cvGet2D(shapeAlignedSet_test[i],0,(2*m)+1);

            newNorm_test+=pow(avx.val[0],2)+pow(avy.val[0],2);
        }

        newNorm_test = sqrt(newNorm_test);
        for (int m=0;m<numberofpoints;m++)
        {
            CvScalar avx,avy;

            avx= cvGet2D(shapeAlignedSet_test[i],0,2*m);
            avy=cvGet2D(shapeAlignedSet_test[i],0,(2*m)+1);
	            avx.val[0] *= norm_test / newNorm_test;
	            avy.val[0] *= norm_test / newNorm_test;
            cvSet2D(shapeAlignedSet_test[i],0,2*m,avx);
            cvSet2D(shapeAlignedSet_test[i],0,(2*m)+1,avy);
        }

    }
    IplImage *imgPoints_test =cvCreateImage(cvSize(120,170), IPL_DEPTH_8U, 3);

    cvSetZero(imgPoints_test);

    for (int m = 0; m < numberofpoints; m ++)
    {
        CvScalar avx,avy;	

        avx= cvGet2D(average_test, m, 0);
        avy= cvGet2D(average_test, m, 1);
        cvCircle( imgPoints_test, cvPoint(avx.val[0]+100,avy.val[0]+100), 1, CV_RGB(255,255,255), 2 );

    }
#ifdef DEBUG
    //cvNamedWindow("warped1", CV_WINDOW_AUTOSIZE);
    //cvShowImage("warped1",imgPoints);
    //cvWaitKey(-1);	//int cvWaitKey( int delay=0 ) 返回值?int型，函?的???int型，?delay小于等于0的?候，如果?有??触?，?一直等待
    
	for (int i = 0; i < k_test; i ++)
    {
        double newNorm_test = 0;
        cvSetZero(imgPoints_test);

        for (int m=0;m<numberofpoints;m++)
        {
            CvScalar avx,avy;

            avx=cvGet2D(shapeAlignedSet_test[i],0,2*m);
            avy=cvGet2D(shapeAlignedSet_test[i],0,(2*m)+1);

//            cvCircle( imgPoints, cvPoint(avx.val[0]+100,avy.val[0]+100),1, CV_RGB(255,255, 0),2 );//CV_RGB(255,255,i*20),2 );
			cvCircle( imgPoints_test, cvPoint(avx.val[0] + 58, avy.val[0] + 90),1, CV_RGB(255,255, 0),2 );//CV_RGB(255,255,i*20),2 );
        }

//        cvNamedWindow("warped1",1);
//        cvShowImage("warped1",imgPoints);
//        cvWaitKey(-1);
		
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		char shapeName_test[80];
		sprintf(shapeName_test, "Test Face Shape/%d.jpg",  i+1);
		cvSaveImage(shapeName_test, imgPoints_test);
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    }
#endif




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



    PCA_AAM  newPCAShape;
    PCA_AAM newPCAAppearance;
    newPCAShape.runPCA(shapeAlignedSet, meanXY, k, SHAPE_SELECT_K_EIGENS, shapeAlignedSet_test, meanXY_test, k_test);

    CvMat *shape ;
    CvMat ** warpedImage = (CvMat **)cvAlloc(k*sizeof(CvMat *));
    shape= cvCreateMat(average->height,average->width, CV_64FC1);
    shape = cvCloneMat(average);

	IplImage **texture =  (IplImage **)cvAlloc(k*sizeof(IplImage *));

    if (shape!=NULL)
    {
        delaunay newDelaunay(shape);	//	計算 Triangle 的 3 個 座標點 儲存於 矩陣 Ai & 計算 Affine (Inverse_Ai)
        IplImage *test =cvCreateImage(cvSize(newDelaunay.width, newDelaunay.height),IPL_DEPTH_8U,1);
        cvSetZero(test);

        for (int m=0;m<k;m++)
            warpedImage[m] =  cvCreateMat(newDelaunay.height, newDelaunay.width,CV_64FC1);

        for (int m=0;m<k;m++)
        {
            cvSetZero(test);


            cvSetZero(warpedImage[m]);
            int t = newDelaunay.numberOfPixels();	//	return totalNumberOfPixels;  即 Triangle 個數
            newDelaunay.calculateAffineWarpParameters(testWarpSet[m]);	//	calculateAffineWarpParameters

            for (int i=0;i<t;i++)	//	t = totalNumberOfPixels;
            {
                pixel * pix = newDelaunay.getpixel(i);	//	取得該[特徵點]的 warp，所對應的參數

                pixel * pix2 = newDelaunay.findCorrespondingPixelInImage(pix);	//	找出 對應點 的 " X 座標(pix2->x) " 和 " Y 座標(pix2->y) "

                float imageValue = interpolate<uchar>(images[m],float(pix2->x),float(pix2->y));	//	Get interpolated_value

                CvScalar value;
                value.val[0]=imageValue;
                if (floor(pix->y+pix->ty)>=0 && floor(pix->x+pix->tx)>=0)	
					//	double floor (double x); : Rounds x downward, returning the largest integral value that is not greater than x.
                {
                    cvSet2D(test,(int)floor(pix->y+pix->ty),(int)floor(pix->x+pix->tx),value);
                    cvSet2D(warpedImage[m],(int)floor(pix->y+pix->ty),(int)floor(pix->x+pix->tx),value);
                }


            }

#ifdef DEBUG
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//            cvNamedWindow("Warped Image",CV_WINDOW_AUTOSIZE);
//            cvShowImage("Warped Image",test);
//            cvWaitKey(-1);

			char textureName[80];
			sprintf(textureName, "Face Appearance/%d.jpg",  m+1);
			cvSaveImage(textureName, test);

			texture[m] = test;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

        }

    }
    cvReleaseMat(&dataset);
    for (int i=0;i<k;i++)	//	共 k 張 的訓練影像
    {
        cvReleaseMat(&testWarpSet[i]);
        cvReleaseMat(&shapeAlignedSet[i]);
    }




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////// Appearance Alignment For Testing Images ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    CvMat *shape_test ;
    CvMat **  warpedImage_test = (CvMat **)cvAlloc(k_test * sizeof(CvMat *));
    shape_test = cvCreateMat(average_test->height,average_test->width, CV_64FC1);
    shape_test = cvCloneMat(average_test);

	IplImage ** texture_test =  (IplImage **)cvAlloc(k*sizeof(IplImage *));

    if (shape_test!=NULL)
    {
        delaunay newDelaunay_test(shape_test);	//	計算 Triangle 的 3 個 座標點 儲存於 矩陣 Ai & 計算 Affine (Inverse_Ai)
        IplImage *test_test = cvCreateImage(cvSize(newDelaunay_test.width, newDelaunay_test.height),IPL_DEPTH_8U,1);

        cvSetZero(test_test);

        for (int m = 0; m < k_test; m ++)
			warpedImage_test[m] =  cvCreateMat(warpedImage[0]->height, warpedImage[0]->width, CV_64FC1);
													//cvCreateMat(newDelaunay_test.height, newDelaunay_test.width,CV_64FC1);

        for (int m = 0; m < k_test; m ++)
        {
            cvSetZero(test_test);


            cvSetZero(warpedImage_test[m]);
            int t_test = newDelaunay_test.numberOfPixels();	//	return totalNumberOfPixels;  即 Triangle 個數
            newDelaunay_test.calculateAffineWarpParameters(testWarpSet_test[m]);	//	calculateAffineWarpParameters

            for (int i=0;i<t_test;i++)	//	t = totalNumberOfPixels;
            {
                pixel * pix_test = newDelaunay_test.getpixel(i);	//	取得該[特徵點]的 warp，所對應的參數

                pixel * pix2_test = newDelaunay_test.findCorrespondingPixelInImage(pix_test);	
							//	找出 對應點 的 " X 座標(pix2->x) " 和 " Y 座標(pix2->y) "

                float imageValue_test = interpolate<uchar>(images_test[m], float(pix2_test->x), float(pix2_test->y));	//	Get interpolated_value

                CvScalar value;
                value.val[0] = imageValue_test;
                if (floor(pix_test->y + pix_test->ty) >= 0 && floor(pix_test->x + pix_test->tx) >= 0)	
						//	double floor (double x); : Rounds x downward, returning the largest integral value that is not greater than x.
                {
                    cvSet2D(test_test, (int)floor(pix_test->y + pix_test->ty), (int)floor(pix_test->x + pix_test->tx), value);
                    cvSet2D(warpedImage_test[m], (int)floor(pix_test->y + pix_test->ty), (int)floor(pix_test->x + pix_test->tx), value);
                }


            }

#ifdef DEBUG
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//            cvNamedWindow("Warped Image",CV_WINDOW_AUTOSIZE);
//            cvShowImage("Warped Image",test);
//            cvWaitKey(-1);

			char textureName_test[80];
			sprintf(textureName_test, "Test Face Appearance/%d.jpg",  m+1);
			cvSaveImage(textureName_test, test_test);

			texture_test[m] = test_test;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

        }

    }
    cvReleaseMat(&dataset_test);
    for (int i = 0; i < k_test; i ++)	//	共 k 張 的訓練影像
    {
        cvReleaseMat(&testWarpSet_test[i]);
        cvReleaseMat(&shapeAlignedSet_test[i]);
    }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    newPCAAppearance.runPCA(warpedImage, meanXY, k, APPEARANCE_SELECT_K_EIGENS, warpedImage_test, meanXY_test, k_test);	
			//	1.  2. k : 共 k 張 的訓練影像，得 1.Eigen Vector 2.Eigen Value 3.averageInput
    eigenVectors_AAM *app = newPCAAppearance.returnEigens();	//	Get Eigens of Appearance
    eigenVectors_AAM *shapes = newPCAShape.returnEigens();	//	Get Eigens of Shape

	//////////////////////////////////////////////////////Store In The File Storage//////////////////////////////////////////////////////
	fs = cvOpenFileStorage( "./aam.template", 0, CV_STORAGE_WRITE );
    for (int m=0;m<app->count;m++)
    {
        char temp[200];
        if(app->eigens[m]!=NULL)
        {
        sprintf(temp,"appEigenVectors%d",m);
        cvWrite( fs,temp ,app->eigens[m], cvAttrList(0,0) );
    }}

    for (int m=0;m<shapes->count;m++)
    {
        char temp[200];
        if(shapes->eigens[m]!=NULL)
        {
        sprintf(temp,"shapeEigenVectors%d",m);
        cvWrite( fs,temp ,shapes->eigens[m], cvAttrList(0,0) );
    }}

    IplImage *avgShape=newPCAShape.returnAverage();	//	Get averageInput
    IplImage *avgApp=newPCAAppearance.returnAverage();	//	Get averageInput
    
	for (int i=0;i<avgShape->width;i++)
    {
        for (int j=0;j<avgShape->height;j++)
        {

            CvScalar s1,s2;	//	s2 ? 
            s1=cvGet2D(average,(i/2),i%2);
            cvSet2D(avgShape,j,i,s1);

        }
    }
    
	cvWrite( fs,"avgApp" ,avgApp, cvAttrList(0,0) );
    cvWrite( fs,"avgShape" ,avgShape, cvAttrList(0,0) );

    cvWriteReal( fs, "numberOfAppearanceEigenVectors",app->count);
    cvWriteReal( fs, "numberOfShapeEigenVectors",shapes->count);

    CvMat *eigenVal=newPCAShape.returnEigenVals();
	cvWrite( fs,"eigenVal" ,eigenVal, cvAttrList(0,0) );
    
	cvReleaseFileStorage( &fs );

printf("Training Completed \n\n");

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////// Using The Training Faces' Weights To Rescrount The Testing Faces //////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//	FILE * fptr_test = 0; 
//	char imgName_test[32];
//	
//	float x_point_test = 0, y_point_test = 0;
//	char coma_test = 0;
//
////	char* filename = "cctu_DB/TrainingData/norpoint.txt";
//	char* filename_test = "cctu_DB/TestingData/norpoint.txt";
//	// open the input file
//	if (!(fptr_test = fopen(filename_test, "r"))) {
//		printf("Can't open Testing Faces File %s\n", filename_test);
//	}
//
//	//	load Images' count
//	int k_test = 0;//int nFaceImgs = 0;
//	fscanf(fptr_test, "%d", &k_test);//fscanf(fptr, "%d", &nFaceImgs);
//	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    CvMat * meanXY_test = cvCreateMat(k_test, 2, CV_64FC1 );
//    CvMat * dataset_test = cvCreateMat(k_test, numberofpoints, CV_64FC2 );
//    CvMat ** testWarpSet_test = (CvMat **)cvAlloc(k_test * sizeof(CvMat *));
//
//    for (int i = 0; i < k_test; i ++)
//        testWarpSet_test[i] = cvCreateMat(numberofpoints, 2, CV_64FC1 );
//
//    IplImage ** images_test = (IplImage **)cvAlloc(k_test * sizeof(IplImage *));
//
//	int index_test = 0;
//
//	// load the txt file
//	//CvMat* dataSet = cvCreateMat(nFaceImgs, 51 * 2, CV_32FC1);	// total number of Sample = 51 
//
//	double meanx_test, meany_test;
//
//	for (int iFace = 0; iFace < k_test; iFace++) {
//
//		meanx_test = 0, meany_test = 0;
//
//		fscanf(fptr_test, "%s", imgName_test);
//
//		images_test[iFace]=cvLoadImage(imgName_test,0);
//
//		for(int iPoint = 0; iPoint < 51; iPoint ++){
//
//			CvScalar s;
//            s.val[0]=0;
//            s.val[1]=0;
//            s.val[2]=0;
//
//			fscanf(fptr_test, "%f%c%f", &x_point_test, &coma_test, &y_point_test);
//
//			s.val[0] = x_point_test;
//            s.val[1] = y_point_test;
//
//			meanx_test += s.val[0];	// 把各個.dat檔的座標位置相加累計
//            meany_test += s.val[1];
//
//			//printf("x = %f, y = %f\n", s.val[0], s.val[1]);
//			cvSet2D(dataset_test, iFace, iPoint, s);
//
//			CvScalar t;
//            t.val[0] = s.val[0];
//            cvSet2D(testWarpSet_test[iFace], iPoint, 0, t);
//            t.val[0] = s.val[1];
//            cvSet2D(testWarpSet_test[iFace], iPoint, 1, t);
//		}
//		//printf("==========face%d=============\n", iFace + 2);
//
//		CvScalar s;
//        s.val[0] = meanx_test / numberofpoints;
//        cvSet2D(  meanXY_test, iFace, 0, s);
//        s.val[0] = meany / numberofpoints;
//        cvSet2D(  meanXY_test, iFace, 1, s);
//	}
//
//	fclose(fptr_test);
//
//	/////////////////////////////////////////////////////////////////////////////////////////////
//    double avNorm_test = 0;
//    for (index = 0;index < k_test; index ++)	//	preProcess (linear alignment : zero mean)
//    {
//        for (int i = 0; i < numberofpoints; i ++)
//        {
//            CvScalar s,m1,m2;
//            s =    cvGet2D(dataset_test, index, i);
//            m1 =    cvGet2D(meanXY_test, index,0);
//            m2 =    cvGet2D(meanXY_test, index,1);
//
//            s.val[0] -= m1.val[0];
//            s.val[1] -= m2.val[0];
//            cvSet2D(  dataset_test, index,i, s);
//        }
//    }
//
//    CvMat * x1Tx2_test = cvCreateMat(2,2, CV_64FC1 );
//    CvMat * x1T_test = cvCreateMat(2,numberofpoints, CV_64FC1 );
//    CvMat * x2_test = cvCreateMat(numberofpoints,2, CV_64FC1 );
//    CvMat * UT_test= cvCreateMat(2,2, CV_64FC1 );
//    CvMat * V_test = cvCreateMat(2,2, CV_64FC1 );
//    CvMat * tempW_test = cvCreateMat(2,2, CV_64FC1 );
//    CvMat * VUT_test = cvCreateMat(2,2, CV_64FC1 );
//    CvMat * average_test = cvCreateMat(numberofpoints,2, CV_64FC1 );
//
//
//    for (int i = 0; i < numberofpoints; i ++)	// face1(已zero mean)的 land mark point
//    {
//        CvScalar s;
//        s= cvGet2D(dataset_test,0,i);
//        CvScalar m;
//        m.val[0]=s.val[0];
//        cvSet2D(  x2_test, i,0, m);
//        m.val[0]=s.val[1];
//        cvSet2D(  x2_test, i,1, m);
//    }
//
//    for (int iteration = 0; iteration < 5; iteration ++)
//    {
//        for (index = 0; index < k_test; index ++)	// face1~k(已zero mean)的 land mark point
//        {
//			//printf("========Face%d=========\n", k);
//            for (int i = 0; i < numberofpoints; i ++)	//	generate X1T (row:2 , col:63)
//            {
//                CvScalar s;
//                s= cvGet2D(dataset_test, index, i);
//				//printf("x = %lf, y = %lf\n", s.val[0], s.val[1]);
//                CvScalar m;
//                m.val[0] = s.val[0];
//                cvSet2D(  x1T_test,0, i, m);
//                m.val[0] = s.val[1];
//                cvSet2D(  x1T_test,1, i, m);
//            }
//
//            cvMatMul(x1T_test, x2_test, x1Tx2_test);
//            cvSVD( x1Tx2_test, tempW_test, UT_test, V_test,CV_SVD_U_T);
//            cvMatMul(V_test, UT_test, VUT_test);
//
//			//printf("\n\n\n\n");
//            for (int i = 0; i < numberofpoints; i ++)
//            {
//                CvScalar s;
//                s= cvGet2D(dataset_test, index, i);
//				
//                CvScalar v1,v2,v3,v4;
//                v1= cvGet2D(VUT_test, 0, 0);
//                v2= cvGet2D(VUT_test, 0, 1);
//                v3= cvGet2D(VUT_test, 1, 0);
//                v4= cvGet2D(VUT_test, 1, 1);
//                double k1 = v1.val[0] * s.val[0] + v2.val[0] * s.val[1];
//                double k2 = v3.val[0] * s.val[0] + v4.val[0] * s.val[1];
//                s.val[0] = k1;
//                s.val[1] = k2;
//				//printf("x_k = %lf, y_k = %lf\n", s.val[0], s.val[1]);
//                cvSet2D(dataset_test, index, i, s);
//
//            }
//
//        }
//        cvSetZero(average_test);
//
//        for (int i = 0; i < numberofpoints; i ++)
//        {
//            CvScalar s, avx, avy;
//            s= cvGet2D(dataset_test, 0, i);
//            avx= cvGet2D(average_test, i, 0);
//            avy= cvGet2D(average_test, i, 1);
//            avx.val[0]+=s.val[0];
//            avy.val[0]+=s.val[1];
//            cvSet2D(average_test, i, 0, avx);
//            cvSet2D(average_test, i, 1, avy);
//        }
//        for (index = 1;index < k_test; index ++)	//	共 k 張 的訓練影像
//        {
//            for (int i = 0;i < numberofpoints; i ++)
//            {
//                CvScalar s, avx, avy;
//                s = cvGet2D(dataset_test, index, i);
//                avx = cvGet2D(average_test, i, 0);
//                avy = cvGet2D(average_test, i, 1);
//                avx.val[0]= (avx.val[0] * index + s.val[0]) / (index + 1);
//                avy.val[0]= (avy.val[0] * index + s.val[1]) / (index + 1);
//                cvSet2D(average_test, i, 0, avx);
//                cvSet2D(average_test, i, 1, avy);
//            }
//        }
//
//    }
//
//    //CvFileStorage *fs_test;
//    //fs_test = cvOpenFileStorage( "./alignedset.aam", 0, CV_STORAGE_WRITE );
//    //cvWrite( fs_test, "shape",dataset_test, cvAttrList(0,0) );
//    //cvReleaseFileStorage( &fs_test );
//
//    CvMat **shapeAlignedSet_test =(CvMat**)cvAlloc(sizeof(CvMat*) * k_test);	//	共 k 張 的訓練影像
//
//    for (int i = 0; i < k_test; i ++)	//	共 k 張 的訓練影像
//    {
//        shapeAlignedSet_test[i]=cvCreateMat(1,totalnumberofpoints,CV_64FC1);
//
//        for (int m = 0; m < numberofpoints; m ++)
//        {
//            CvScalar v;
//            v= cvGet2D(dataset_test,i,m);
//
//			CvScalar s1,s2;
//            s1.val[0]=v.val[0];
//            s2.val[0]=v.val[1];
//            cvSet2D(shapeAlignedSet_test[i],0,2*m,s1);
//            cvSet2D(shapeAlignedSet_test[i],0,(2*m)+1,s2);
//        }
//    }
//
//    double norm_test = 0;
//    for (int i = 0 ; i < numberofpoints; i ++)	//	(sum of (entries^2))^0.5
//    {
//        CvScalar avx,avy;
//        avx= cvGet2D(average_test, i, 0);
//        avy= cvGet2D(average_test, i, 1);
//
//        norm += pow(avx.val[0],2)+pow(avy.val[0],2);
//
//    }
//
//
//    norm_test = sqrt(norm_test);
//
//    for (int i = 0; i < k_test; i ++)
//    {
//        double newNorm_test=0;
//
//        for (int m=0;m<numberofpoints;m++)
//        {
//            CvScalar avx,avy;
//
//            avx=cvGet2D(shapeAlignedSet_test[i],0,2*m);
//            avy=cvGet2D(shapeAlignedSet_test[i],0,(2*m)+1);
//
//            newNorm_test+=pow(avx.val[0],2)+pow(avy.val[0],2);
//        }
//
//        newNorm_test = sqrt(newNorm_test);
//        for (int m=0;m<numberofpoints;m++)
//        {
//            CvScalar avx,avy;
//
//            avx= cvGet2D(shapeAlignedSet_test[i],0,2*m);
//            avy=cvGet2D(shapeAlignedSet_test[i],0,(2*m)+1);
////            avx.val[0]*=norm/newNorm;
////            avy.val[0]*=norm/newNorm;
//            cvSet2D(shapeAlignedSet_test[i],0,2*m,avx);
//            cvSet2D(shapeAlignedSet_test[i],0,(2*m)+1,avy);
//        }
//
//    }
//    IplImage *imgPoints_test =cvCreateImage(cvSize(120,170), IPL_DEPTH_8U, 3);
//
//    cvSetZero(imgPoints_test);
//
//    for (int m = 0; m < numberofpoints; m ++)
//    {
//        CvScalar avx,avy;	
//
//        avx= cvGet2D(average_test, m, 0);
//        avy= cvGet2D(average_test, m, 1);
//        cvCircle( imgPoints_test, cvPoint(avx.val[0]+100,avy.val[0]+100), 1, CV_RGB(255,255,255), 2 );
//
//    }
//#ifdef DEBUG
//    //cvNamedWindow("warped1", CV_WINDOW_AUTOSIZE);
//    //cvShowImage("warped1",imgPoints);
//    //cvWaitKey(-1);	//int cvWaitKey( int delay=0 ) 返回值?int型，函?的???int型，?delay小于等于0的?候，如果?有??触?，?一直等待
//    
//	for (int i = 0; i < k_test; i ++)
//    {
//        double newNorm_test = 0;
//        cvSetZero(imgPoints_test);
//
//        for (int m=0;m<numberofpoints;m++)
//        {
//            CvScalar avx,avy;
//
//            avx=cvGet2D(shapeAlignedSet_test[i],0,2*m);
//            avy=cvGet2D(shapeAlignedSet_test[i],0,(2*m)+1);
//
////            cvCircle( imgPoints, cvPoint(avx.val[0]+100,avy.val[0]+100),1, CV_RGB(255,255, 0),2 );//CV_RGB(255,255,i*20),2 );
//			cvCircle( imgPoints_test, cvPoint(avx.val[0] + 58, avy.val[0] + 90),1, CV_RGB(255,255, 0),2 );//CV_RGB(255,255,i*20),2 );
//        }
//
////        cvNamedWindow("warped1",1);
////        cvShowImage("warped1",imgPoints);
////        cvWaitKey(-1);
//		
//		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		char shapeName_test[80];
//		sprintf(shapeName_test, "Test Face Shape/%d.jpg",  i+1);
//		cvSaveImage(shapeName_test, imgPoints_test);
//		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    }
//#endif
//
//
//
//
//
//
//
//
//	PCA_AAM  newPCAShape_test;
//    PCA_AAM newPCAAppearance_test;
//    newPCAShape_test.runPCA(shapeAlignedSet_test, meanXY_test, k_test, 5);
//
//    CvMat *shape_test;
//    CvMat ** warpedImage_test = (CvMat **)cvAlloc(k_test*sizeof(CvMat *));
//    shape_test = cvCreateMat(average_test->height, average_test->width, CV_64FC1);
//    shape_test = cvCloneMat(average_test);
//
//	IplImage **texture_test =  (IplImage **)cvAlloc(k_test*sizeof(IplImage *));
//
//    if (shape_test != NULL)
//    {
//        delaunay newDelaunay_test(shape_test);	//	計算 Triangle 的 3 個 座標點 儲存於 矩陣 Ai & 計算 Affine (Inverse_Ai)
//        IplImage *test_test = cvCreateImage(cvSize(newDelaunay_test.width, newDelaunay_test.height), IPL_DEPTH_8U,1);
//        cvSetZero(test_test);
//
//        for (int m = 0; m < k_test; m ++)
//            warpedImage_test[m] =  cvCreateMat(newDelaunay_test.height, newDelaunay_test.width,CV_64FC1);
//
//        for (int m = 0; m < k_test; m ++)
//        {
//            cvSetZero(test_test);
//
//
//            cvSetZero(warpedImage_test[m]);
//            int t_test = newDelaunay_test.numberOfPixels();	//	return totalNumberOfPixels;  即 Triangle 個數
//            newDelaunay_test.calculateAffineWarpParameters(testWarpSet_test[m]);	//	calculateAffineWarpParameters
//
//            for (int i = 0; i < t_test; i ++)	//	t = totalNumberOfPixels;
//            {
//                pixel * pix_test = newDelaunay_test.getpixel(i);	//	取得該[特徵點]的 warp，所對應的參數
//
//                pixel * pix2_test = newDelaunay_test.findCorrespondingPixelInImage(pix_test);	//	找出 對應點 的 " X 座標(pix2->x) " 和 " Y 座標(pix2->y) "
//
//                float imageValue_test = interpolate<uchar>(images_test[m], float(pix2_test->x), float(pix2_test->y));	//	Get interpolated_value
//
//                CvScalar value;
//                value.val[0] = imageValue_test;
//                if (floor(pix_test->y+pix_test->ty) >= 0 && floor(pix_test->x + pix_test->tx) >= 0)	//	double floor (double x); : Rounds x downward, returning the largest integral value that is not greater than x.
//                {
//                    cvSet2D(test_test, (int)floor(pix_test->y+pix_test->ty),(int)floor(pix_test->x+pix_test->tx),value);
//                    cvSet2D(warpedImage_test[m], (int)floor(pix_test->y+pix_test->ty),(int)floor(pix_test->x+pix_test->tx),value);
//                }
//
//
//            }
//
//#ifdef DEBUG
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////            cvNamedWindow("Warped Image",CV_WINDOW_AUTOSIZE);
////            cvShowImage("Warped Image",test);
////            cvWaitKey(-1);
//
//			char textureName_test[80];
//			sprintf(textureName_test, "Test Face Appearance/texture%d.jpg",  m+1);
//			cvSaveImage(textureName_test, test_test);
//
//			texture_test[m] = test_test;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#endif
//
//        }
//
//    }
//    cvReleaseMat(&dataset_test);
//    for (int i = 0; i < k_test; i ++)	//	共 k 張 的訓練影像
//    {
//        cvReleaseMat(&testWarpSet_test[i]);
//        cvReleaseMat(&shapeAlignedSet_test[i]);
//    }
//    newPCAAppearance_test.runPCA(warpedImage_test, meanXY_test, k_test, 5);	//	1.  2. k : 共 k 張 的訓練影像，得 1.Eigen Vector 2.Eigen Value 3.averageInput
//    eigenVectors_AAM *app_test = newPCAAppearance_test.returnEigens();	//	Get Eigens of Appearance
//    eigenVectors_AAM *shapes_test = newPCAShape_test.returnEigens();	//	Get Eigens of Shape
//
//
//
//	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//*
//	if (count>0)
//        {}
//    else
//        return;
//		
//    int datasetSize = dataset[0]->width*dataset[0]->height;
//	
//    nEigens = (count > datasetSize) ? datasetSize : (count);
//
//
//    CvMat* tmpEigenValues = cvCreateMat(1, nEigens, CV_64FC1);
//    CvMat* tmpEigenVectors = cvCreateMat(nEigens, datasetSize, CV_64FC1);
//    CvMat* MeanShape = cvCreateMat(1, datasetSize, CV_64FC1 );
//    cvZero(MeanShape);
//
//    CvMat * inputData=cvCreateMat(count, datasetSize, CV_64FC1);
//
//    int i;
//    for (i=0; i<count; i++)
//    {
//        for (int n=0;n<dataset[i]->height;n++)
//        {
//
//            for (int l=0;l<dataset[i]->width;l++)
//            {
//
//
//                CvScalar s;
//                s=cvGet2D(dataset[i],n,l);
//                cvSet2D(inputData,i,(n*dataset[i]->width) + l,s);
//                CvScalar t = cvGet2D(MeanShape,0,(n*dataset[i]->width) + l);
//                t.val[0]+=s.val[0];
//                cvSet2D(MeanShape,0,(n*dataset[i]->width) + l,t);
//
//
//            }
//        }
//    }
//    for (int n=0;n<dataset[0]->height;n++)
//    {
//
//        for (int l=0;l<dataset[0]->width;l++)
//        {
//
//            CvScalar t = cvGet2D(MeanShape,0,(n*dataset[0]->width) + l);
//            t.val[0]/=count;
//            cvSet2D(MeanShape,0,(n*dataset[0]->width) + l,t);
//
//        }
//    }
//
//    cvCalcPCA(inputData, MeanShape,tmpEigenValues, tmpEigenVectors, CV_PCA_DATA_AS_ROW);
//	*/
//	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	
//	printf("");
//	
//	//CvMat* pResult_test = cvCreateMat( count, kEigens, CV_32FC1 );	//	总的样本数, PCA变换后的样本维数(即主成份的数目)
//	//cvProjectPCA( inputData, MeanShape, tmpEigenVectors, pResult_test );
//
//
//	printf("");
//	
//	
//	//	重构,结果保存在pRecon中
//
//	CvMat* pRecon_test = cvCreateMat( shapes_test->count, shapes_test->datasetDimension, CV_32FC1 );		//	总的样本数, 每个样本的维数
//	
////	cvBackProjectPCA( shapes->space, newPCAShape_test.returnAverage(), shapes_test->eigens, pRecon_test );
//	
//	
//
//
//
//
//
//
//	cvReleaseMat( &shapes->space );
//	cvReleaseImage( shapes->eigens );
//
//
//
//	cvReleaseMat( warpedImage_test );
//
//	cvReleaseMat( &shapes_test->space );
//	cvReleaseImage( shapes_test->eigens );
//
//	cvReleaseMat( &app_test->space );
//	cvReleaseImage( app_test->eigens );
//

	printf("Training Faces' Weights To Rescrount The Testing Faces Completed ...\n\n");



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
/*
void aamTrainer::captureClick()
{
    cap=true;


}
void aamTrainer::timerEvent( QTimerEvent * )
{
    IplImage * queryImage = webcam.queryFrame();

    QImage * qm=QImageIplImageCvt(queryImage);
    if (cap==true)
    {
        time_t ltime;
        struct tm *Tm;
        struct timeval detail_time;

        char* setDir  = new char[200];
        char* uniqueName  = new char[200];
        ltime=time(NULL);
        Tm=localtime(&ltime);
        gettimeofday(&detail_time,NULL);
        sprintf(uniqueName,"%d%d%d%d%d%d%d.jpg",Tm->tm_year,Tm->tm_mon,Tm->tm_mday,Tm->tm_hour,Tm->tm_min,Tm->tm_sec,(detail_time.tv_usec/1000));
        sprintf(setDir,"./images/%s",uniqueName);
        cvSaveImage(setDir,queryImage);
        cap=false;
        populateQList();
    }

    setQImageWebcam(qm);
    cvWaitKey(1);
    delete qm;
    cvReleaseImage(&queryImage);

}
*/
int main()
{
	//aamTrainer tab1;
//	tab1.gendata();

	//int kEigens;

	//printf("Input kEigens = ");

	//scanf("%d", &kEigens);

	gendata();
	
	system("pause");
	return 0;
}
