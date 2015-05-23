#pragma once

/**
 *  Logger util
 */
#define FILE_AND_LINE ofSplitString(ofToString(__FILE__), "/").back()+"|line:"+ofToString(__LINE__)
#define LOG_NOTICE ofLogNotice(FILE_AND_LINE)
#define LOG_WARNING ofLogWarning(FILE_AND_LINE)
#define LOG_ERROR ofLogError(FILE_AND_LINE)
#define LOG_DEBUG ofLogNotice("[DEBUG]frame:"+ofToString(ofGetFrameNum())+"|"+FILE_AND_LINE)


