#include "ofApp.h"

//------------------------------------------------------------------------------
// BORDERLANDS:  An interactive granular sampler.
//------------------------------------------------------------------------------
// More information is available at
//     http::/ccrma.stanford.edu/~carlsonc/256a/Borderlands/index.html
//
//
// Copyright (C) 2011  Christopher Carlson
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


//
//  main.cpp
//  Borderlands
//
//  Created by Christopher Carlson on 11/13/11.
//

// Global variable containing pointer on instance of this class, used for callback function
// In setup(), the openStream method wait a pointer on a callback function which
// respond to typedef RtAudioCallback defined in RTAudio.h, and the callback must access member variables 
// So we can't provide a pointer on this class in parameter, and we can't make the callback static
ofApp* g_thisApp;

using namespace std;

//--------------------------------------------------------------------------------
// Initialization
//--------------------------------------------------------------------------------

void ofApp::init(){
    //grain cloud audio objects
    grainCloud = NULL;
    
    fullscreen = false;
    menuFlag = true;
    showCursor = true;
    
    //audio system
    theAudio = NULL;
    //library path
    drawAudioPath = string("data/loops/");
    g_audioPath = string("./loops/");
    //parameter string
    paramString = "";
    //desired audio buffer size
    g_buffSize = 1024;
    // load sounds
    newFileMgr = NULL;
    //audio files
    mySounds = NULL;
    //audio file visualization objects
    soundViews = NULL;
    //grain cloud visualization objects
    grainCloudVis = NULL;
    //cloud counter
    numClouds = 0;
    
    //Initial camera movement vars
    //my position
    position = ofPoint(0.0,0.0,0.0f);
    
    //ENUMS
    //default selection mode
    selectionMode = CLOUD;
    dragMode = MOVE;
    resizeDir = false; //for rects
    //rubber band select params
    rb_anchor_x = -1;
    rb_anchor_y = -1;
    
    //not used yet - for multiple selection
    selectionIndices = new vector<int>;
    
    //selection helper vars
    selectedCloud = -1;
    selectedRect = -1;
    selectionIndex = 0;
    
    //flag indicating parameter change
    paramChanged = false;
    currentParam = NUMGRAINS;
    lastParamChangeTime = 0.0;
    tempParamVal = -1.0;

    //mouse coordinate initialization
    mouseX = -1;
    mouseY = -1;
    veryHighNumber = 50000000;
    lastDragX = veryHighNumber;
    lastDragY = veryHighNumber;
    
    //keyboard modifier key
    modkey = -1;
    
    //flag for help menu display
    showHelpMenu = false;
    
    //flag for voice limiter function
    voiceLimiterActive = false;
    
    //voice limiter variables
    voicesLimit = 150;
    maxVoicesPerCloud = 10;
    
    //initial number of voices when a new grain is created
    numVoices = 8;
    
    generateOutsideInteractiveArea = false;
    interactiveArea = ofRectangle(0.0f, 0.0f, 1.0f, 1.0f);
    oscPort = 12000;
#ifdef MAC_OS_X_VERSION_10_6
    syphonServerName = string("Borderlands");
#endif
    
}

void ofApp::loadSettings(){
    
    if(settings.getValue("appSettings:fullscreen", "") == "true")
        fullscreen = true;
    else if(settings.getValue("appSettings:fullscreen", "") == "false")
        fullscreen = false;
    //else, default value defined in init()
    
    if(settings.getValue("appSettings:showStartMenu", "") == "true")
        menuFlag = true;
    else if(settings.getValue("appSettings:showStartMenu", "") == "false")
        menuFlag = false;
    //else, default value defined in init()
    
    if(settings.getValue("appSettings:showCursor", "") == "true")
        showCursor = true;
    else if(settings.getValue("appSettings:showCursor", "") == "false")
        showCursor = false;
    //else, default value defined in init()
    
    //library path
    drawAudioPath = string("data/") + settings.getValue("appSettings:audioPath", g_audioPath);
    g_audioPath = settings.getValue("appSettings:audioPath", g_audioPath);
    
    //desired audio buffer size
    g_buffSize = (unsigned int)settings.getValue("appSettings:bufferSize", (int)g_buffSize);
    
    //flag for voice limiter function
    if(settings.getValue("appSettings:useVoiceLimiter", "") == "true")
        voiceLimiterActive = true;
    else if(settings.getValue("appSettings:useVoiceLimiter", "") == "false")
        voiceLimiterActive = false;
    //else, default value defined in init()
    
    voicesLimit = settings.getValue("appSettings:voicesLimit", voicesLimit);
    maxVoicesPerCloud = settings.getValue("appSettings:maxVoicesPerCloud", maxVoicesPerCloud);
    
    numVoices = settings.getValue("cloudSettings:numVoices", numVoices);
    
    
    if(settings.getValue("appSettings:createSoundsOutside", "") == "true")
        generateOutsideInteractiveArea = true;
    else if(settings.getValue("appSettings:createSoundsOutside", "") == "false")
        generateOutsideInteractiveArea = false;
    //else, default value defined in init()
    
    interactiveArea = ofRectangle(settings.getValue("appSettings:interactiveAreaOriginX", interactiveArea.x),
                                  settings.getValue("appSettings:interactiveAreaOriginY", interactiveArea.y),
                                  settings.getValue("appSettings:interactiveAreaWidth", interactiveArea.width),
                                  settings.getValue("appSettings:interactiveAreaHeight", interactiveArea.height));
    
    oscPort = settings.getValue("appSettings:oscPort", oscPort);
#ifdef MAC_OS_X_VERSION_10_6
    syphonServerName = settings.getValue("appSettings:syphonServerName", syphonServerName);
#endif
}

//--------------------------------------------------------------------------------
// Cleanup code
//--------------------------------------------------------------------------------

void ofApp::cleaningFunction(){
    try {
        theAudio->stopStream();
        theAudio->closeStream();
    } catch (RtError &err) {
        err.printMessage();
    }
    if (newFileMgr != NULL)
        delete newFileMgr;
    if (mySounds != NULL)
        delete mySounds;
    if (theAudio !=NULL)
        delete theAudio;
    
    if (grainCloud!=NULL){
        delete grainCloud;
    }
    
    if (grainCloudVis!=NULL){
        delete grainCloudVis;
    }
    if (soundViews != NULL){
        delete soundViews;
    }
    if (selectionIndices != NULL){
        delete selectionIndices;
    }
}

//================================================================================
//   Audio Callback
//================================================================================

//audio callback
int audioCallback( void * outputBuffer, void * inputBuffer, unsigned int numFrames, double streamTime,
                  RtAudioStreamStatus status, void * userData)
{
    //cast audio buffers
    SAMPLE * out = (SAMPLE *)outputBuffer;
    SAMPLE * in = (SAMPLE *)inputBuffer;
    
    memset(out, 0, sizeof(SAMPLE)*numFrames*MY_CHANNELS );
    if (g_thisApp->menuFlag == false){
        for(int i = 0; i < g_thisApp->grainCloud->size(); i++){
            g_thisApp->grainCloud->at(i)->nextBuffer(out, numFrames);
        }
    }
    borderlands::GTime::instance().sec += numFrames*g_thisApp->samp_time_sec;
    // cout << GTime::instance().sec<<endl;
    return 0;
}


///-----------------------------------------------------------------------------
// name: drawAxis()
// desc: draw 3d axis
//-----------------------------------------------------------------------------
void ofApp::drawAxis()
{
    //PUSH -- //store state
    glPushMatrix();
    
    //specify vertices with this drawing mode
    glBegin(GL_LINES);
    glLineWidth(0.9f);
    //x axis
    glColor4f(1,0,0,0.9);
    glVertex3f(0,0,0);
    glVertex3f(ofGetWidth(),0,0);
    
    //x axis
    glColor4f(0,1,0,0.9);
    glVertex3f(0,0,0);
    glVertex3f(0,ofGetHeight(),0);
    
    //z axis
    glColor4f(0,0,1,0.7);
    glVertex3f(0,0,0);
    glVertex3f(0,0,400);
    
    //stop drawing
    glEnd();
    
    //POP -- //restore state
    glPopMatrix();
}


//-----------------------------------------------------------------------------
// Display simple string
// desc: from sndpeek source - Ge Wang, et al
//-----------------------------------------------------------------------------
void ofApp::draw_string( GLfloat x, GLfloat y, GLfloat z, const char * str, GLfloat scale = 1.0f )
{
    GLint len = strlen( str ), i;
    
    glPushMatrix();
    glTranslatef( x, ofGetHeight()-y, z );
    glScalef( .001f * scale, .001f * scale, .001f * scale );
    
    myFont.drawString(str, 0, 0);
    
    glPopMatrix();
}


//-----------------------------------------------------------------------------
// Show usage on screen.  TODO:  add usage info
//-----------------------------------------------------------------------------
void ofApp::printUsage(){
    float smallSize = 0.03f;
    float mediumSize = 0.04f;
    glLineWidth(2.0f);
    float theA = 0.6f + 0.2*sin(0.8*PI*borderlands::GTime::instance().sec);
    glColor4f(theA,theA,theA,theA);
    draw_string(0.2f*(float)ofGetWidth()/2,(float)ofGetHeight()/2.0f, 0.5f,"AUGMENTA BORDERLANDS",160);
    
    theA = 0.6f + 0.2*sin(0.8*PI*borderlands::GTime::instance().sec-1);
    float insColor = theA*0.4f;
    glColor4f(insColor,insColor,insColor,theA);
    //key info
    string folderPath = "LOAD .AIFF OR .WAV IN "+drawAudioPath+" FOLDER";
    draw_string(0.2f*(float)ofGetWidth()/2+20.0,(float)ofGetHeight()/2.0f - 40.0, 0.5f, (char*)folderPath.c_str(),96);
    
    theA = 0.6f + 0.2*sin(0.8*PI*borderlands::GTime::instance().sec-2);
    insColor = theA*0.4f;
    glColor4f(insColor,insColor,insColor,theA);
    //key info
    draw_string(0.2f*(float)ofGetWidth()/2+20.0,(float)ofGetHeight()/2.0f - 65.0, 0.5f,"CLICK TO START",96);
    
    theA = 0.6f + 0.2*sin(0.8*PI*borderlands::GTime::instance().sec-3);
    insColor = theA*0.4f;
    glColor4f(insColor,insColor,insColor,theA);
    //key info
    draw_string(0.2f*(float)ofGetWidth()/2+20.0,(float)ofGetHeight()/2.0f - 90.0, 0.5f,"ESCAPE TO QUIT",96);
    
    theA = 0.6f + 0.2*sin(0.8*PI*borderlands::GTime::instance().sec-4);
    insColor = theA*0.4f;
    glColor4f(insColor,insColor,insColor,theA);
    //key info
    draw_string(0.2f*(float)ofGetWidth()/2+20.0,(float)ofGetHeight()/2.0f - 115.0, 0.5f,"H FOR HELP",96);
}

void ofApp::printParam(){
    if ((numClouds > 0) && (selectedCloud >=0)){
        GrainClusterVis * theCloudVis= grainCloudVis->at(selectedCloud);
        GrainCluster * theCloud = grainCloud->at(selectedCloud);
        float cloudX = theCloudVis->getX();
        float cloudY = theCloudVis->getY();
        string myValue;
        ostringstream sinput;
        ostringstream sinput2;
        float theA = 0.7f + 0.3*sin(1.6*PI*borderlands::GTime::instance().sec);
        glColor4f(1.0f,1.0f,1.0f,theA);
        
        switch (currentParam) {
            case NUMGRAINS:
                myValue = "Voices: ";
                sinput << theCloud->getNumVoices();
                myValue = myValue+ sinput.str();
                draw_string((GLfloat)cloudX,(GLfloat) (ofGetHeight()-cloudY),0.0,myValue.c_str(),100.0f);
                break;
            case DURATION:
                myValue = "Duration: ";
                if (paramString == ""){
                    sinput << theCloud->getDurationMs();
                    myValue = myValue + sinput.str() + " ms";
                }else{
                    myValue = myValue + paramString + " ms";
                }
                draw_string((GLfloat)cloudX,(GLfloat) (ofGetHeight()-cloudY),0.0,myValue.c_str(),100.0f);
                //            myValue = "Duration (ms): " + theCloud->getDurationMs();
                break;
            case WINDOW:
                switch (theCloud->getWindowType()) {
                    case HANNING:
                        myValue = "Window: HANNING";
                        break;
                    case TRIANGLE:
                        myValue = "Window: TRIANGLE";
                        break;
                    case REXPDEC:
                        myValue = "Window: REXPDEC";
                        break;
                    case EXPDEC:
                        myValue = "Window: EXPDEC";
                        break;
                    case SINC:
                        myValue = "Window: SINC";
                        break;
                    case RANDOM_WIN:
                        myValue = "Window: RANDOM_WIN";
                        break;
                    default:
                        myValue = "";
                        break;
                }
                
                draw_string((GLfloat)cloudX,(GLfloat) (ofGetHeight()-cloudY),0.0,myValue.c_str(),100.0f);
                break;
            case MOTIONX:
                myValue = "X: ";
                sinput << theCloudVis->getXRandExtent();
                myValue = myValue + sinput.str();
                draw_string((GLfloat)cloudX,(GLfloat) (ofGetHeight()-cloudY),0.0,myValue.c_str(),100.0f);
                break;
            case MOTIONY:
                myValue = "Y: ";
                sinput << theCloudVis->getYRandExtent();
                myValue = myValue + sinput.str();
                draw_string((GLfloat)cloudX,(GLfloat) (ofGetHeight()-cloudY),0.0,myValue.c_str(),100.0f);
                break;
            case MOTIONXY:
                myValue = "X,Y: ";
                sinput << theCloudVis->getXRandExtent();
                myValue = myValue + sinput.str() + ", ";
                sinput2 << theCloudVis->getYRandExtent();
                myValue = myValue + sinput2.str();
                draw_string((GLfloat)cloudX,(GLfloat) (ofGetHeight()-cloudY),0.0,myValue.c_str(),100.0f);
                break;
                
            case DIRECTION:
                switch(theCloud->getDirection()){
                    case FORWARD:
                        myValue = "Direction: FORWARD";
                        break;
                    case BACKWARD:
                        myValue = "Direction: BACKWARD";
                        break;
                    case RANDOM_DIR:
                        myValue = "Direction: RANDOM";
                        break;
                    default:
                        myValue = "";
                        break;
                }
                draw_string((GLfloat)cloudX,(GLfloat) (ofGetHeight()-cloudY),0.0,myValue.c_str(),100.0f);
                break;
                
            case SPATIALIZE:
                switch(theCloud->getSpatialMode()){
                    case UNITY:
                        myValue = "Spatial Mode: UNITY";
                        break;
                    case STEREO:
                        myValue = "Spatial Mode: STEREO";
                        break;
                    case AROUND:
                        myValue = "Spatial Mode: AROUND";
                        break;
                    default:
                        myValue = "";
                        break;
                }
                draw_string((GLfloat)cloudX,(GLfloat) (ofGetHeight()-cloudY),0.0,myValue.c_str(),100.0f);
                break;
            case VOLUME:
                myValue = "Volume (dB): ";
                if (paramString == ""){
                    sinput << theCloud->getVolumeDb();
                    myValue = myValue + sinput.str();
                }else{
                    myValue = myValue + paramString;
                }
                draw_string((GLfloat)cloudX,(GLfloat) (ofGetHeight()-cloudY),0.0,myValue.c_str(),100.0f);
                break;
            case OVERLAP:
                myValue = "Overlap: ";
                if (paramString == ""){
                    sinput << theCloud->getOverlap();
                    myValue = myValue + sinput.str();
                }else{
                    myValue = myValue + paramString;
                }
                draw_string((GLfloat)cloudX,(GLfloat) (ofGetHeight()-cloudY),0.0,myValue.c_str(),100.0f);
                //            myValue = "Duration (ms): " + theCloud->getDurationMs();
                break;
            case PITCH:
                myValue = "Pitch: ";
                if (paramString == ""){
                    sinput << theCloud->getPitch();
                    myValue = myValue + sinput.str();
                }else{
                    myValue = myValue + paramString;
                }
                draw_string((GLfloat)cloudX,(GLfloat) (ofGetHeight()-cloudY),0.0,myValue.c_str(),100.0f);
                //            myValue = "Duration (ms): " + theCloud->getDurationMs();
                break;
                
            case P_LFO_FREQ:
                myValue = "Pitch LFO Freq: ";
                if (paramString == ""){
                    sinput << theCloud->getPitchLFOFreq();
                    myValue = myValue + sinput.str();
                }else{
                    myValue = myValue + paramString;
                }
                draw_string((GLfloat)cloudX,(GLfloat) (ofGetHeight()-cloudY),0.0,myValue.c_str(),100.0f);
                //            myValue = "Duration (ms): " + theCloud->getDurationMs();
                break;
            case P_LFO_AMT:
                myValue = "Pitch LFO Amount: ";
                if (paramString == ""){
                    sinput << theCloud->getPitchLFOAmount();
                    myValue = myValue + sinput.str();
                }else{
                    myValue = myValue + paramString;
                }
                draw_string((GLfloat)cloudX,(GLfloat) (ofGetHeight()-cloudY),0.0,myValue.c_str(),100.0f);
                //            myValue = "Duration (ms): " + theCloud->getDurationMs();
                break;
            default:
                break;
        }
    }
    
}


//================================================================================
//   INTERACTION/GLUT
//================================================================================


//update mouse coords based on mousemovement
void ofApp::updateMouseCoords(int x, int y){
    mouseX = x+position.x;
    mouseY = y+position.y;
}


//-----------------------------------------------------------------------------
// Handle mouse clicks, etc.
//-----------------------------------------------------------------------------


//handle deselections
void ofApp::deselect(int shapeType){
    switch (shapeType){
        case CLOUD:
            if (selectedCloud >=0){
                grainCloudVis->at(selectedCloud)->setSelectState(false);
                //reset selected cloud
                selectedCloud = -1;
                //cout << "deselecting cloud" <<endl;
            }
            
        case RECT:
            if (selectedRect >= 0){
                //cout << "deselecting rect" << endl;
                soundViews->at(selectedRect)->setSelectState(false);
                selectedRect = -1;
            }
            
    }
}


//--------------------------------------------------------------
void ofApp::setup(){
    
    // Hide cursor
    ofHideCursor(); // Bug in of 0.8.4, even if we want to show cursor we have to go through that
    
    // Load settings file
    settings.loadFile("settings.xml");
    
    // Initialize Borderlands variables with default values
    init();
    // If needed, override default variables with values defined in settings.xml
    loadSettings();
    
#ifdef DEV_MODE
    // Override settings for dev use
    fullscreen = false;
    showCursor = true;
    
#endif
    
    
    ofSetFullscreen(fullscreen);
    ofSetWindowTitle("Augmenta Borderlands");
    
    // Global pointer to this instance of Borderlands
    g_thisApp = this;
    
    // Initialize path to loops
    ofDirectory g_audioDir(g_audioPath);
    g_audioPath = g_audioDir.path();
    
    // Connect augmenta receiver
    oscPortDisplayMessage = "Listening to OSC on port " + ofToString(oscPort);
    try {
        augmentaReceiver.connect(oscPort);
    } catch (std::exception&e) {
        std::cerr << "Error : " << e.what() << endl;
        oscPortDisplayMessage = "Could not bind to port " + ofToString(oscPort) + " !";
    }
    
    ofxAddAugmentaListeners(this);  // for augmenta events
    
    // TODO : Change to value from xml (beware nothing to do with window size)
    m_fbo.allocate(ofGetWidth(), ofGetHeight());
    
    #ifdef MAC_OS_X_VERSION_10_6
    syphonServer.setName(syphonServerName);
    #endif
    
    // Load font
    myFont.loadFont(OF_TTF_MONO, 128);
    
    //init random number generator
    srand(time(NULL));
    //start time
    
    //-------------Graphics Initialization--------//
    
    // initialize graphics
    //initialize();
    
    // load sounds
    newFileMgr = new AudioFileSet();
    
    if (newFileMgr->loadFileSet(g_audioPath) == 1){
        cleaningFunction();
        return;
    }
    
    mySounds = newFileMgr->getFileVector();
    cout << "Sounds loaded successfully..." << endl;
    
    //create visual representation of sounds
    soundViews = new vector<SoundRect *>;
    for (int i = 0; i < mySounds->size(); i++)
    {
        if(generateOutsideInteractiveArea)
            soundViews->push_back(new SoundRect(interactiveArea));
        else
            soundViews->push_back(new SoundRect());
        soundViews->at(i)->associateSound(mySounds->at(i)->wave,mySounds->at(i)->frames,mySounds->at(i)->channels);
    }
    
    //init grain cloud vector and corresponding view vector
    grainCloud = new vector<GrainCluster *>;
    grainCloudVis = new vector<GrainClusterVis *>;
    
    //-------------Audio Configuration-----------//
    
    //configure RtAudio
    //create the object
    try {
        theAudio = new MyRtAudio(1,MY_CHANNELS, MY_SRATE, &g_buffSize, MY_FORMAT,true);
    } catch (RtError & err) {
        err.printMessage();
        ofExit(1);
    }
    try
    {
        //open audio stream/assign callback
        theAudio->openStream(&audioCallback);
        //get new buffer size
        g_buffSize = theAudio->getBufferSize();
        //start audio stream
        theAudio->startStream();
        //report latency
        theAudio->reportStreamLatency();
        
    }catch (RtError & err )
    {
        err.printMessage();
        cleaningFunction();
        return;
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    
}

//--------------------------------------------------------------
void ofApp::draw(){

    //ofEnableAlphaBlending();
    
    // Draw fbo
    //m_fbo.begin();
    drawVisuals();
    //m_fbo.end();
    
    if(showHelpMenu){
        drawHelp();
    }
    
    //ofDisableAlphaBlending();
    

    //glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    // Draw visuals
    //ofSetColor(ofColor::white); // Needed !
    //m_fbo.draw(0,0,ofGetWidth(),ofGetHeight());

    // publish fbo to syphon
    #ifdef MAC_OS_X_VERSION_10_6
    syphonServer.publishTexture(&m_fbo.getTextureReference());
    #endif
    
    //glDisable(GL_BLEND);
}

//--------------------------------------------------------------
void ofApp::drawVisuals(){
    
    //ofClear(ofColor::white);
    ofBackground(ofColor::black);
    
    ofPushMatrix();
    
    //update viewer position
    ofTranslate(position.x,position.y,position.z); //translate the screen to the position of our camera
    if (menuFlag == false){
        //render rectangles
        if (soundViews){
            for (int i = 0; i < soundViews->size(); i++)
            {
                soundViews->at(i)->draw();
            }
        }
        
        //render grain clouds if they exist
        if (grainCloudVis){
            for (int i = 0; i < grainCloudVis->size(); i++)
            {
                grainCloudVis->at(i)->draw();
            }
        }
        
        //print current param if editing
        if ( (selectedCloud >= 0) || (selectedRect >= 0) )
            printParam();
        
        //draw interactive area
        ofPushStyle();
        ofSetColor(200, 235, 255, 40);
        ofSetLineWidth(2);
        ofRect(interactiveArea.x*ofGetWidth(), interactiveArea.y*ofGetHeight(), interactiveArea.width*ofGetWidth(), interactiveArea.height*ofGetHeight());
        ofNoFill();
        ofSetColor(200, 235, 255, 140);
        ofRect(interactiveArea.x*ofGetWidth(), interactiveArea.y*ofGetHeight(), interactiveArea.width*ofGetWidth(), interactiveArea.height*ofGetHeight());
        ofPopStyle();
    }else{
        printUsage();
    }
    
    ofPopMatrix();
    
    // Draw cursor
    if(showCursor){
        ofCircle(mouseX, mouseY, 10);
    }
}

//--------------------------------------------------------------
void ofApp::drawHelp(){
    
    ofSetColor(ofColor::white);
    ofDrawBitmapString(" Controls\n"
                       "------------------------------------------------------------------------\n"
                       "\n"
                       "Entry/Exit\n"
                       "------------\n"
                       "ESC	        Quit\n"
                       "? key	        Hide/Show Title Screen\n"
                       "o key		Toggle fullscreen/windowed modes\n"
                       "\n"
                       "\n"
                       "Rectangles\n"
                       "------------\n"
                       "Left click	Select\n"
                       "Drag	        Move\n"
                       "TAB key	        Cycle selection of overlapping rectangles under mouse\n"
                       "R key + drag	Resize\n"
                       "F key	        Flip orientation\n"
                       "\n"
                       "\n"
                       "Cloud Addition and Selection\n"
                       "------------\n"
                       "G key       Add cloud to end of collection\n"
                       "Delete key	Remove selected cloud\n"
                       "Left click	Select\n"
                       "Drag	        Move\n"
                       "\n"
                       "\n"
                       "Cloud Parameters\n"
                       "------------\n"
                       "After selecting a cloud, parameters associated with the granular synthesis can be edited.\n"
                       "Most parameters involve keyboard interfacing. Select the key corresponding to the mode,\n"
                       "then press it again to change the value. In some cases, numeric keys are used to enter\n"
                       "specific values. In parameters associated with grain motion, the mouse is used.\n"
                       "\n"
                       "M key             Toggle voice limiter function on/off\n"
                       "V key (+shift)	  Add (remove) voices\n"
                       "A key	          Toggle cloud on/off\n"
                       "D key (+shift)	  Increment (decrement) duration\n"
                       "D key + numbers	  Enter duration value (ms) - press Enter to accept\n"
                       "S key (+shift)	  Increment (decrement) overlap\n"
                       "S key + numbers	  Enter overlap value - press Enter to accept\n"
                       "Z key (+shift)	  Increment (decrement) pitch\n"
                       "Z key + numbers	  Enter pitch value - press Enter to accept\n"
                       "W key	          Change window type (HANNING, TRIANGLE, EXPDEC, REXPDEC, SINC, RANDOM)\n"
                       "W key +\n"
                       "1 through 6	  Jump to specific window type\n"
                       "F key	          Switch grain direction (FORWARD, BACKWARD, RANDOM)\n"
                       "R key	          Enable mouse control of XY extent of grain position randomness\n"
                       "X key	          Enable mouse control of X extent of grain position randomness\n"
                       "Y key	          Enable mouse control of Y extent of grain position randomness\n"
                       "T key             Switch spatialization modes\n"
                       "L key (+shift)    Adjust playback rate LFO frequency\n"
                       "K key (+shift)	  Adjust playback rate LFO amplitude\n"
                       "B key (+shift)	  Adjust cloud volume in dB", 10, 32);
    
    if(voiceLimiterActive){
        ofSetColor(ofColor::green);
        ofDrawBitmapString("Voice Limiter ON", 800, 32);
    }
    else{
        ofSetColor(ofColor::yellow);
        ofDrawBitmapString("Voice Limiter OFF", 800, 32);
    }
    
    ofSetColor(ofColor::white);
    ofDrawBitmapString(oscPortDisplayMessage, 800, 50);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    static float sidewaysMoveSpeed = 10.0f;
    static float upDownMoveSpeed = 10.0f;
    
    static bool negativeFlag = false;//for negative value entry
    
    //cout << "special key" << key <<endl;
    
    switch (key){
        case OF_KEY_SHIFT:
            modkey = OF_KEY_SHIFT;
            break;
        case OF_KEY_ALT:
            modkey = OF_KEY_ALT;
            break;
        case OF_KEY_CONTROL:
            modkey = OF_KEY_CONTROL;
            break;
            
        case OF_KEY_LEFT:
            //move to the left
            position.x -=  sidewaysMoveSpeed;
            mouseX -=sidewaysMoveSpeed;
            break;
        case OF_KEY_RIGHT:
            //move to the right
            position.x += sidewaysMoveSpeed;
            mouseX +=sidewaysMoveSpeed;
            break;
        case OF_KEY_DOWN:
            //move backward
            position.y -= upDownMoveSpeed;
            mouseY +=sidewaysMoveSpeed;
            
            break;
        case OF_KEY_UP:
            //move forward
            position.y += upDownMoveSpeed;
            mouseY -=sidewaysMoveSpeed;
            break;
            
            
        case 9: //tab key
            
            if (selectionIndices->size() > 1){
                soundViews->at(selectedRect)->setSelectState(false);
                selectionIndex++;
                if (selectionIndex >= selectionIndices->size()){
                    selectionIndex = 0;
                }
                selectedRect = selectionIndices->at(selectionIndex);
                soundViews->at(selectedRect)->setSelectState(true);
            }
            break;
        case '1':
            paramString.push_back('1');
            if (currentParam == WINDOW){
                if (selectedCloud >=0){
                    grainCloud->at(selectedCloud)->setWindowType(0);
                }
            }
            
            break;
        case '2':
            paramString.push_back('2');
            if (currentParam == WINDOW){
                if (selectedCloud >=0){
                    grainCloud->at(selectedCloud)->setWindowType(1);
                }
            }
            break;
        case '3':
            paramString.push_back('3');
            if (currentParam == WINDOW){
                if (selectedCloud >=0){
                    grainCloud->at(selectedCloud)->setWindowType(2);
                }
            }
            break;
            
        case '4':
            paramString.push_back('4');
            if (currentParam == WINDOW){
                if (selectedCloud >=0){
                    grainCloud->at(selectedCloud)->setWindowType(3);
                }
            }
            break;
        case'5':
            paramString.push_back('5');
            if (currentParam == WINDOW){
                if (selectedCloud >=0){
                    grainCloud->at(selectedCloud)->setWindowType(4);
                }
            }
            break;
        case'6':
            paramString.push_back('6');
            if (currentParam == WINDOW){
                if (selectedCloud >=0){
                    grainCloud->at(selectedCloud)->setWindowType(5);
                }
            }
            break;
        case'7':
            paramString.push_back('7');
            break;
        case'8':
            paramString.push_back('8');
            break;
        case'9':
            paramString.push_back('9');
            break;
        case '0':
            paramString.push_back('0');
            break;
        case '.':
            paramString.push_back('.');
            break;
            
        case 13://enter key - for saving param string
            if (paramString != ""){
                float value = atof(paramString.c_str());
                
                //cout << "value received " << value << endl;
                switch (currentParam){
                    case DURATION:
                        if (selectedCloud >=0){
                            if (value < 1.0){
                                value = 1.0;
                            }
                            grainCloud->at(selectedCloud)->setDurationMs(value);
                        }
                        break;
                    case OVERLAP:
                        if (selectedCloud >=0){
                            grainCloud->at(selectedCloud)->setOverlap(value);
                        }
                        break;
                    case PITCH:
                        if (selectedCloud >=0){
                            grainCloud->at(selectedCloud)->setPitch(value);
                        }
                        break;
                    case P_LFO_FREQ:
                        if (selectedCloud >=0){
                            grainCloud->at(selectedCloud)->setPitchLFOFreq(value);
                        }
                        break;
                    case P_LFO_AMT:
                        if (selectedCloud >=0){
                            grainCloud->at(selectedCloud)->setPitchLFOAmount(value);
                        }
                        break;
                        
                    case VOLUME:
                        if (selectedCloud >=0){
                            grainCloud->at(selectedCloud)->setVolumeDb(value);
                        }
                    default:
                        break;
                }
                paramString = "";
            }
            // cout << "enter key caught" << endl;
            break;
            
            
            
            
        case 27: //esc key
            cleaningFunction();
            ofExit(1);
            break;
            
        case 'Q'://spatialization
        case 'q':
            break;
            
        case 'O':
        case 'o':
            fullscreen = !fullscreen;
            ofSetFullscreen(fullscreen);
            if(!fullscreen)
                ofSetWindowTitle("Augmenta Borderlands");
            break;
            
        case 'T':
        case 't':
            paramString = "";
            if (selectedCloud >=0){
                if (currentParam != SPATIALIZE){
                    currentParam = SPATIALIZE;
                }else{
                    if (modkey == OF_KEY_SHIFT){
                        if (selectedCloud >=0){
                            int theSpat = grainCloud->at(selectedCloud)->getSpatialMode();
                            grainCloud->at(selectedCloud)->setSpatialMode(theSpat - 1,-1);
                            
                        }
                    }else{
                        if (selectedCloud >=0){
                            int theSpat = grainCloud->at(selectedCloud)->getSpatialMode();
                            grainCloud->at(selectedCloud)->setSpatialMode(theSpat + 1,-1);
                        }
                    }
                }
            }
            break;
            
        case 'S'://overlap control
        case 's':
            paramString = "";
            if (currentParam != OVERLAP){
                currentParam = OVERLAP;
            }else{
                if (modkey == OF_KEY_SHIFT){
                    if (selectedCloud >=0){
                        float theOver =  grainCloud->at(selectedCloud)->getOverlap();
                        grainCloud->at(selectedCloud)->setOverlap(theOver - 0.01f);
                    }
                }else{
                    if (selectedCloud >=0){
                        float theOver =  grainCloud->at(selectedCloud)->getOverlap();
                        grainCloud->at(selectedCloud)->setOverlap(theOver + 0.01f);
                    }
                }
            }
            break;
        case 'R':
        case 'r':
            if (selectedCloud >=0){
                currentParam = MOTIONXY;
            }
            //toggle selection modes
            dragMode = RESIZE;
            break;
        case 'F'://direction
        case 'f':
            paramString = "";
            if (selectedCloud >=0){
                if (currentParam != DIRECTION){
                    currentParam = DIRECTION;
                }else{
                    if (modkey == OF_KEY_SHIFT){
                        if (selectedCloud >=0){
                            int theDir = grainCloud->at(selectedCloud)->getDirection();
                            grainCloud->at(selectedCloud)->setDirection(theDir - 1);
                            
                        }
                    }else{
                        if (selectedCloud >=0){
                            int theDir = grainCloud->at(selectedCloud)->getDirection();
                            grainCloud->at(selectedCloud)->setDirection(theDir + 1);
                        }
                    }
                }
            }
            if (selectedRect >=0){
                soundViews->at(selectedRect)->toggleOrientation();
            }
            //cerr << "Looking from the front" << endl;
            break;
        case 'P'://waveform display on/off
        case 'p':
            
            //            for (int i = 0; i < soundViews->size();i++){
            //                soundViews->at(i)->toggleWaveDisplay();
            //            }
            break;
        case 'W'://window editing for grain
        case 'w':
            paramString = "";
            if (currentParam != WINDOW){
                currentParam = WINDOW;
            }else{
                if (modkey == OF_KEY_SHIFT){
                    if (selectedCloud >=0){
                        int theWin = grainCloud->at(selectedCloud)->getWindowType();
                        grainCloud->at(selectedCloud)->setWindowType(theWin - 1);
                        
                    }
                }else{
                    if (selectedCloud >=0){
                        int theWin = grainCloud->at(selectedCloud)->getWindowType();
                        grainCloud->at(selectedCloud)->setWindowType(theWin + 1);
                    }
                }
            }
            
            break;
            
        case 'B':
        case 'b':
            //cloud volume
            paramString = "";
            if (currentParam != VOLUME){
                currentParam = VOLUME;
            }else{
                if (modkey == OF_KEY_SHIFT){
                    if (selectedCloud >=0){
                        float theVol = grainCloud->at(selectedCloud)->getVolumeDb();
                        grainCloud->at(selectedCloud)->setVolumeDb(theVol - 0.5f);
                    }
                }else{
                    if (selectedCloud >=0){
                        float theVol = grainCloud->at(selectedCloud)->getVolumeDb();
                        grainCloud->at(selectedCloud)->setVolumeDb(theVol + 0.5f);
                    }
                }
            }
            
            
            break;
            
        case '/'://switch to menu view
        case '?':
            menuFlag = !menuFlag;
            
            break;
        case 'G':
        case 'g':
            paramString = "";
            deselect(RECT);
            if (grainCloud != NULL){
                if (modkey == OF_KEY_SHIFT){
                    /*if (grainCloud->size() > 0){
                        grainCloud->pop_back();
                        grainCloudVis->pop_back();
                        numClouds-=1;
                        //cout << "cloud removed" << endl;
                    }
                    if (numClouds == 0){
                        selectedCloud = -1;
                    }else{
                        //still have a cloud so select
                        selectedCloud = numClouds-1;
                        grainCloudVis->at(selectedCloud)->setSelectState(true);
                    }
                    break;
                    */
                }else{
                    int idx = grainCloud->size();
                    /*if (selectedCloud >=0){
                        if (numClouds > 0){
                            grainCloudVis->at(selectedCloud)->setSelectState(false);
                        }
                    }
                    selectedCloud = idx;*/
                    //create audio
                    grainCloud->push_back(new GrainCluster(mySounds,numVoices, settings));
                    //create visualization
                    grainCloudVis->push_back(new GrainClusterVis(mouseX,mouseY,numVoices,soundViews));
                    //select new cloud
                    grainCloudVis->at(idx)->setSelectState(false);
                    //register visualization with audio
                    grainCloud->at(idx)->registerVis(grainCloudVis->at(idx));
                    //grainCloud->at(idx)->toggleActive();
                    numClouds+=1;
                }
                //                        cout << "cloud added" << endl;
                //grainControl->newCluster(mouseX,mouseY,1);
            }
            
            break;
        case 'V': //grain voices (add, delete)
        case 'v':
            paramString = "";
            if (currentParam != NUMGRAINS){
                currentParam = NUMGRAINS;
            }else{
                if (modkey == OF_KEY_SHIFT){
                    if (selectedCloud >=0){
                        if (grainCloud)
                            grainCloud->at(selectedCloud)->removeGrain();
                        //cout << "grain removed" << endl;
                    }
                    
                }else{
                    if (selectedCloud >=0){
                        if (grainCloud)
                            grainCloud->at(selectedCloud)->addGrain();
                        //cout << "grain added" << endl;
                    }
                }
            }
            break;
            
        case 'D':
        case 'd':
            paramString = "";
            if (currentParam != DURATION){
                currentParam = DURATION;
            }else{
                if (modkey == OF_KEY_SHIFT){
                    if (selectedCloud >=0){
                        float theDur = grainCloud->at(selectedCloud)->getDurationMs();
                        grainCloud->at(selectedCloud)->setDurationMs(theDur - 5.0f);
                    }
                }else{
                    if (selectedCloud >=0){
                        float theDur = grainCloud->at(selectedCloud)->getDurationMs();
                        grainCloud->at(selectedCloud)->setDurationMs(theDur + 5.0f);
                    }
                }
            }
            break;
        case 'I':
        case 'i':
            break;
            
            
        case 'L':
        case 'l':
            paramString = "";
            if (currentParam != P_LFO_FREQ){
                currentParam = P_LFO_FREQ;
            }else{
                if (modkey == OF_KEY_SHIFT){
                    if (selectedCloud >=0){
                        float theLFOFreq = grainCloud->at(selectedCloud)->getPitchLFOFreq();
                        grainCloud->at(selectedCloud)->setPitchLFOFreq(theLFOFreq - 0.01f);
                    }
                }else{
                    if (selectedCloud >=0){
                        float theLFOFreq = grainCloud->at(selectedCloud)->getPitchLFOFreq();
                        grainCloud->at(selectedCloud)->setPitchLFOFreq(theLFOFreq + 0.01f);
                    }
                }
            }
            break;
            
        case 'K':
        case 'k':
            paramString = "";
            if (currentParam != P_LFO_AMT){
                currentParam = P_LFO_AMT;
            }else{
                if (modkey == OF_KEY_SHIFT){
                    if (selectedCloud >=0){
                        float theLFOAmt = grainCloud->at(selectedCloud)->getPitchLFOAmount();
                        grainCloud->at(selectedCloud)->setPitchLFOAmount(theLFOAmt - 0.001f);
                    }
                }else{
                    if (selectedCloud >=0){
                        float theLFOAmnt = grainCloud->at(selectedCloud)->getPitchLFOAmount();
                        grainCloud->at(selectedCloud)->setPitchLFOAmount(theLFOAmnt + 0.001f);
                    }
                }
            }
            break;
        case 'H':
        case 'h':
            showHelpMenu = true;
            break;
        case ' '://add delete
            
            break;
            
        case 'X':
        case 'x':
            paramString = "";
            if (selectedCloud >= 0){
                currentParam = MOTIONX;
            }
            break;
        case 'Y':
        case 'y':
            paramString = "";
            if (selectedCloud >= 0){
                currentParam = MOTIONY;
            }
            break;
            
        case 'Z':
        case 'z':
            paramString = "";
            if (currentParam != PITCH){
                currentParam = PITCH;
            }else{
                if (modkey == OF_KEY_SHIFT){
                    if (selectedCloud >=0){
                        float thePitch =  grainCloud->at(selectedCloud)->getPitch();
                        grainCloud->at(selectedCloud)->setPitch(thePitch - 0.01f);
                    }
                }else{
                    if (selectedCloud >=0){
                        float thePitch =  grainCloud->at(selectedCloud)->getPitch();
                        grainCloud->at(selectedCloud)->setPitch(thePitch + 0.01f);
                    }
                }
            }
            break;
            
        case '-':
        case '_':
            paramString.insert(0,"-");
            break;
        case OF_KEY_DEL:
        case OF_KEY_BACKSPACE:
            if (paramString == ""){
                if (selectedCloud >=0 && !belongsToAugmenta(grainCloud->at(selectedCloud)->getId())){
                    grainCloud->erase(grainCloud->begin() + selectedCloud);
                    grainCloudVis->erase(grainCloudVis->begin() + selectedCloud);
                    selectedCloud = -1;
                    numClouds-=1;
                }
            }else{
                if (paramString.size () > 0)  paramString.resize (paramString.size () - 1);
            }
            
            break;
        case 'A':
        case 'a':
            paramString = "";
            if (selectedCloud >=0){
                grainCloud->at(selectedCloud)->toggleActive();
            }
            break;
        case 'M':
        case 'm':
            voiceLimiterActive = !voiceLimiterActive;
            break;
            
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
    switch (key) {
        case OF_KEY_SHIFT:
        case OF_KEY_ALT:
        case OF_KEY_CONTROL:
            modkey = -1;
            break;
            
        case 'a':
            break;
        case 'R':
        case 'r':
            dragMode = MOVE;
            lastDragX = veryHighNumber;
            lastDragY = veryHighNumber;
            break;
        case 'h':
        case 'H':
            showHelpMenu = false;
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){
    
    updateMouseCoords(x,y);
    
    if (selectedCloud >=0){
        switch (currentParam) {
            case MOTIONX:
                grainCloudVis->at(selectedCloud)->setXRandExtent(mouseX);
                break;
            case MOTIONY:
                grainCloudVis->at(selectedCloud)->setYRandExtent(mouseY);
                break;
            case MOTIONXY:
                grainCloudVis->at(selectedCloud)->setRandExtent(mouseX,mouseY);
                break;
            default:
                break;
        }
    }
    
    //ofShowCursor(); // Hotfix of 0.8.4 bug if you want to show real cursor
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    //update mouse coordinates based on drag position
    updateMouseCoords(x,y);
    int xDiff = 0;
    int yDiff = 0;
    
    if (selectedCloud >= 0){
        grainCloudVis->at(selectedCloud)->updateCloudPosition(mouseX,mouseY);
    }else{
        
        switch (dragMode) {
            case MOVE:
                if( (lastDragX != veryHighNumber) && (lastDragY != veryHighNumber)){
                    
                    if (selectedRect >=0){                    //movement case
                        soundViews->at(selectedRect)->move(mouseX - lastDragX,mouseY - lastDragY);
                    }
                }
                lastDragX = mouseX;
                lastDragY = mouseY;
                break;
                
            case RESIZE:
                if( (lastDragX != veryHighNumber) && (lastDragY != veryHighNumber)){
                    //cout << "drag ok" << endl;
                    //for width height - use screen coords
                    
                    if (selectedRect >= 0){
                        xDiff = x - lastDragX;
                        yDiff = y - lastDragY;
                        //get width and height
                        float newWidth = soundViews->at(selectedRect)->getWidth();
                        float newHeight = soundViews->at(selectedRect)->getHeight();
                        
                        int thresh = 0;
                        //check motion mag
                        if (xDiff < -thresh){
                            newWidth = newWidth * 0.8 + 0.2*(newWidth * (0.85 - abs(xDiff/50.0)));
                        }else{
                            if (xDiff > thresh)
                                newWidth = newWidth * 0.8 + 0.2*(newWidth * (1.1 + abs(xDiff/50.0)));
                        }
                        if (yDiff > thresh){
                            newHeight = newHeight * 0.8 + 0.2*(newHeight * (1.1 + abs(yDiff/50.0)));
                        }else{
                            if (yDiff < -thresh)
                                newHeight = newHeight * 0.8 + 0.2*(newHeight * (0.85 - abs(yDiff/50.0)));
                        }
                        
                        //update width and height
                        soundViews->at(selectedRect)->setWidthHeight(newWidth,newHeight);
                        
                    }
                    
                    
                }
                lastDragX = x;
                lastDragY = y;
                break;
            default:
                break;
        }
    }

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    //look for selections if button is down
    if ((button == OF_MOUSE_BUTTON_1) || (button == OF_MOUSE_BUTTON_2)){
        
        paramString = "";
        
        //hide menu
        if (menuFlag == true)
            menuFlag = false;
        
        deselect(CLOUD);
        //deselect existing selections
        deselect(RECT);
        //exit parameter editing
        currentParam = -1;
        
        lastDragX = veryHighNumber;
        lastDragY = veryHighNumber;
        //first check grain clouds to see if we have selection
        for (int i = 0; i < grainCloudVis->size(); i++){
            if (grainCloudVis->at(i)->select(mouseX, mouseY) == true){
                grainCloudVis->at(i)->setSelectState(true);
                selectedCloud = i;
                break;
            }
        }
        
        
        //clear selection buffer
        if (selectionIndices)
            delete selectionIndices;
        //allocate new buffer
        selectionIndices = new vector<int>;
        selectionIndex = 0;
        //if grain cloud is not selected - search for rectangle selection
        if (selectedCloud < 0){
            //search for selections
            resizeDir = false;//set resize direction to horizontal
            for (int i = 0; i < soundViews->size(); i++){
                if (soundViews->at(i)->select(mouseX,mouseY) == true){
                    selectionIndices->push_back(i);
                    //soundViews->at(i)->setSelectState(true);
                    //selectedRect = i;
                    //break;
                }
            }
            
            if (selectionIndices->size() > 0){
                selectedRect = selectionIndices->at(0);
                soundViews->at(selectedRect)->setSelectState(true);
            }
        }
        
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
// Augmenta Events
//--------------------------------------------------------------
void ofApp::onPersonEntered( Augmenta::EventArgs & augmentaEvent ){
    // Translate relative position of the person (between 0 & 1) to a screen position in pixels, restricted to the interactive area
    int posX = ofGetWidth() * (augmentaEvent.person->centroid.x * interactiveArea.width + interactiveArea.x);
    int posY = ofGetHeight() * (augmentaEvent.person->centroid.y * interactiveArea.height + interactiveArea.y);
    
    // Create a new grain based on person
    int idx = grainCloud->size();
    
    //create audio
    grainCloud->push_back(new GrainCluster(augmentaEvent.person->pid, mySounds,numVoices, settings));
    //create visualization
    grainCloudVis->push_back(new GrainClusterVis(posX,posY,numVoices,soundViews));
    //select new cloud
    grainCloudVis->at(idx)->setSelectState(false);
    //register visualization with audio
    grainCloud->at(idx)->registerVis(grainCloudVis->at(idx));
    //grainCloud->at(idx)->toggleActive();
    numClouds+=1;
    
    //update voices with voice limiter
    if(voiceLimiterActive)
        voiceLimiter();
}

void ofApp::onPersonUpdated( Augmenta::EventArgs & augmentaEvent ){
    // Translate relative position of the person (between 0 & 1) to a screen position in pixels, restricted to the interactive area
    int posX = ofGetWidth() * (augmentaEvent.person->centroid.x * interactiveArea.width + interactiveArea.x);
    int posY = ofGetHeight() * (augmentaEvent.person->centroid.y * interactiveArea.height + interactiveArea.y);
    
    // Update grain's position with person associated
    grainCloudVis->at(getIndexOfGrainCloudWithPID(augmentaEvent.person->pid))->updateCloudPosition(posX,posY);
}

void ofApp::onPersonWillLeave( Augmenta::EventArgs & augmentaEvent ){
    int pid = augmentaEvent.person->pid;
    
    if (grainCloud->size() > 0){
        //GrainCluster* grainCloudToDelete = getGrainCloudWithPID(pid);
        //GrainClusterVis* grainCloudVisToDelete = grainCloudToDelete->getRegisteredVis();
        
        int index = getIndexOfGrainCloudWithPID(pid);
        
        if(selectedCloud != -1){
            int selectedCloudPid = grainCloud->at(selectedCloud)->getId();
            
            // if the person leaving is selected, we deselect it
            if(selectedCloudPid == pid){
                deselect(CLOUD);
            }
            // if an other person is selected and is after the person leaving in the vector, we update its index
            else if(selectedCloud >= index){
                selectedCloud--;
            }
        }

        grainCloud->erase(grainCloud->begin() + index);
        grainCloudVis->erase(grainCloudVis->begin() + index);
        
        // TO FIX ? Do not delete grain cloud, because GrainVoice Destructor delete the sounds and singleton Window
        //delete grainCloudVisToDelete;
        //delete grainCloudToDelete;
        
        numClouds-=1;
    }
    
    //update voices with voice limiter
    if(voiceLimiterActive)
        voiceLimiter();
}

GrainCluster* ofApp::getGrainCloudWithPID(int pid){
    for(int i=0; i<grainCloud->size(); i++){
        if(grainCloud->at(i)->getId() == pid)
            return grainCloud->at(i);
    }
    return NULL;
}

int ofApp::getIndexOfGrainCloudWithPID(int pid){
    for(int i=0; i<grainCloud->size(); i++){
        if(grainCloud->at(i)->getId() == pid)
            return i;
    }
    return -1;
}

bool ofApp::belongsToAugmenta(int pid){
    vector<Augmenta::Person *> people = augmentaReceiver.getPeople();
    
    for(int i=0; i<people.size(); i++){
        if(people[i]->pid == pid)
            return true;
    }
    return false;
}

void ofApp::voiceLimiter(){
    // Distribute the voices in clouds
    float voicesPerCloud = (float)voicesLimit / (float)grainCloud->size();
    
    // There is not enough clouds to reach the limit : we set voices to the max limit per cloud
    if(voicesPerCloud >= maxVoicesPerCloud){
        for(int i=0; i<grainCloud->size(); i++){
            grainCloud->at(i)->setGrains(maxVoicesPerCloud);
        }
    }
    // There is enough clouds to reach the limit, but there is not more clouds than the limit : number of voices is an integer between 1 and the voicesPerCloud limit
    else if(voicesPerCloud < maxVoicesPerCloud && voicesPerCloud >= 1){
        for(int i=0; i<grainCloud->size(); i++){
            grainCloud->at(i)->setGrains((int)voicesPerCloud);
        }
    }
    // There is more clouds than the limit : some clouds will not have any voice
    else if(voicesPerCloud < 1){
        int remainingVoices = voicesLimit;
        for(int i=0; i<grainCloud->size(); i++){
            if(remainingVoices == 0)
                grainCloud->at(i)->setGrains(0);
            else{
                // Distribute voices randomly
                float probability = (float)remainingVoices / (float)(grainCloud->size()-i);
                
                if((float)std::rand()/RAND_MAX <= probability){
                    grainCloud->at(i)->setGrains(1);
                    remainingVoices--;
                }
                else{
                    grainCloud->at(i)->setGrains(0);
                }
            }
        }
    }
}
