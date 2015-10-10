#pragma once

/**
 *  Logger util
 */
#define FILE_AND_LINE ofSplitString(ofToString(__FILE__), "/").back()+"|line:"+ofToString(__LINE__)
#define LOG_NOTICE ofLogNotice(FILE_AND_LINE)
#define LOG_WARNING ofLogWarning(FILE_AND_LINE)
#define LOG_ERROR ofLogError(FILE_AND_LINE)
#define LOG_DEBUG ofLogNotice("[DEBUG]frame:"+ofToString(ofGetFrameNum())+"|"+FILE_AND_LINE)



static void makeUniqueRandom(vector<int>& dst, int min, int max)
{
    if (max <= min)
    {
        return dst;
    }
    for (int i = min; i < max; ++i)
    {
        while(1)
        {
            int random = ofRandom(min, max);
            for (const auto& e : dst)
            {
                if (e == random)
                {
                    continue;
                }
            }
            dst.push_back(random);
            break;
        }
    }
}

static vector<int> makeUniqueRandom(int min, int max)
{
    vector<int> dst;
    makeUniqueRandom(dst, min, max);
    return dst;
}
