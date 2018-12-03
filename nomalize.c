int main()
{
         IplImage* src = cvLoadImage("dm5.bmp");
         IplImage* gray = cvCreateImage(cvGetSize(src), src->depth, 1);
         cvCvtColor(src,gray,CV_BGR2GRAY);    //灰度化
         int width = src->width;
         int height = src->height;
         IplImage* dst = cvCreateImage(cvGetSize(src), src->depth, gray->nChannels);
         IplImage* scr = cvCreateImage(cvGetSize(gray), gray->depth, gray->nChannels);
         cvSmooth(gray, gray, CV_MEDIAN, 3, 0, 0, 0); //中值滤波，消除小的噪声；
         cvSmooth(gray, gray, CV_GAUSSIAN, 9, gray->nChannels);//高斯滤波
         cvCvtColor(src,scr,CV_BGR2GRAY);
     cvThreshold( gray, gray, 190, 255, CV_THRESH_BINARY);//二值化
         int nChannels =gray->nChannels;
         cvNamedWindow("origin",0);
    cvResizeWindow("origin",int(width/2),int(height/2));
         cvShowImage("origin", src);
         unsigned char* img = new unsigned char[width * height ];
         unsigned char* des = new unsigned char[width * height ];
         unsigned char* gra = new unsigned char[width * height];
         unsigned char* grt = new unsigned char[width * height];
         img_data(gray, gra,width,height, nChannels);
         img_data(scr, img,width,height,nChannels);
         AHE(des, img, width, height,nChannels,10);//自适应直方图均衡
         Projection( grt,gra,width,height);         //投影检测表盘区域
         img_extract(des,grt,width,height,1);      //表盘区域还原
         //kirsch(des,gra, width,height);
         data_img( scr, des, width,  height, nChannels);
         cvNamedWindow("表盘",0);
         cvResizeWindow("表盘",int(width/2),int(height/2));
         cvShowImage("表盘", scr);
         cvThreshold(scr, scr, 100, 255, CV_THRESH_BINARY); //表盘区域二值化以查找数字
         img_data(scr, img,width,height,nChannels);
         Adaptivemedianfilter(des, img, width, height,  nChannels);  //自适应中值滤波去噪
         ImageDilation( img, des, width, height,nChannels,1);    
         ImageErosion( des,img,width, height,nChannels,1);       //经过一次膨胀腐蚀去噪
         location(img, des, width, height);                      //找出数字所在区域
         data_img( scr, img, width,  height, nChannels);
         cvNamedWindow("数字",0);
    cvResizeWindow("数字",int(width/2),int(height/2));
         cvSaveImage("123.bmp",scr);
         cvShowImage("数字", scr);
         data_img( gray,des, width,  height, nChannels);
         cvNamedWindow("erzhi",0);
         cvResizeWindow("erzhi",int(width/2),int(height/2));
         cvShowImage("erzhi", gray);
         cvWaitKey(0);
}
/**************************************************************************
函数名：Projection
功  能：投影法找出矩形区域
输  入：目标图像des,  原图像 src,图像宽width, 高height
返回值：no
*************************************************************************/
void Projection(unsigned char* des,  const unsigned char* src,int width, int height)
{
         int* h_sum = new int[height];
         int* w_sum = new int[width];
         int up=0;
         int below=height;
         int left=0;
         int right=width;
         for(int y=0;y<height;y++)
         {
                   for(int x=0;x<width;x++)
                   {
                            des[y*width+x]=255;
                   }
         }
         for(int y=0;y<height;y++)
         {
                   h_sum[y]=0;
                   for(int x=0;x<width;x++)
                   {
                            //printf("src %d",src[y*width+x]);
                            h_sum[y]=h_sum[y]+src[y*width+x];
                   }
                   //printf("%d行%d ",y,h_sum[y]);
         }
         for(int y=height/2;y<height;y++)
         {
  
                   if((h_sum[y]-h_sum[height/2])>255*60)
                   {
                            below=y;
                            break;
                   }
         }
         for(int y=height/2;y>0;y--)
         {
                   if((h_sum[y]-h_sum[height/2])>255*60)
                   {
                            up=y;
                            break;
                   }
         }
         for(int x=0;x<width;x++)
         {
                   w_sum[x]=0;
                   for(int y=up;y<below;y++)
                   {
                            w_sum[x]=w_sum[x]+src[y*width+x];
                   }
                   //printf("%d列%d ",x,w_sum[x]);
         }
         int max_r=0;
         int max_l=0;
         for(int x=width/2+100;x<width;x++)
         {
                   if(w_sum[x]>max_r)
                   {
                            right=x;
                            max_r=w_sum[x];
                   }
         }
         for(int x=width/2-100;x>0;x--)
         {
                   if(w_sum[x]>max_l)
                   {
                            left=x;
                            max_l=w_sum[x];
                   }
         }
         for(int y=up;y<below;y++)
         {
                   for(int x=left;x<right;x++)
                   {
                            des[y*width+x]=0;
                   }
         }
         printf("up%d below%d left%d right%d",up, below,left, right);
  
}
void img_extract(unsigned char* des,  const unsigned char* src,int width, int height, int nChannels)
{
         for (int y=0;y<height;y++)
                   for(int x=0;x<width;x++)
                            if(src[y*width+x]!=0)
                            {
                                     for(int n = 0; n < nChannels; n++)
                                     {
                                               des[y * width * nChannels + x * nChannels + n ] = 255;
                                     }
                            }
}
/************************************************************************
函数名：location
功  能：投影法找出数字
输  入：目标图像des,  原图像 src,图像宽width, 高height
返回值：no
**********************************************************************/
void location(unsigned char* des,  const unsigned char* src,int width, int height)
{
         int* h_sum = new int[height];
         int* w_sum = new int[width];
         int up=0;
         int below=height;
         int left=0;
         int right=width;
         for(int y=0;y<height;y++)
         {
                   for(int x=0;x<width;x++)
                   {
                            des[y*width+x]=255;
                   }
         }
         for(int y=0;y<height;y++)
         {
                   h_sum[y]=0;
                   for(int x=0;x<width;x++)
                   {
                            //printf("src %d",src[y*width+x]);
                            h_sum[y]=h_sum[y]+src[y*width+x];
                   }
                   //printf("%d行%d ",y,h_sum[y]);
         }
         int h_mid=(h_sum[height/2]+h_sum[height/2-10]+h_sum[height/2-20]+h_sum[height/2-30]+h_sum[height/2-40]);
         h_mid=h_mid/5;
         for(int y=height/2;y<height;y++)
         {
  
                   if((h_sum[y]-h_mid)>255*35)
                   {
                            below=y;
                            break;
                   }
         }
         for(int y=height/2;y>0;y--)
         {
                   if((h_sum[y]-h_mid)>255*37)
                   {
                            up=y;
                            break;
                   }
         }
         for(int x=0;x<width;x++)
         {
                   w_sum[x]=0;
                   for(int y=up;y<below;y++)
                   {
                            w_sum[x]=w_sum[x]+src[y*width+x];
                   }
                   //printf("%d列%d ",x,w_sum[x]);
         }
         int right_start=width-10;
         for(int x=width-10;x>width/2;x--)
         {
                   if(w_sum[x]!=(below-up)*255)
                   {
                            right_start=x;
                            break;
                   }
         }
         for(int x=right_start-45;x>width/2;x--)
         {
                   if(w_sum[x]<255*(below-up-40))
                   {
                            right=x;
                            break;
                   }
         }
         int left_start=10;
         for(int x=10;x<width;x++)
         {
                   if(w_sum[x]!=(below-up)*255)
                   {
                            left_start=x;
                            break;
                   }
         }
         for(int x=left_start+100;x<width;x++)
         {
                            if(w_sum[x]<255*(below-up-20))
                            {
                                     left=x;
                                     break;
                            }
         }
  
         for(int y=up;y<below;y++)
         {
                   for(int x=left-5;x<right+5;x++)
                   {
                            des[y*width+x]=src[y*width+x];
                   }
         }
         printf("up%d below%d left%d right%d left_start%d h_mid%d height/2%d width%d",up, below,left, right,left_start,h_mid,height/2,width);
  
}
1.3结果展示

