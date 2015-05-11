#pragma once

#include "ofMain.h"

//my includes
#include "Borderlands/theglobals.h"

//audio related
#include "Borderlands/MyRtAudio.h"
#include "Borderlands/AudioFileSet.h"
#include "Borderlands/Window.h"

//graphics related
#include "Borderlands/SoundRect.h"

//graphics and audio related
#include "Borderlands/GrainCluster.h"


int audioCallback( void * outputBuffer, void * inputBuffer, unsigned int numFrames, double streamTime,RtAudioStreamStatus status, void * userData);

class ofApp : public ofBaseApp{
	public:
        
        void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        //grain cloud audio objects
        vector<GrainCluster *> * grainCloud = NULL;
        //global time increment - samples per second
        //global time is incremented in audio callback
        const double samp_time_sec = (double) 1.0 / (double)MY_SRATE;
        bool menuFlag = true;
    
    private:
        ofFbo m_fbo; // frame buffer object
    
        //-----------------------------------------------------------------------------
        // Shared Data Structures, Global parameters
        //-----------------------------------------------------------------------------
        //audio system
        MyRtAudio * theAudio = NULL;
        //library path
        string g_audioPath = string("./loops/");
        //parameter string
        string paramString = "";
        //desired audio buffer size
        unsigned int g_buffSize = 1024;
        //audio files
        vector <AudioFile *> * mySounds = NULL;
        //audio file visualization objects
        vector <SoundRect *> * soundViews = NULL;
        //grain cloud visualization objects
        vector<GrainClusterVis *> * grainCloudVis;
        //cloud counter
        unsigned int numClouds = 0;
        
        
        //Initial camera movement vars
        //my position
        ofPoint position = ofPoint(0.0,0.0,0.0f);
        
        
        //ENUMS
        //user selection mode
        enum{RECT,CLOUD};
        enum{MOVE,RESIZE};
        //default selection mode
        int selectionMode = CLOUD;
        int dragMode = MOVE;
        bool resizeDir = false; //for rects
        //rubber band select params
        int rb_anchor_x = -1;
        int rb_anchor_y = -1;
        
        //not used yet - for multiple selection
        vector<int> * selectionIndices = new vector<int>;
        
        //selection helper vars
        int selectedCloud = -1;
        int selectedRect = -1;
        int selectionIndex = 0;
        
        //cloud parameter changing
        enum{NUMGRAINS,DURATION,WINDOW, MOTIONX, MOTIONY,MOTIONXY,DIRECTION,OVERLAP, PITCH, ANIMATE,P_LFO_FREQ,P_LFO_AMT,SPATIALIZE,VOLUME};
        //flag indicating parameter change
        bool paramChanged = false;
        unsigned int currentParam = NUMGRAINS;
        double lastParamChangeTime = 0.0;
        double tempParamVal = -1.0;
        
        
        
        
        //mouse coordinate initialization
        int mouseX = -1;
        int mouseY = -1;
        long veryHighNumber = 50000000;
        long lastDragX = veryHighNumber;
        long lastDragY = veryHighNumber;
        
        
        //--------------------------------------------------------------------------------
        // FUNCTION PROTOTYPES
        //--------------------------------------------------------------------------------
        
        void idleFunc();
        void displayFunc();
        void reshape(int w, int h);
        void specialFunc(int key, int x, int y);
        void keyboardFunc(unsigned char key, int x, int y);
        void keyUpFunc(unsigned char key, int x, int y);
        void deselect(int mode);
        
        void mouseFunc(int button, int state, int x, int y);
        void mouseDrag(int x, int y);
        void mousePassiveMotion(int x, int y);
        void updateMouseCoords(int x, int y);
        void initialize();
        void draw_string( GLfloat x, GLfloat y, GLfloat z, const char * str, GLfloat scale);
        void printUsage();
        void printParam();
        void drawAxis();
        
        void cleaningFunction();
};
