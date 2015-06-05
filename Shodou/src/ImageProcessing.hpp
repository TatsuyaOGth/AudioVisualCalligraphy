#pragma once

#include "ofMain.h"
#include "../../common/utils.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"

namespace ImageProcessing
{
    using namespace ofxCv;
    using namespace cv;
    
    // shared values
    static ofxCvGrayscaleImage  cvGrayImg;
    static ofxCvContourFinder   cvContourFinder;
    
    static void clonePixels(const ofPixels& src, ofPixels& dst)
    {
        dst.setFromPixels(src.getPixels(), src.getWidth(), src.getHeight(), src.getNumChannels());
    }
    
    static void resize(ofPixels& pix, int width, int height)
    {
        pix.resize(width, height);
    }
    
    static void resize(const ofPixels& src, ofPixels& dst, int width, int height)
    {
        clonePixels(src, dst);
        resize(dst, width, height);
    }
    
    static void flip(const ofPixels& src, ofPixels& dst, bool horizon, bool vertical)
    {
        clonePixels(src, dst);
        dst.mirror(vertical, horizon);
    }
    
    static void crop(ofPixels& pix, int x1, int y1, int x2, int y2)
    {
        pix.crop(x1, y1, x2 - x1, y2 - y1);
    }
    
    static void crop(ofPixels& pix, const ofVec2f& xy1, const ofVec2f& xy2)
    {
        crop(pix, xy1.x, xy1.y, xy2.x, xy2.y);
    }
    
    static void crop(const ofPixels& src, ofPixels& dst, int x1, int y1, int x2, int y2)
    {
        clonePixels(src, dst);
        dst.crop(x1, y1, x2 - x1, y2 - y1);
    }
    
    static void crop(const ofPixels& src, ofPixels& dst, const ofVec2f& xy1, const ofVec2f& xy2)
    {
        clonePixels(src, dst);
        crop(dst, xy1.x, xy1.y, xy2.x, xy2.y);
    }
    
    static void warpPerspective(ofPixels& src, ofPixels& dst, const double vecX, const double vecY)
    {
        cv::Mat img_src = toCv(src);
        cv::Mat img_dst;
        const double w = src.getWidth();
        const double h = src.getHeight();
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
        toOf(img_dst, dst);
    }
    
    static void warpPerspective(ofPixels& pix, const double vecX, const double vecY)
    {
        ofPixels dst;
        warpPerspective(pix, dst, vecX, vecY);
        pix.swap(dst);
    }
    
    static void rgbToGray(const ofPixels& pix, ofPixels& dst)
    {
        const int w = pix.getWidth();
        const int h = pix.getHeight();
        dst.allocate(w, h, 1);
        for (int i = 0; i < w * h; ++i)
        {
            dst[i] = (pix[i*3+0] * 0.298912) + (pix[i*3+1] * 0.586611) + (pix[i*3+2] * 0.114478);
        }
    }
    
    static void rgbToGray(ofPixels& pix)
    {
        ofPixels dst;
        rgbToGray(pix, dst);
        pix.swap(dst);
    }
    
    static void thresholdOtsu(ofPixels& pix)
    {
        cv::Mat tmp = toCv(pix);
        cv::threshold(tmp, tmp, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    }
    
    static void thresholdOtsu(ofPixels& src, ofPixels& dst)
    {
        cv::Mat img_src = toCv(src);
        cv::Mat img_dst;
        cv::threshold(img_src, img_dst, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
        toOf(img_dst, dst);
    }
    
    static void threshold(ofPixels& pix, double th)
    {
        cv::Mat tmp = toCv(pix);
        cv::threshold(tmp, tmp, th, 255, cv::THRESH_BINARY_INV);
    }
    
    static void threshold(ofPixels& src, ofPixels& dst, double th)
    {
        // TODO: no update src image
        cv::Mat img_src = toCv(src);
        cv::Mat img_dst = toCv(dst);
        cv::threshold(img_src, img_dst, th, 255, cv::THRESH_BINARY_INV);
//        toOf(img_dst, dst);
    }

    
    
    
    static void findContours(const ofPixels& pix, int nConsidered)
    {
        if (!cvGrayImg.bAllocated || cvGrayImg.getWidth() != pix.getWidth() || cvGrayImg.getHeight() != pix.getHeight())
        {
            cvGrayImg.allocate(pix.getWidth(), pix.getHeight());
        }
        cvGrayImg.setFromPixels(pix);
        cvContourFinder.findContours(cvGrayImg, 10, 640*480, nConsidered, true, false);
    }
    
    
    
//    static bool joinPixels(const ofPixels& pix1, const ofPixels pix2, ofPixels& distPix)
//    {
//        if (pix1.getNumChannels() != pix2.getNumChannels())
//        {
//            LOG_ERROR << "different channels";
//            return false;
//        }
//    
//        
//        
//        const int w1 = pix1.getWidth();
//        const int h1 = pix1.getHeight();
//        const int w2 = pix1.getWidth();
//        const int h2 = pix1.getHeight();
//        const int ch = pix1.getNumChannels();
//        distPix.allocate(w1 + w2, h, ch);
//        
//        for (int y = 0; y < h; ++y)
//        {
//            for (int x = 0; x < w; ++x)
//            {
//                const int index   = w * y + x;
//                const int dstIdx1 = index + w * y;
//                const int dstIdx2 = index + w * y * w;
//                for (int j = 0; j < ch; ++j)
//                {
//                    distPix[dstIdx1 * ch + j] = pix1[index * ch + j];
//                    distPix[dstIdx2 * ch + j] = pix2[index * ch + j];
//                }
//            }
//        }
//        return true;
//    }
}

namespace imp = ImageProcessing;
