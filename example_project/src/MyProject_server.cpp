/*******************************************************
*Description: Use w,a,s,d to move camera
*Press 'space' to move the animated sphere to a new random location
*******************************************************/
//cave stuff
//Include this before everything else !
#include <renderServer.h>
#include <synchObject.h>
#include <config.h>

#include <Corrade/Utility/Arguments.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
//#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/Math/Color.h>

//other
#include <functional>
#include <iostream>
#include <thread>
#include <chrono>


using namespace Magnum;
using namespace Math::Literals;

class MyProject: public Platform::GlfwApplication{
    public:
        explicit MyProject(const Arguments& arguments);
		~MyProject() {
			if (rServer.get()) {
				rServer->stopSynching();
			}
		}
    private: 
        void drawEvent() override;
		void keyPressEvent(KeyEvent& event)override;
		void keyReleaseEvent(KeyEvent& event)override;
		//

		bool wPressed = false, aPressed = false, sPressed = false, dPressed = false;


		std::shared_ptr<synchlib::renderServer> rServer;
		std::shared_ptr<synchlib::udpSynchObject<long long> > timeSyncher;
		std::shared_ptr< synchlib::udpSynchObject<glm::bvec4 > > keyboardSyncher;

		std::chrono::high_resolution_clock::time_point time_start;

};


MyProject::MyProject(const Arguments& arguments) :
	Platform::Application{ arguments, Configuration{}
		.setTitle("Magnum Server Example")
	.setWindowFlags(Configuration::WindowFlag::Resizable)
}
{
	rServer = std::make_shared<synchlib::renderServer>(arguments.argv[1], arguments.argc, arguments.argv);

	timeSyncher = synchlib::udpSynchObject<long long>::create();
	keyboardSyncher = synchlib::udpSynchObject<glm::bvec4 >::create();


	rServer->addSynchObject(timeSyncher, synchlib::renderServer::SENDER, 0,0);
	rServer->addSynchObject(keyboardSyncher, synchlib::renderServer::SENDER, 0,0);
	rServer->init();
	rServer->startSynching();

}

void MyProject::drawEvent() {
	GL::Renderer::setClearColor(Color4(0, 1, 0, 1.f));
	GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

	auto timeNow = std::chrono::high_resolution_clock::now();
	long long dur = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - time_start).count();
	timeSyncher->setData(dur);
	timeSyncher->send();

    swapBuffers();
	redraw();
}


void MyProject::keyPressEvent(KeyEvent & event)
{
	Debug{} << "Key press";
	glm::bvec4 tmp = keyboardSyncher->getData();
	switch (event.key()) {
	case KeyEvent::Key::W :tmp[0] = true; break;
	case KeyEvent::Key::A :tmp[1] = true; break;
	case KeyEvent::Key::S :tmp[2] = true; break;
	case KeyEvent::Key::D :tmp[3] = true; break;
	default:break;
	}
	keyboardSyncher->setData(tmp);
	keyboardSyncher->send(); 
}

void MyProject::keyReleaseEvent(KeyEvent & event)
{
	Debug{} << "Key release ";
	glm::bvec4 tmp = keyboardSyncher->getData();
	switch (event.key()) {
		case KeyEvent::Key::W : tmp[0] = false; break;
		case KeyEvent::Key::A : tmp[1] = false; break;
		case KeyEvent::Key::S : tmp[2] = false; break;
		case KeyEvent::Key::D : tmp[3] = false; break;
		default:Debug{} << "Def"; break;
	}
	keyboardSyncher->setData(tmp);
	keyboardSyncher->send();

}


MAGNUM_APPLICATION_MAIN(MyProject)
