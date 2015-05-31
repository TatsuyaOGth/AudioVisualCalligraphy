#pragma once

#include "ofMain.h"
#include "utils.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"

namespace ImageProcessing
{
    using namespace ofxCv;
    using namespace cv;
    
    // shared values
    static ofxCvGrayscaleImage  cvGrayImg;
    static ofxCvContourFinder   cvContourFinder;
    
    static void resize(ofPixels& pix, int width, int height)
    {
        pix.resize(width, height);
    }
    
    static void crop(ofPixels& pix, int x1, int y1, int x2, int y2)
    {
        pix.crop(x1, y1, x2 - x1, y2 - y1);
    }
    
    static void crop(ofPixels& pix, const ofVec2f& xy1, const ofVec2f& xy2)
    {
        crop(pix, xy1.x, xy1.y, xy2.x, xy2.y);
    }
    
    static void warpPerspective(ofPixels& pix, const double vecX, const double vecY)
    {
        cv::Mat img_src = toCv(pix);
        cv::Mat img_dst;
        const double w = pix.getWidth();
        const double h = pix.getHeight();
        
        cv::Point2f src_pt[4], dst_pt[4];
        
        src_pt[0] = cvPoint2D32f(0.0,  0.0);
        src_pt[1] = cvPoint2D32f(0.0,  h);
        src_pt[2] = cvPoint2D32f(w,    h);
        src_pt[3] = cvPoint2D32f(w,    0.0);
        
        dst_pt[0] = cvPoint2D32f( (vecX > 0 ? -vecX  : 0.0     ), (vecY > 0 ? -vecY  : 0.0   ) );
        dst_pt[1] = cvPoint2D32f( (vecX > 0 ? 0.0    : vecX    ), (vecY > 0 ? h+vecY : h     ) );
        dst_pt[2] = cvPoint2D32f( (vecX > 0 ? w      : w-vecX  ), (vecY > 0 ? h      : h-vecY) );
        dst_pt[3] = cvPoint2D32f( (vecX > 0 ? w+vecX : w       ), (vecY > 0 ? 0.0    : vecY  ) );
        
        const cv::Mat homography_matrix = cv::getPerspectiveTransform(src_pt, dst_pt);
        cv::warpPerspective(img_src, img_dst, homography_matrix,img_src.size());
        
        toOf(img_dst, pix);
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

    static void findContours(ofPixels& pix, int nConsidered)
    {
        if (!cvGrayImg.bAllocated || cvGrayImg.getWidth() != pix.getWidth() || cvGrayImg.getHeight() != pix.getHeight())
        {
            cvGrayImg.allocate(pix.getWidth(), pix.getHeight());
        }
        cvGrayImg.setFromPixels(pix);
        cvContourFinder.findContours(cvGrayImg, 10, 640*480, nConsidered, true, false);
    }
}

namespace imp = ImageProcessing;
