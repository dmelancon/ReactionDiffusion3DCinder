#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"
#include "cinder/gl/Vbo.h"
#include "Resources.h"

#define SIZE 800
using namespace ci;
using namespace ci::app;
using namespace std;

class ReactionDiffusionV1App : public AppNative {
  public:
    void	prepareSettings( Settings *settings );
	void	setup();
	void	update();
	void	draw();
	void	keyDown( KeyEvent event );
	void	mouseMove( MouseEvent event );
	void	mouseDown( MouseEvent event );
	void	mouseDrag( MouseEvent event );
	void	mouseUp( MouseEvent event );
	void	resetFBOs();
    void    createVBOMesh();
    params::InterfaceGlRef	mParams;
    
	int				mCurrentFBO;
	int				mOtherFBO;
	gl::Fbo			mFBOs[2];
	gl::GlslProgRef	mShader;
    gl::GlslProgRef	mShaderRefraction;
    const Vec2i	kWindowSize	= Vec2i( SIZE, SIZE );

	gl::Texture		mTexture;

	gl::VboMeshRef mWrapper;
	Vec2f			mMouse;
	bool			mMousePressed;
	bool            textFeed = false;
	float			mReactionU;
	float			mReactionV;
	float			mReactionK;
	float			mReactionF;
    float			mScale;
    float           mTilt;
    int             mSize;
    int             mSpeed;
	static const int		FBO_WIDTH = SIZE, FBO_HEIGHT = SIZE;
    
    CameraPersp mCam;
};



void ReactionDiffusionV1App::prepareSettings(Settings *settings)
    {
//        settings->enableHighDensityDisplay();
//        settings->enableMultiTouch( false );

 settings->setWindowSize(SIZE,SIZE);
settings->setFrameRate( 60.0f );

}

void ReactionDiffusionV1App::mouseDown( MouseEvent event )
{
    mMousePressed = true;
}

void ReactionDiffusionV1App::mouseUp(MouseEvent event)
{
    mMousePressed = false;
}


void ReactionDiffusionV1App::mouseMove( MouseEvent event )
{
	mMouse = event.getPos();
}

void ReactionDiffusionV1App::mouseDrag( MouseEvent event )
{
	mMouse = event.getPos();
}
void ReactionDiffusionV1App::createVBOMesh(){
   int trisize =1;
   int  NUMX = SIZE/trisize;
    int NUMY = SIZE/trisize;
    int numVertices = NUMX*NUMY;
    int totalTriangles = ( NUMX - 1 ) * ( NUMY - 1 );
    
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticColorsRGBA();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();
    layout.setStaticNormals();
    
    vector<Vec3f> positions;
    vector<Vec3f> normals;
    vector<Vec2f> texCoords;
    vector<uint32_t>   indices;
    vector<ColorA> colors;
    
    mWrapper = gl::VboMesh::create(numVertices, totalTriangles * 6, layout, GL_TRIANGLES);
    for(int x=0;x<NUMX;x+=trisize){
        for(int y=0;y<NUMY;y+=trisize){
            positions.push_back( Vec3f(x,y,0. ) );
            if( ( x + trisize < NUMX ) && ( y + trisize < NUMY) ) {
                indices.push_back( (x+0) * NUMY + (y+0) );
                indices.push_back( (x+0) * NUMY + (y+trisize) );
                indices.push_back( (x+trisize) * NUMY + (y+trisize) );
                indices.push_back( (x+0) * NUMY + (y+0) );
                indices.push_back( (x+trisize) * NUMY + (y+trisize) );
                indices.push_back( (x+trisize) * NUMY + (y+0) );
            }
            
            Vec3f normal = cross(Vec3f(x, y+trisize,0.f)-Vec3f(x, y,0.f), Vec3f(x+trisize, y,0.f)-Vec3f(x, y,0.f)).normalized();
            normals.push_back(normal);
            texCoords.push_back( Vec2f( (x / (float)NUMX), (y / (float)NUMY) ) );
            colors.push_back(ColorA(1.,1.,1.,8.));
        }
        
    }
    mWrapper->bufferIndices(indices);
    mWrapper->bufferPositions(positions);
    mWrapper->bufferColorsRGBA(colors);
    mWrapper->bufferTexCoords2d(0, texCoords);
    mWrapper->bufferNormals(normals);
    mWrapper->unbindBuffers();
//    indices.clear();
//    texCoords.clear();
//    positions.clear();
}

void ReactionDiffusionV1App::setup()
{
    mSpeed = 5;
    mTilt = -300;
    mReactionU = 0.25f;
	mReactionV = 0.04f;
	mReactionK = 0.047f;
	mReactionF = 0.131f;
    mScale =100.0;
    mSize = 3;
	mMousePressed = false;
    mParams = params::InterfaceGl::create( "Parameters", Vec2i( 175, 200 ) );
	mParams->addParam( "Reaction u", &mReactionU, "min=0.0 max=0.4 step=0.01 keyIncr=u keyDecr=U" );
	mParams->addParam( "Reaction v", &mReactionV, "min=0.0 max=0.4 step=0.01 keyIncr=v keyDecr=V" );
	mParams->addParam( "Reaction k", &mReactionK, "min=0.0 max=1.0 step=0.001 keyIncr=k keyDecr=K" );
	mParams->addParam( "Reaction f", &mReactionF, "min=0.0 max=1.0 step=0.001 keyIncr=f keyDecr=F" );
	mParams->addParam( "Scale", &mScale, "min=0.0 max=300.0 step=0.5 keyIncr=f keyDecr=F" );
    mParams->addParam( "speed", &mSpeed, "min = 1 max= 25 step=1 ");
    mParams->addParam( "Tilt", &mTilt, "min=-1000. max=1000. step=1.0 keyIncr=f keyDecr=F" );
    mParams->addParam( "Size", &mSize, "min=1. max=16. step=1.0 keyIncr=f keyDecr=F" );

    gl::Fbo::Format format;
	format.enableDepthBuffer( false );
    format.setColorInternalFormat(GL_RGBA32F_ARB);
    mCurrentFBO =0;
	mOtherFBO = 1;
	mFBOs[0] = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, format );
	mFBOs[1] = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, format );
	
    mShaderRefraction = gl::GlslProg::create( loadResource( RES_REFRACTION_VERT  ), loadResource( PASS_FRAG   ) );
	mShader = gl::GlslProg::create( loadResource( RES_PASS_THRU_VERT ), loadResource( RES_GSRD_FRAG ) );
    
    createVBOMesh();
    
    mFBOs[0].bindFramebuffer();
    gl::clear();
    
    mFBOs[0].unbindFramebuffer();
  
	resetFBOs();
    
    //create Texture
    mTexture = gl::Texture( loadImage( loadResource( RES_STARTER_IMAGE ) ) );
    mTexture.setWrap( GL_REPEAT, GL_REPEAT );
    mTexture.setMinFilter( GL_LINEAR );
    mTexture.setMagFilter( GL_LINEAR );
    
    mCam.setPerspective(60, getWindowAspectRatio(), 1, 10000);
    mCam.lookAt(Vec3f(0,mTilt,300),Vec3f(0,0,0),Vec3f::yAxis());

}

void ReactionDiffusionV1App::update(){
    mCam.lookAt(Vec3f(0,mTilt,700),Vec3f(0,0,0),Vec3f::yAxis());

    //FBO PING PONGING
    const int ITERATIONS = mSpeed;
		gl::setMatricesWindow( mFBOs[0].getSize(), false );
	gl::setViewport( mFBOs[0].getBounds() );
	for( int i = 0; i < ITERATIONS; i++ ) {
		mCurrentFBO = ( mCurrentFBO + 1 ) % 2;
		mOtherFBO   = ( mCurrentFBO + 1 ) % 2;
		
		mFBOs[ mCurrentFBO ].bindFramebuffer();
        if (textFeed == false){
        mTexture.bind(0);
            textFeed = true;
        }else{
		mFBOs[ mOtherFBO ].bindTexture();
        }
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        
		mShader->bind();
		mShader->uniform( "texture", 0 );
		mShader->uniform( "srcTexture", 1 );
		mShader->uniform( "width", (float)FBO_WIDTH );
		mShader->uniform( "ru", mReactionU );
		mShader->uniform( "rv", mReactionV );
		mShader->uniform( "k", mReactionK );
		mShader->uniform( "f", mReactionF );
        mShader->uniform( "f", mReactionF );
        //mShader->uniform( "scale", mScale );
        
		gl::drawSolidRect( mFBOs[ mCurrentFBO ].getBounds() );
        
		mShader->unbind();
        
        
        
		if( mMousePressed ){
			glColor4f( 1.0f, 1.0f, 1.0f, -.5f );
			RectMapping windowToFBO( getWindowBounds(), mFBOs[mCurrentFBO].getBounds() );
			gl::drawSolidCircle( windowToFBO.map( mMouse ), 5.0f, 32 );
		}
     
        mFBOs[ mCurrentFBO ].unbindFramebuffer();
       
        

	}
    
}

void ReactionDiffusionV1App::draw()
{
    
    gl::clear( ColorA( 0, 0, 0, 0 ) );
	gl::setMatrices( mCam );
	gl::setViewport( getWindowBounds() );
    

    gl::pushMatrices();
    gl::translate(Vec2f(-getWindowWidth()/2, -getWindowHeight()/2));
    mFBOs[mCurrentFBO].bindTexture(0);
    mShaderRefraction->bind();
    mShaderRefraction->uniform("displacementMap", 0);
    mShaderRefraction->uniform("scale", mScale);
    mShaderRefraction->uniform("size", mSize);

    gl::draw(mWrapper);
    mShaderRefraction->unbind();
    mFBOs[mCurrentFBO].unbindTexture();
    gl::popMatrices();
    

    mParams->draw();
   
    
}

void ReactionDiffusionV1App::resetFBOs()
{
   // mTexture.bind( 0 );
	gl::setMatricesWindow( mFBOs[0].getSize(), false );
	gl::setViewport( mFBOs[0].getBounds() );
	for( int i = 0; i < 2; i++ ){
		mFBOs[i].bindFramebuffer();
        gl::clear();
        textFeed = true;

	}
    gl::Fbo::unbindFramebuffer();

}

void ReactionDiffusionV1App::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r' ) {
		resetFBOs();
	}
}




    CINDER_APP_NATIVE( ReactionDiffusionV1App, RendererGl );
