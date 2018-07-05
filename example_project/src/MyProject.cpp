/*******************************************************
*Description: Use w,a,s,d to move camera
*Press 'space' to move the animated sphere to a new random location
*******************************************************/
//cave stuff
//Include this before everything else !
#include <renderNode.h>
#include <synchObject.h>
#include <config.h>

#include <Corrade/Containers/Array.h>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Arguments.h>
#include <Magnum/Mesh.h>
#include <Magnum/PixelFormat.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Math/Matrix.h>
//#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/Trade/MeshData3D.h>
#include <Magnum/Trade/MeshObjectData3D.h>
#include <Magnum/Trade/PhongMaterialData.h>
#include <Magnum/Trade/SceneData.h>
#include <Magnum/Trade/TextureData.h>
//
#include <Magnum/SceneGraph/Animable.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/Timeline.h>
#include <Magnum/SceneGraph/AnimableGroup.h>
#include <Magnum/Timeline.h>

#include <Magnum/GL/Buffer.h>

#include <Magnum/MeshTools/Compile.h>

//other
#include <functional>
#include <iostream>
#include <thread>



using namespace Magnum;
using namespace Math::Literals;

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

Matrix4x4 glm2magnum(const glm::mat4& input) {
	
	Matrix4 tmp(Vector4(input[0][0],input[0][1],input[0][2],input[0][3]),
		Vector4(input[1][0], input[1][1],input[1][2], input[1][3]),
		Vector4( input[2][0], input[2][1], input[2][2], input[2][3]),
		Vector4( input[3][0], input[3][1], input[3][2], input[3][3]) );
	return tmp;
}


class ColoredSphere :public Object3D, public SceneGraph::Drawable3D{
public:
	explicit ColoredSphere(Object3D &parent, SceneGraph::DrawableGroup3D& group, Color4 color) 
		: Object3D{&parent},
		SceneGraph::Drawable3D{*this, &group } {
		std::tie(_mesh, _vertices, _indices) = MeshTools::compile(Primitives::uvSphereSolid(16, 32), BufferUsage::StaticDraw);
	
		_color = color;
	}
private:
	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override {
		_shader.setDiffuseColor(Color3::fromHSV(216.0_degf, 0.85f, 1.0f))
			.setLightPosition(camera.cameraMatrix().transformPoint({ 500.0f, 500.0f, 700.0f }))
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.rotation())
			.setProjectionMatrix(camera.projectionMatrix())
			.setDiffuseColor(_color);
		_mesh.draw(_shader);
	};
	Color4 _color;
	GL::Mesh _mesh;
	std::unique_ptr<GL::Buffer> _vertices, _indices;
	Shaders::Phong _shader;
};

class AnimatedSphere :public Object3D, public SceneGraph::Drawable3D,public SceneGraph::Animable3D {
public:
	explicit AnimatedSphere(Object3D &parent, SceneGraph::DrawableGroup3D& group, Color4 color,SceneGraph::AnimableGroup3D& group2)
		: Object3D{ &parent },
		SceneGraph::Animable3D{ *this, &group2 },
		SceneGraph::Drawable3D{ *this, &group } {
		std::tie(_mesh, _vertices, _indices) = MeshTools::compile(Primitives::uvSphereSolid(16, 32), GL::BufferUsage::StaticDraw);
		_color = color;
	}
private:
	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override {
		_shader.setDiffuseColor(Color3::fromHSV(216.0_degf, 0.85f, 1.0f))
			.setLightPosition(camera.cameraMatrix().transformPoint({ 500.0f, 500.0f, 700.0f }))
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.rotation())
			.setProjectionMatrix(camera.projectionMatrix())
			.setDiffuseColor(_color);
		_mesh.draw(_shader);
	};
	void animationStep(Float time, Float delta) override {

		_color = Color4((sin(time/5000. * 3.1415) +1.)/2., 0, 0, 0);
	}
	Color4 _color;
	GL::Mesh _mesh;
	std::unique_ptr<GL::Buffer> _vertices, _indices;
	Shaders::Phong _shader;
	float counterR = 0;
};


class MyProject: public Platform::GlfwApplication{

    public: 
        explicit MyProject(const Arguments& arguments);
		~MyProject();
		 
    private:  
        void drawEvent() override;
        void viewportEvent(const Vector2i& size) override;
		void keyPressEvent(KeyEvent& event)override;
		void keyReleaseEvent(KeyEvent& event)override;
		//
		void moveCamera();
		
		void addDebugSpheres();
		void changeLocationOfAnimatedSphere();

        Shaders::Phong _coloredShader,
            _texturedShader{Shaders::Phong::Flag::DiffuseTexture};
      

        Scene3D _scene;
		Object3D _cameraObject;
        std::shared_ptr<SceneGraph::Camera3D> _camera;
        SceneGraph::DrawableGroup3D _drawables;
		Vector2i _previousPosition;
		bool wPressed=false, aPressed=false, sPressed=false, dPressed=false;
		SceneGraph::AnimableGroup3D _animables; 
		//Timeline _timeline;
		AnimatedSphere* _animatedSphere;


		// synchlib stuff
		std::shared_ptr<synchlib::renderNode> rNode;
		std::shared_ptr<synchlib::udpSynchObject<long long> > timeSyncher;
		std::shared_ptr< synchlib::udpSynchObject<glm::bvec4 > > keyboardSyncher;
		synchlib::caveConfig m_conf;
		long long m_lastTime = 0;

};



MyProject::~MyProject() {
	rNode->stopSynching();
}

#define CAVE_BUILD
MyProject::MyProject(const Arguments& arguments) : 
	Platform::Application{ arguments, Configuration{}
	.setTitle("Magnum MyProject")
#ifdef CAVE_BUILD
	//.setWindowFlags(Configuration::WindowFlag::Fullscreen)
	.setSize(Vector2i(1920,1200))
	.setFlags(GLConfiguration::Flag::Stereo)
	.setCursorMode(Configuration::CursorMode::Hidden)
#else
	.setWindowFlags(Configuration::WindowFlag::Resizable)
#endif
}
{
	if (arguments.argc < 3) {
		std::cout << "Not enough input arguments. Exiting now" << std::endl; 
		this->exit();
		return;
	}

	rNode = std::make_shared<synchlib::renderNode>(arguments.argv[2], arguments.argc, arguments.argv);

	timeSyncher = synchlib::udpSynchObject<long long>::create();
	keyboardSyncher = synchlib::udpSynchObject<glm::bvec4 >::create();
	


	rNode->addSynchObject(timeSyncher, synchlib::renderNode::RECEIVER,0);
	rNode->addSynchObject(keyboardSyncher, synchlib::renderNode::RECEIVER, 0);
	rNode->init();
	rNode->startSynching();
	// synchlib stuff end

    /* Every scene needs a camera */
    _cameraObject
        .setParent(&_scene)
        .translate(Vector3::zAxis(15.0f));
	(*(_camera = std::make_shared<SceneGraph::Camera3D>( _cameraObject )))
//        .setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
//        .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.01f, 100000.0f))
        .setViewport(GL::defaultFramebuffer.viewport().size());

    /* Setup renderer and shader defaults */
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
    _coloredShader
        .setAmbientColor(0x111111_rgbf)
        .setSpecularColor(0xffffff_rgbf)
        .setShininess(80.0f);
    _texturedShader
        .setAmbientColor(0x111111_rgbf)
        .setSpecularColor(0x111111_rgbf)
        .setShininess(80.0f);


    GL::Renderer::setClearColor(Color4(0.8,0.8,0.8,1.f));

	addDebugSpheres();
	//add the animated sphere at origin
	_animatedSphere = new AnimatedSphere(_scene, _drawables, Color4::blue(1, 1), _animables);
	_animatedSphere->scale(Vector3(10, 10, 10));
	_animatedSphere->setState(SceneGraph::AnimationState::Running);


	//_timeline.start();
}

void MyProject::addDebugSpheres()
{
	for (int i = 1; i < 100; i++) {
		(new ColoredSphere(_scene, _drawables, Color4::red(1, 1)))->translate(
			Vector3(10.0 * i, 0, 0)).scaleLocal(Vector3(5., 5., 5.));
	}
	for (int i = 1; i < 100; i++) {
		(new ColoredSphere(_scene, _drawables, Color4::green(1, 1)))->translate(
			Vector3(0, 10.0 * i, 0)).scaleLocal(Vector3(5., 5., 5.));
	}
	for (int i = 1; i < 100; i++) {
		(new ColoredSphere(_scene, _drawables, Color4::blue(1, 1)))->translate(
			Vector3(0, 0, 10.0 * i)).scaleLocal(Vector3(5., 5., 5.));
	}
}

void MyProject::changeLocationOfAnimatedSphere()
{
	Debug{} << "Moving animated sphere";

	//select a new (random) position for the sphere
	Vector3 newPosition(std::fmod(std::rand() - (RAND_MAX / 2.0f), 7),
		std::fmod(std::rand() - (RAND_MAX / 2.0f), 7),std::fmod(std::rand() - (RAND_MAX / 2.0f), 7));

	_animatedSphere->resetTransformation();
	_animatedSphere->translate(newPosition);
}


int nFrames = 0;

void MyProject::drawEvent() {

	if (keyboardSyncher->hasChanged()) {
		glm::bvec4 tmp = keyboardSyncher->getData();
		wPressed = tmp[0];
		aPressed = tmp[1];
		sPressed = tmp[2];
		dPressed = tmp[3];
	}

	moveCamera();
	glm::mat4 projMatL, projMatR, viewMat;
	rNode->getProjectionMatrices(projMatL, projMatR);
	rNode->getSceneTrafo(viewMat);
	Matrix4 mMatL = glm2magnum(projMatL);
	Matrix4 mMatR = glm2magnum(projMatR);
	Matrix4 mview = glm2magnum(viewMat);

	long long curTime = timeSyncher->getData();
	_animables.step(curTime, curTime - m_lastTime);
	m_lastTime = curTime;



		GL::defaultFramebuffer.mapForDraw(GL::DefaultFramebuffer::DrawAttachment::BackLeft);
		GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);
		_camera->setProjectionMatrix(mMatL*mview);
		_camera->draw(_drawables);

#ifdef CAVE_BUILD
		GL::defaultFramebuffer.mapForDraw(GL::DefaultFramebuffer::DrawAttachment::BackRight);
		GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);
		_camera->setProjectionMatrix(mMatR*mview);
		_camera->draw(_drawables);
#endif

	glFinish();
	if (!rNode->synchFrame()) {
		std::cout << "Could not sync frame" << std::endl;
		this->exit();
	}
    swapBuffers();

	redraw();
}

void MyProject::viewportEvent(const Vector2i& size) {
    GL::defaultFramebuffer.setViewport({{}, size});
    _camera->setViewport(size);
}

void MyProject::keyPressEvent(KeyEvent & event)
{
	switch (event.key()) {
		
	case KeyEvent::Key::W :wPressed = true; break;
	case KeyEvent::Key::A :aPressed = true; break;
	case KeyEvent::Key::S :sPressed = true; break;
	case KeyEvent::Key::D :dPressed = true; break;
	default:break;
	}
}

void MyProject::keyReleaseEvent(KeyEvent & event)
{
	switch (event.key()) {
		case KeyEvent::Key::W : wPressed = false; break;
		case KeyEvent::Key::A : aPressed = false; break;
		case KeyEvent::Key::S : sPressed = false; break;
		case KeyEvent::Key::D : dPressed = false; break;
		case KeyEvent::Key::Space :Debug{}<<"Space"; changeLocationOfAnimatedSphere();break;
		default:Debug{} << "Def"; break;
	}

}

void MyProject::moveCamera()
{
	if (wPressed) {
		_cameraObject.translate(Vector3::zAxis(
			-1.));
	}
	if (aPressed) {
		_cameraObject.translate(Vector3::xAxis(
			-1.));
	}
	if (sPressed) {
		_cameraObject.translate(Vector3::zAxis(
			1.));
	}
	if (dPressed) {
		_cameraObject.translate(Vector3::xAxis(
			1.));
	}
}


MAGNUM_APPLICATION_MAIN(MyProject)
