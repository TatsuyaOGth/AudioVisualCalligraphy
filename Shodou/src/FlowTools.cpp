#include "FlowTools.h"

//--------------------------------------------------------------
void FlowTools::setup()
{
    ofSetVerticalSync(false);
    
    drawWidth = 1280;
    drawHeight = 720;
    // process all but the density on 16th resolution
    flowWidth = drawWidth/4;
    flowHeight = drawHeight/4;
    
    // Flow & Mask
    opticalFlow.setup(flowWidth, flowHeight);
    velocityMask.setup(drawWidth, drawHeight);
    
    // Fluid
#ifdef USE_FASTER_INTERNAL_FORMATS
    fluid.setup(flowWidth, flowHeight, drawWidth, drawHeight, true);
#else
    fluid.setup(flowWidth, flowHeight, drawWidth, drawHeight, false);
#endif
    
    flowToolsLogoImage.loadImage("flowtools.png");
    fluid.addObstacle(flowToolsLogoImage.getTextureReference());
    showLogo = true;
    
    // Particles
    particleFlow.setup(flowWidth, flowHeight, drawWidth, drawHeight);
    
    // Visualisation
    displayScalar.allocate(flowWidth, flowHeight);
    velocityField.allocate(flowWidth / 4, flowHeight / 4);
    temperatureField.allocate(flowWidth / 4, flowHeight / 4);
    
    // Draw Forces
    numDrawForces = 6;
    flexDrawForces = new ftDrawForce[numDrawForces];
    flexDrawForces[0].setup(drawWidth, drawHeight, FT_DENSITY, true);
    flexDrawForces[0].setName("draw full res");
    flexDrawForces[1].setup(flowWidth, flowHeight, FT_VELOCITY, true);
    flexDrawForces[1].setName("draw flow res 1");
    flexDrawForces[2].setup(flowWidth, flowHeight, FT_TEMPERATURE, true);
    flexDrawForces[2].setName("draw flow res 2");
    flexDrawForces[3].setup(drawWidth, drawHeight, FT_DENSITY, false);
    flexDrawForces[3].setName("draw full res");
    flexDrawForces[4].setup(flowWidth, flowHeight, FT_VELOCITY, false);
    flexDrawForces[4].setName("draw flow res 1");
    flexDrawForces[5].setup(flowWidth, flowHeight, FT_TEMPERATURE, false);
    flexDrawForces[5].setName("draw flow res 2");
    
    // Camera
    //    simpleCam.initGrabber(640, 480, true);
    //    didCamUpdate = false;
    //    cameraFbo.allocate(drawWidth, drawHeight);
    //    cameraFbo.begin(); ofClear(0); cameraFbo.end();
    
    
    setupGui();
    
    lastTime = ofGetElapsedTimef();
    lastMouse.set(0,0);
    
}

void FlowTools::setupGui(){
    gui.setup("settings");
    gui.setDefaultBackgroundColor(ofColor(0, 0, 0, 127));
    gui.setDefaultFillColor(ofColor(160, 160, 160, 160));
    gui.add(guiFPS.set("FPS", 0, 0, 60));
    gui.add(doFullScreen.set("fullscreen (F)", false));
    doFullScreen.addListener(this, &FlowTools::setFullScreen);
    gui.add(toggleGuiDraw.set("show gui (G)", false));
    gui.add(doFlipCamera.set("flip camera (C)", true));
    numVisualisationModes = 12;
    
    visualisationModeTitles = new string[numVisualisationModes];
    visualisationModeTitles[0] = "Source         (0)";
    visualisationModeTitles[1] = "Optical Flow   (1)";
    visualisationModeTitles[2] = "Flow Mask      (2)";
    visualisationModeTitles[3] = "Fluid Velocity (3)";
    visualisationModeTitles[4] = "Fluid Pressure (4)";
    visualisationModeTitles[5] = "Fld Temperature(5)";
    visualisationModeTitles[6] = "Fld Divergence (6)";
    visualisationModeTitles[7] = "Fluid Vorticity(7)";
    visualisationModeTitles[8] = "Fluid Buoyancy (8)";
    visualisationModeTitles[9] = "Fluid Obstacle (9)";
    visualisationModeTitles[10] = "Fluid Color    (-)";
    visualisationModeTitles[11] = "Fld Composite  (=)";
    
    int guiColorSwitch = 0;
    ofColor guiHeaderColor[2];
    guiHeaderColor[0].set(160, 160, 80, 200);
    guiHeaderColor[1].set(80, 160, 160, 200);
    ofColor guiFillColor[2];
    guiFillColor[0].set(160, 160, 80, 200);
    guiFillColor[1].set(80, 160, 160, 200);
    
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(opticalFlow.parameters);
    
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(velocityMask.parameters);
    
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(fluid.parameters);
    
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(particleFlow.parameters);
    
    visualisationParameters.setName("visualisation");
    visualisationParameters.add(showScalar.set("show scalar", true));
    visualisationParameters.add(showField.set("show field", true));
    visualisationParameters.add(displayScalarScale.set("display scalar scale", 0.25, 0.05, 0.5));
    displayScalarScale.addListener(this, &FlowTools::setDisplayScalarScale);
    visualisationParameters.add(velocityFieldArrowScale.set("arrow scale", 0.6, 0.2, 1));
    velocityFieldArrowScale.addListener(this, &FlowTools::setVelocityFieldArrowScale);
    visualisationParameters.add(temperatureFieldBarScale.set("temperature scale", 0.25, 0.05, 0.5));
    temperatureFieldBarScale.addListener(this, &FlowTools::setTemperatureFieldBarScale);
    visualisationParameters.add(visualisationLineSmooth.set("line smooth", false));
    visualisationLineSmooth.addListener(this, &FlowTools::setVisualisationLineSmooth);
    
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(visualisationParameters);
    
    leftButtonParameters.setName("mouse left button");
    for (int i=0; i<3; i++) {
        leftButtonParameters.add(flexDrawForces[i].parameters);
    }
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(leftButtonParameters);
    
    rightButtonParameters.setName("mouse right button");
    for (int i=3; i<6; i++) {
        rightButtonParameters.add(flexDrawForces[i].parameters);
    }
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(rightButtonParameters);
    
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(doResetDrawForces.set("reset draw forces (D)", false));
    doResetDrawForces.addListener(this,  &FlowTools::resetDrawForces);
    
    gui.loadFromFile("settings.xml");
    gui.minimizeAll();
    
    toggleGuiDraw = true;
}

void FlowTools::genEmergence(){
    
    ofVec2f velocity;
    //    genCounter+=1;
    //    if(genCounter<2+ofRandom(5)){return;}
    //    int i = 0;
    
    // 発散
    point = center + ofVec2f(ofRandom(-0.01,0.01),ofRandom(-0.01,0.01));
    for (int i = 0; i < 10; i++){
        velocity = point + 2 * ofVec2f(cos(pi * (i+ofRandom(10)) / 10), sin(pi * (i+ofRandom(10)) / 10));
        for (int i=0; i<3; i++) {
            if (flexDrawForces[i].getType() == FT_VELOCITY){
                flexDrawForces[i].setForce(velocity);
            }else if (flexDrawForces[i].getType() == FT_DENSITY){
                flexDrawForces[i].setForce(ofVec4f(1, 0, 0, 1)); // 色の設定
            }
            flexDrawForces[i].applyForce(point);
        }
    }
    
    // ギザギザ
    last_point = center;
    for (int i = 0; i < 10 ; i++) {
        point = center + ofVec2f(ofRandom(-0.05, 0.05), ofRandom(-0.05, 0.05));
        velocity = point - last_point;
        last_point = point;
        for (int i=0; i<3; i++) {
            if (flexDrawForces[i].getType() == FT_VELOCITY){
                flexDrawForces[i].setForce(velocity);
            }
            flexDrawForces[i].applyForce(point);
        }
    }
    
    genCounter = 0;
}

//--------------------------------------------------------------
void FlowTools::update(){
    //    didCamUpdate = false;
    //    simpleCam.update();
    //    if (simpleCam.isFrameNew())
    //        didCamUpdate = true;
    
    //    if(isGen){
    //        genEmergence();
    //    }
    
    deltaTime = ofGetElapsedTimef() - lastTime;
    lastTime = ofGetElapsedTimef();
}

//--------------------------------------------------------------
void FlowTools::draw(){
    ofClear(0);
    
    //    if (didCamUpdate) {
    //        ofPushStyle();
    //        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    //        cameraFbo.begin();
    //        if (doFlipCamera)
    //            simpleCam.draw(cameraFbo.getWidth(), 0, -cameraFbo.getWidth(), cameraFbo.getHeight());  // Flip Horizontal
    //        else
    //            simpleCam.draw(0, 0, cameraFbo.getWidth(), cameraFbo.getHeight());
    //        cameraFbo.end();
    //        ofPopStyle();
    //
    //        opticalFlow.setSource(cameraFbo.getTextureReference());
    //        opticalFlow.update(deltaTime);
    //
    //        velocityMask.setDensity(cameraFbo.getTextureReference());
    //        velocityMask.setVelocity(opticalFlow.getOpticalFlow());
    //        velocityMask.update();
    //    }
    
    fluid.addVelocity(opticalFlow.getOpticalFlowDecay());
    fluid.addDensity(velocityMask.getColorMask());
    fluid.addTemperature(velocityMask.getLuminanceMask());
    
    for (int i=0; i<numDrawForces; i++) {
        flexDrawForces[i].update();
        if (flexDrawForces[i].didChange()) {
            // if a force is constant multiply by deltaTime
            float strength = flexDrawForces[i].getStrength();
            if (!flexDrawForces[i].getIsTemporary())
                strength *=deltaTime;
            switch (flexDrawForces[i].getType()) {
                case FT_DENSITY:
                    fluid.addDensity(flexDrawForces[i].getTextureReference(), strength);
                    break;
                case FT_VELOCITY:
                    fluid.addVelocity(flexDrawForces[i].getTextureReference(), strength);
                    particleFlow.addFlowVelocity(flexDrawForces[i].getTextureReference(), strength);
                    break;
                case FT_TEMPERATURE:
                    fluid.addTemperature(flexDrawForces[i].getTextureReference(), strength);
                    break;
                case FT_PRESSURE:
                    fluid.addPressure(flexDrawForces[i].getTextureReference(), strength);
                    break;
                case FT_OBSTACLE:
                    fluid.addTempObstacle(flexDrawForces[i].getTextureReference());
                default:
                    break;
            }
        }
    }
    
    fluid.update();
    
    //    if (particleFlow.isActive()) {
    //        particleFlow.setSpeed(fluid.getSpeed());
    //        particleFlow.setCellSize(fluid.getCellSize());
    //        particleFlow.addFlowVelocity(opticalFlow.getOpticalFlow());
    //        particleFlow.addFluidVelocity(fluid.getVelocity());
    //        particleFlow.setObstacle(fluid.getObstacle());
    //    }
    //    particleFlow.update();
    
    
    int windowWidth = ofGetWindowWidth();
    int windowHeight = ofGetWindowHeight();
    ofClear(0,0);
    
    ofPushStyle();
    fluid.draw(0, 0, windowWidth, windowHeight);
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    //    if (particleFlow.isActive())
    //        particleFlow.draw(0, 0, windowWidth, windowHeight);
    if (showLogo) {
        ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
        ofSetColor(255,255,255,255);
        flowToolsLogoImage.draw(0, 0, windowWidth, windowHeight);
    }
    ofPopStyle();
    
    
    if (toggleGuiDraw) {
        guiFPS = ofGetFrameRate();
        gui.draw();
    }
    
}

//--------------------------------------------------------------
void FlowTools::emit(int x, int y)
{
    center.set(x / (float)ofGetWindowWidth(), y / (float)ofGetWindowHeight());
    if(!isGen){
        isGen = true;
        genCounter = 0;
    }
    genEmergence();
}
