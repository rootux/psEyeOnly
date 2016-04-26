#include "ofApp.h"

//--------------------------------------------------------------

static const int ITUR_BT_601_CY = 1220542;
static const int ITUR_BT_601_CUB = 2116026;
static const int ITUR_BT_601_CUG = -409993;
static const int ITUR_BT_601_CVG = -852492;
static const int ITUR_BT_601_CVR = 1673527;
static const int ITUR_BT_601_SHIFT = 20;

static void yuv422_to_rgba(const uint8_t *yuv_src, const int stride, uint8_t *dst, const int width, const int height)
{
	const int bIdx = 2;
	const int uIdx = 0;
	const int yIdx = 0;

	const int uidx = 1 - yIdx + uIdx * 2;
	const int vidx = (2 + uidx) % 4;
	int j, i;

#define _max(a, b) (((a) > (b)) ? (a) : (b))
#define _saturate(v) static_cast<uint8_t>(static_cast<uint32_t>(v) <= 0xff ? v : v > 0 ? 0xff : 0)

	for (j = 0; j < height; j++, yuv_src += stride)
	{
		uint8_t* row = dst + (width * 4) * j; // 4 channels

		for (i = 0; i < 2 * width; i += 4, row += 8)
		{
			int u = static_cast<int>(yuv_src[i + uidx]) - 128;
			int v = static_cast<int>(yuv_src[i + vidx]) - 128;

			int ruv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CVR * v;
			int guv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CVG * v + ITUR_BT_601_CUG * u;
			int buv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CUB * u;

			int y00 = _max(0, static_cast<int>(yuv_src[i + yIdx]) - 16) * ITUR_BT_601_CY;
			row[2 - bIdx] = _saturate((y00 + ruv) >> ITUR_BT_601_SHIFT);
			row[1] = _saturate((y00 + guv) >> ITUR_BT_601_SHIFT);
			row[bIdx] = _saturate((y00 + buv) >> ITUR_BT_601_SHIFT);
			row[3] = (0xff);

			int y01 = _max(0, static_cast<int>(yuv_src[i + yIdx + 2]) - 16) * ITUR_BT_601_CY;
			row[6 - bIdx] = _saturate((y01 + ruv) >> ITUR_BT_601_SHIFT);
			row[5] = _saturate((y01 + guv) >> ITUR_BT_601_SHIFT);
			row[4 + bIdx] = _saturate((y01 + buv) >> ITUR_BT_601_SHIFT);
			row[7] = (0xff);
		}
	}
}

void ofApp::setup(){
	setupSpout();
	setupPsEye();
	cameraFbo.allocate(640, 480);
	cameraFbo.black();
}

void ofApp::setupSpout() {
	// ====== SPOUT =====
	spoutsender = new SpoutSender;			// Create a Spout sender object
	bInitialized = false;		        // Spout sender initialization
	strcpy(sendername, "OF Spout Sender");	// Set the sender name
	ofSetWindowTitle(sendername);			// show it on the title bar
											// Create an OpenGL texture for data transfers
	sendertexture = 0; // make sure the ID is zero for the first time
	InitGLtexture(sendertexture, ofGetWidth(), ofGetHeight());

	// 3D drawing setup for a sender
	glEnable(GL_DEPTH_TEST);							// enable depth comparisons and update the depth buffer
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	ofDisableArbTex();									// needed for textures to work
}

void ofApp::setupPsEye() {
	if (eye) {
		return;
	}
	using namespace ps3eye;
	std::vector<PS3EYECam::PS3EYERef> devices(PS3EYECam::getDevices());
	if (devices.size())
	{
		eye = devices.at(0);
		bool res = eye->init(640, 480, 60);
		if (res) {
			eye->start();
			eye->setExposure(100);
			videoFrame = new unsigned char[eye->getWidth()*eye->getHeight() * 4];
			videoTexture.allocate(eye->getWidth(), eye->getHeight(), GL_RGBA);
		}
		else {
			eye = NULL;
		}
	}
	else {
		ofLogError() << "Failed to open PS eye!";
	}
}

bool ofApp::InitGLtexture(GLuint &texID, unsigned int width, unsigned int height)
{
	if (texID != 0) glDeleteTextures(1, &texID);

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

//--------------------------------------------------------------
void ofApp::update(){
	cameraFbo.black();
	cameraFbo.begin();
	if (eye)
	{
		uint8_t* new_pixels = eye->getFrame();
		yuv422_to_rgba(new_pixels, eye->getRowBytes(), videoFrame, eye->getWidth(), eye->getHeight());
		videoTexture.loadData(videoFrame, eye->getWidth(), eye->getHeight(), GL_RGBA);
		free(new_pixels);
	}

	videoTexture.draw(cameraFbo.getWidth(), 0, -cameraFbo.getWidth(), cameraFbo.getHeight());
	cameraFbo.end();
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(0, 0);
	drawSource();
	drawSpout();
}

void ofApp::drawSpout() {
	char str[256];
	ofSetColor(255);

	// ====== SPOUT =====
	// A render window must be available for Spout initialization and might not be
	// available in "update" so do it now when there is definitely a render window.
	if (!bInitialized) {
		bInitialized = spoutsender->CreateSender(sendername, ofGetWidth(), ofGetHeight()); // Create the sender
	}
	// ===================


	// - - - - - - - - - - - - - - - - - - - - - - - - -
	// Draw 3D graphcs demo - this could be anything
	/*
	ofPushMatrix();
	glTranslatef((float)ofGetWidth() / 2.0, (float)ofGetHeight() / 2.0, 0); // centre
	//ofRotateY(rotX); // rotate - must be float
	//ofRotateX(rotY);
	//myTextureImage.getTextureReference().bind(); // bind our texture
	ofDrawBox(0.4*(float)ofGetHeight()); // Draw the graphic
	ofPopMatrix();*/
	//rotX += 0.5;
	//rotY += 0.5;
	// - - - - - - - - - - - - - - - - - - - - - - - - -

	// ====== SPOUT =====
	if (bInitialized) {

		//if (ofGetWidth() > 0 && ofGetHeight() > 0) { // protect against user minimize

													 // Grab the screen into the local spout texture
			/*glBindTexture(GL_TEXTURE_2D, sendertexture);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, ofGetWidth(), ofGetHeight());
			glBindTexture(GL_TEXTURE_2D, 0);
			
			// Send the texture out for all receivers to use
			spoutsender->SendTexture(sendertexture, GL_TEXTURE_2D, ofGetWidth(), ofGetHeight());
			*/
			const auto& texData = cameraFbo.getTexture().getTextureData();
			spoutsender->SendTexture(texData.textureID, texData.textureTarget, texData.width, texData.height, false);

			// Show what it is sending
			ofSetColor(255);
			sprintf(str, "Sending as : [%s]", sendername);
			ofDrawBitmapString(str, 20, 20);

			// Show fps
			sprintf(str, "fps: %3.3d", (int)ofGetFrameRate());
			ofDrawBitmapString(str, ofGetWidth() - 120, 20);

		//}

	}
	// ===================

}

void ofApp::drawSource(int _x, int _y, int _width, int _height) {
	ofPushStyle();
	ofEnableBlendMode(OF_BLENDMODE_DISABLED);
	cameraFbo.draw(_x, _y, _width, _height);
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
	// ====== SPOUT =====
	// Update the sender texture to receive the new dimensions
	// Change of width and height is handled within the SendTexture function
	if (w > 0 && h > 0) // protect against user minimize
		InitGLtexture(sendertexture, w, h);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
