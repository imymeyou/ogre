#define CLIENT_DESCRIPTION "Camera Control"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#include <Ogre.h>
#include <OIS/OIS.h>

const float MAX_CLIPPING_DISTANCE = 5000.0f;
const float MIN_CLIPPING_DISTANCE = 30.0f;
using namespace Ogre;

class InputController : public FrameListener,
	// Fill Here -----------------------------------------------
	public OIS::KeyListener, public OIS::MouseListener

	// ---------------------------------------------------------
{

public:
	InputController(Root* root, OIS::Keyboard *keyboard, OIS::Mouse *mouse) : mRoot(root), mKeyboard(keyboard), mMouse(mouse)
	{
		// Fill Here -----------------------------------------------
		mCamera = mRoot->getSceneManager("main")->getCamera("main");
		mCameraMoveVector = Ogre::Vector3::ZERO;
		mContinue = true;

		keyboard->setEventCallback(this);
		mouse->setEventCallback(this);
		// ---------------------------------------------------------

	}


	bool frameStarted(const FrameEvent &evt)
	{
		// Fill Here -----------------------------------------------
		mKeyboard->capture();
		mMouse->capture();

		// ---------------------------------------------------------
		return true;
	}

	// Key Linstener Interface Implementation

	bool keyPressed(const OIS::KeyEvent &evt)
	{
		// Fill Here -----------------------------------------------
		switch (evt.key)
		{
		case OIS::KC_W: mCameraMoveVector.y += 1; break;
		case OIS::KC_S: mCameraMoveVector.y -= 1; break;
		case OIS::KC_A: mCameraMoveVector.x -= 1; break;
		case OIS::KC_D: mCameraMoveVector.x += 1; break;
		case OIS::KC_ESCAPE: mContinue = false; break;
		}
		return true;
		// ---------------------------------------------------------
	}

	bool keyReleased(const OIS::KeyEvent &evt)
	{

		switch (evt.key)
		{
		case OIS::KC_W: mCameraMoveVector.y -= 1; break;
		case OIS::KC_S: mCameraMoveVector.y += 1; break;
		case OIS::KC_A: mCameraMoveVector.x += 1; break;
		case OIS::KC_D: mCameraMoveVector.x -= 1; break;
		case OIS::KC_ESCAPE: mContinue = false; break;
		}

		return true;
	}


	// Mouse Listener Interface Implementation

	bool mouseMoved(const OIS::MouseEvent &evt)
	{
		// Fill Here -----------------------------------------------
		if (evt.state.buttonDown(OIS::MB_Right))
		{
			mCamera->yaw(Degree(-evt.state.X.rel));
			mCamera->pitch(Degree(-evt.state.Y.rel));
		}

		mCamera->moveRelative(Ogre::Vector3(0, 0, -evt.state.Z.rel*0.1f));
		// ---------------------------------------------------------
		return true;
	}

	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
	{
		return true;
	}

	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
	{
		return true;
	}


private:
	bool mContinue;
	Ogre::Root* mRoot;
	OIS::Keyboard* mKeyboard;
	OIS::Mouse* mMouse;
	Camera* mCamera;

	Ogre::Vector3 mCameraMoveVector;
};



class ProfessorController : public FrameListener
{
	SceneNode *mProfessorNode;
	Ogre::Entity *mProfessorEntity;
	Ogre::AnimationState* mAnimationState;

	std::deque<Vector3> mWalkList;
	Real mWalkSpeed;
	Vector3 mDirection;
	Real mDistance;
	Vector3 mDestination;

public:
	ProfessorController(Root* root)
	{
		mProfessorNode = root->getSceneManager("main")->getSceneNode("Professor");
		mProfessorEntity = root->getSceneManager("main")->getEntity("Professor");

		mWalkSpeed = 80.0f;
		mDirection = Vector3::ZERO;
		mAnimationState = mProfessorEntity->getAnimationState("Walk");
		mAnimationState->setEnabled(true);
		mAnimationState->setLoop(true);
	}

	bool frameStarted(const FrameEvent &evt)
	{
	
		

		mAnimationState->addTime(evt.timeSinceLastFrame);

		return true;
	}



};



class LectureApp {

	Root* mRoot;
	RenderWindow* mWindow;
	SceneManager* mSceneMgr;
	Camera* mCamera;
	Viewport* mViewport;
	OIS::Keyboard* mKeyboard;
	OIS::Mouse* mMouse;

	OIS::InputManager *mInputManager;



public:

	LectureApp() {}

	~LectureApp() {}

	void go(void)
	{
		// OGRE의 메인 루트 오브젝트 생성
#if !defined(_DEBUG)
		mRoot = new Root("plugins.cfg", "ogre.cfg", "ogre.log");
#else
		mRoot = new Root("plugins_d.cfg", "ogre.cfg", "ogre.log");
#endif


		// 초기 시작의 컨피규레이션 설정 - ogre.cfg 이용
		if (!mRoot->restoreConfig()) {
			if (!mRoot->showConfigDialog()) return;
		}

		mWindow = mRoot->initialise(true, CLIENT_DESCRIPTION " : Copyleft by Dae-Hyun Lee");

		mSceneMgr = mRoot->createSceneManager(ST_GENERIC, "main");
		mCamera = mSceneMgr->createCamera("main");


		mCamera->setPosition(0.0f, 100.0f, 500.0f);
		mCamera->lookAt(0.0f, 100.0f, 0.0f);
		mCamera->setNearClipDistance(MIN_CLIPPING_DISTANCE);
		mCamera->setFarClipDistance(MAX_CLIPPING_DISTANCE);


		mViewport = mWindow->addViewport(mCamera);
		mViewport->setBackgroundColour(ColourValue(0.0f, 0.0f, 0.5f));
		mCamera->setAspectRatio(Real(mViewport->getActualWidth()) / Real(mViewport->getActualHeight()));


		ResourceGroupManager::getSingleton().addResourceLocation("resource.zip", "Zip");
		ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		mSceneMgr->setAmbientLight(ColourValue(1.0f, 1.0f, 1.0f));

		// 좌표축 표시
		Ogre::Entity* mAxesEntity = mSceneMgr->createEntity("Axes", "axes.mesh");
		mSceneMgr->getRootSceneNode()->createChildSceneNode("AxesNode", Ogre::Vector3(0, 0, 0))->attachObject(mAxesEntity);
		mSceneMgr->getSceneNode("AxesNode")->setScale(5, 5, 5);

		_drawGridPlane();


		Entity* entity1 = mSceneMgr->createEntity("Professor", "DustinBody.mesh");
		SceneNode* node1 = mSceneMgr->getRootSceneNode()->createChildSceneNode("Professor", Vector3(0.0f, 0.0f, 0.0f));
		node1->attachObject(entity1);


#if 0
		Entity* entity2 = mSceneMgr->createEntity("Ninja", "ninja.mesh");
		SceneNode* node2 = mSceneMgr->getRootSceneNode()->createChildSceneNode("Ninja", Vector3(0.0f, 0.0f, 0.0f));
		node2->attachObject(entity2);
		node2->setOrientation(Ogre::Quaternion(Ogre::Degree(180), Ogre::Vector3::UNIT_Y));
#endif

		size_t windowHnd = 0;
		std::ostringstream windowHndStr;
		OIS::ParamList pl;
		mWindow->getCustomAttribute("WINDOW", &windowHnd);
		windowHndStr << windowHnd;
		pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
		pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND")));
		pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
		pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
		pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
		mInputManager = OIS::InputManager::createInputSystem(pl);


		// Fill Here -----------------------------------------------
		mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
		mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));

		InputController* inputController = new InputController(mRoot, mKeyboard, mMouse);
		mRoot->addFrameListener(inputController);

		// ---------------------------------------------------------

		ProfessorController* professorController = new ProfessorController(mRoot);
		mRoot->addFrameListener(professorController);

		mRoot->startRendering();

		mInputManager->destroyInputObject(mKeyboard);
		mInputManager->destroyInputObject(mMouse);
		OIS::InputManager::destroyInputSystem(mInputManager);

		delete professorController;
		delete inputController;

		delete mRoot;
	}

private:
	void _drawGridPlane(void)
	{
		Ogre::ManualObject* gridPlane = mSceneMgr->createManualObject("GridPlane");
		Ogre::SceneNode* gridPlaneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("GridPlaneNode");

		Ogre::MaterialPtr gridPlaneMaterial = Ogre::MaterialManager::getSingleton().create("GridPlanMaterial", "General");
		gridPlaneMaterial->setReceiveShadows(false);
		gridPlaneMaterial->getTechnique(0)->setLightingEnabled(true);
		gridPlaneMaterial->getTechnique(0)->getPass(0)->setDiffuse(1, 1, 1, 0);
		gridPlaneMaterial->getTechnique(0)->getPass(0)->setAmbient(1, 1, 1);
		gridPlaneMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(1, 1, 1);

		gridPlane->begin("GridPlaneMaterial", Ogre::RenderOperation::OT_LINE_LIST);
		for (int i = 0; i<21; i++)
		{
			gridPlane->position(-500.0f, 0.0f, 500.0f - i * 50);
			gridPlane->position(500.0f, 0.0f, 500.0f - i * 50);

			gridPlane->position(-500.f + i * 50, 0.f, 500.0f);
			gridPlane->position(-500.f + i * 50, 0.f, -500.f);
		}

		gridPlane->end();

		gridPlaneNode->attachObject(gridPlane);
	}
};


#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
	int main(int argc, char *argv[])
#endif
	{
		LectureApp app;

		try {

			app.go();

		}
		catch (Ogre::Exception& e) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
			std::cerr << "An exception has occured: " <<
				e.getFullDescription().c_str() << std::endl;
#endif
		}

		return 0;
	}

#ifdef __cplusplus
}
#endif

