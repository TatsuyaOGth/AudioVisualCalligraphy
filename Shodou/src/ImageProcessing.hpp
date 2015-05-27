#pragma once

#include "ofMain.h"
#include "utils.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"

namespace ImageProcessing
{
    using namespace ofxCv;
    using namespace cv;
    
    static void masking(ofPixels& pix, int x1, int y1, int x2, int y2)
    {
        const int w = pix.getWidth();
        const int h = pix.getHeight();
        const int c = pix.getNumChannels();
        for (int i = 0; i < h; ++i)
        {
            for (int j = 0; j < w; ++j)
            {
                const int index = i * w + j;
                if (i < y1 || i > y2 || j < x1 || j > x2)
                {
                    for (int k = 0; k < c; ++k)
                    {
                        pix[index * c + k] = 0;
                    }
                }
            }
        }
    }
    
    static void crop(ofPixels& pix, int x1, int y1, int x2, int y2)
    {
        pix.crop(x1, y1, x2 - x1, y2 - y1);
    }
    
    static void crop(ofPixels& pix, const ofVec2f& xy1, const ofVec2f& xy2)
    {
        crop(pix, xy1.x, xy1.y, xy2.x, xy2.y);
    }
    
    static void tiltWarp(ofPixels& pix, const double v)
    {
        cv::Mat img_src = toCv(pix);
        cv::Vec3b zero(0, 0, 0);
        cv::Mat_<cv::Vec3b> img_dst(img_src.rows, img_src.cols, zero);
        const double w = pix.getWidth();
        const double h = pix.getHeight();
        
        cv::Point2f src_pt[4], dst_pt[4];
        
        src_pt[0] = cvPoint2D32f(0.0,  0.0);
        src_pt[1] = cvPoint2D32f(0.0,  h);
        src_pt[2] = cvPoint2D32f(w,    h);
        src_pt[3] = cvPoint2D32f(w,    0.0);
        
        if (v > 0)
        {
            dst_pt[0] = cvPoint2D32f(-v, 0.0);
            dst_pt[1] = cvPoint2D32f(0.0, h);
            dst_pt[2] = cvPoint2D32f(w, h);
            dst_pt[3] = cvPoint2D32f(w + v, 0.0);
        }
        else {
            dst_pt[0] = cvPoint2D32f(0.0, 0.0);
            dst_pt[1] = cvPoint2D32f(v, h);
            dst_pt[2] = cvPoint2D32f(w - v, h);
            dst_pt[3] = cvPoint2D32f(w, 0.0);
        }
        
        const cv::Mat homography_matrix = cv::getPerspectiveTransform(src_pt, dst_pt);
        cv::warpPerspective(img_src, img_dst, homography_matrix,img_src.size());
        
        toOf(img_dst.clone(), pix);
    }
    
    static void rgbToGray(ofPixels& pix)
    {
        const int w = pix.getWidth();
        const int h = pix.getHeight();
        ofPixels dst;
        dst.allocate(w, h, 1);
        for (int i = 0; i < w * h; ++i)
        {
            dst[i] = (pix[i*3+0] * 0.298912) + (pix[i*3+1] * 0.586611) + (pix[i*3+2] * 0.114478);
        }
        pix.swap(dst);
    }
    
    static void threshold(ofPixels& pix)
    {
        cv::Mat tmp = toCv(pix);
        cv::threshold(tmp, tmp, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    }
    
    static void threshold(ofPixels& pix, double th)
    {
        cv::Mat tmp = toCv(pix);
        cv::threshold(tmp, tmp, th, 255, cv::THRESH_BINARY_INV);
    }

    
}

namespace imp = ImageProcessing;
