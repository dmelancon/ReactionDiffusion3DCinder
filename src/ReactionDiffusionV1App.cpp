#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ReactionDiffusionV1App : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void ReactionDiffusionV1App::setup()
{
}

void ReactionDiffusionV1App::mouseDown( MouseEvent event )
{
}

void ReactionDiffusionV1App::update()
{
}

void ReactionDiffusionV1App::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( ReactionDiffusionV1App, RendererGl )
