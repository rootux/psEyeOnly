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
	setupPsEye();
	cameraFbo.allocate(1280, 720);
	cameraFbo.black();
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

//--------------------------------------------------------------
void ofApp::update(){
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

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
