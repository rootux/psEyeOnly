#pragma once

#include "ofMain.h"

#include "ofxGui.h"
#include "ps3eye.h"
#include "ofxFlowTools.h"
#include "..\..\..\SpoutSDK\Spout.h" // Spout SDK

using namespace flowTools;
class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		ftFbo				cameraFbo;
		ps3eye::PS3EYECam::PS3EYERef eye = NULL;
		unsigned char *		videoFrame;
		ofTexture			videoTexture;

		void setupPsEye();

		void				drawSource() { drawSource(0, 0, ofGetWindowWidth(), ofGetWindowHeight()); }
		void				drawSource(int _x, int _y, int _width, int _height);

		//Spout
		SpoutSender *spoutsender; // A sender object
		char sendername[256];     // Sender name
		GLuint sendertexture;     // Local OpenGL texture used for sharing
		bool bInitialized;        // Initialization result

		void setupSpout();

		void drawSpout();
};
