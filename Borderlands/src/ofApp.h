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

//augmenta
#include "ofxAugmenta.h"
//xml settings
#include "ofxXmlSettings.h"

#ifdef MAC_OS_X_VERSION_10_6
#include "ofxSyphon.h"
#endif

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
    
        // Augmenta event listeners
        void onPersonEntered( Augmenta::EventArgs & augmentaEvent );
        void onPersonUpdated( Augmenta::EventArgs & augmentaEvent );
        void onPersonWillLeave( Augmenta::EventArgs & augmentaEvent );
    
        //grain cloud audio objects
        vector<GrainCluster *> * grainCloud;
        //global time increment - samples per second
        //global time is incremented in audio callback
        const double samp_time_sec = (double) 1.0 / (double)MY_SRATE;
        bool menuFlag;
    
    private:
        ofFbo m_fbo; // frame buffer object
    
        ofTrueTypeFont myFont; // font used for print parameters
    
        bool fullscreen;
    
        // Augmenta object to receive tracking information
        Augmenta::Receiver augmentaReceiver;
        // OSC Port to receive Augmenta Data
        int oscPort;
        // OSC debug message
        string oscPortDisplayMessage;
        // Augmenta interactive area
        bool generateOutsideInteractiveArea;
        ofRectangle interactiveArea;
    
        // XML Settings
        ofxXmlSettings settings;
    
        //-----------------------------------------------------------------------------
        // Shared Data Structures, Global parameters
        //-----------------------------------------------------------------------------
        //audio system
        MyRtAudio * theAudio;
        //library path
        string drawAudioPath; // for start screen
        string g_audioPath;
        //parameter string
        string paramString;
        //desired audio buffer size
        unsigned int g_buffSize;
        // load sounds
        AudioFileSet* newFileMgr;
        //audio files
        vector <AudioFile *> * mySounds;
        //audio file visualization objects
        vector <SoundRect *> * soundViews;
        //grain cloud visualization objects
        vector<GrainClusterVis *> * grainCloudVis;
        //cloud counter
        unsigned int numClouds;
        
        
        //Initial camera movement vars
        //my position
        ofPoint position;
        
        
        //ENUMS
        //user selection mode
        enum{RECT,CLOUD};
        enum{MOVE,RESIZE};
        //default selection mode
        int selectionMode;
        int dragMode;
        bool resizeDir; //for rects
        //rubber band select params
        int rb_anchor_x;
        int rb_anchor_y;
        
        //not used yet - for multiple selection
        vector<int> * selectionIndices;
        
        //selection helper vars
        int selectedCloud;
        int selectedRect;
        int selectionIndex;
        int editMode;
        bool isEditingParameter;
    
        //cloud parameter changing
        enum{NUMGRAINS,DURATION,WINDOW, MOTIONX, MOTIONY,MOTIONXY,DIRECTION,OVERLAP, PITCH, ANIMATE,P_LFO_FREQ,P_LFO_AMT,SPATIALIZE,VOLUME};
        //flag indicating parameter change
        bool paramChanged;
        unsigned int currentParam;
        double lastParamChangeTime;
        double tempParamVal;
        
        long veryHighNumber;
        long lastDragX;
        long lastDragY;
        unsigned int mousePressedCounter;
        unsigned int lastClickFrame;
    
        //keyboard modifier key
        int modkey;
    
        //flag for help menu display
        bool showHelpMenu;
    
        //flag for showing cursor
        bool showCursor;
    
        //flag for voice limiter function
        bool voiceLimiterActive;
    
        //voice limiter variables
        int voicesLimit;
        int maxVoicesPerCloud;
    
        //initial number of voices when a new grain is created
        int numVoices;
    
        #ifdef MAC_OS_X_VERSION_10_6
        ofxSyphonServer syphonServer;
        string syphonServerName;
        #endif
    
        //--------------------------------------------------------------------------------
        // FUNCTION PROTOTYPES
        //--------------------------------------------------------------------------------
    
        void init();
        void loadSettings();
    
        void idleFunc();
        void displayFunc();
        void reshape(int w, int h);
        void specialFunc(int key, int x, int y);
        void keyboardFunc(unsigned char key, int x, int y);
        void keyUpFunc(unsigned char key, int x, int y);
        void deselect(int mode);
    
        void mouseDoubleClicked(int x, int y, int button);
        void mouseFunc(int button, int state, int x, int y);
        void mouseDrag(int x, int y);
        void mousePassiveMotion(int x, int y);
        void updateMouseCoords(int x, int y);
        void initialize();
        void draw_string( GLfloat x, GLfloat y, GLfloat z, const char * str, GLfloat scale);
        void printUsage();
        void printParam();
        void drawAxis();
        void drawVisuals();
        void drawHelp();
        void toggleFullscreen();
    
        void cleaningFunction();
    
        GrainCluster* getGrainCloudWithPID(int pid);
        int getIndexOfGrainCloudWithPID(int pid);
    
        bool belongsToAugmenta(int pid);
    
        void voiceLimiter();
    
        float convertValueRange(float oldValue, float oldMin, float oldMax, float newMin, float newMax);
};
