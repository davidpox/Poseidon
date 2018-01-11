//
// Copyright (c) 2008-2016 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "Sample.h"
#include "boidsSet.h"
#include "missle.h"
#include "menu.h"
#include <iostream>
#include <vector>

namespace Urho3D
{

class Node;
class Scene;
class Window;

}

class Character;
class Touch;

/// Moving character example.
/// This sample demonstrates:
///     - Controlling a humanoid character through physics
///     - Driving animations using the AnimationController component
///     - Manual control of a bone scene node
///     - Implementing 1st and 3rd person cameras, using raycasts to avoid the 3rd person camera clipping into scenery
///     - Defining attributes of a custom component so that it can be saved and loaded
///     - Using touch inputs/gyroscope for iOS/Android (implemented through an external file)
class CharacterDemo : public Sample
{
    URHO3D_OBJECT(CharacterDemo, Sample);

	enum gamestate {
		NONE,
		CLIENT,
		SERVER,
		SINGLEPLAYER,
		PAUSED,
		WON,
		LOST,
		ENDED
	};

public:
    /// Construct.
    CharacterDemo(Context* context);
    /// Destruct.
    ~CharacterDemo();

    /// Setup after engine initialization and before running the main loop.
    virtual void Start();

private:


    /// Create static scene content.
    void CreateServerScene();
	void CreateClientScene();
    /// Create controllable character.
    Node* CreateCharacter();
    /// Subscribe to necessary events.
    void SubscribeToEvents();
    /// Handle application update. Set controls to character.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle application post-update. Update camera position after character has moved.
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);

	void HandleCollision(StringHash eventType, VariantMap& eventData);

	void spawnMissle();
	void CreateMainMenu();
	void CreateEnvironemnt();

	//Button* CreateButton(const String& text, int pHeight, Urho3D::Window* whichWindow, Font* font);
	//LineEdit* CreateLineEdit(const String& text, int pHeight, Urho3D::Window* whichWindow, Font* font);


	/* Menu callbacks */
	void HandleQuit(StringHash eventType, VariantMap& eventData);
	void StartSingleplayer(StringHash eventType, VariantMap& eventData);
	void handleConnect(StringHash eventType, VariantMap& eventData);
	void handleCreateServer(StringHash eventType, VariantMap& eventData);
	void handleDisconnect(StringHash eventType, VariantMap& eventData);

	/* Server Callbacks */
	void handleClientConnected(StringHash eventType, VariantMap& eventData);
	void handleClientDisconnected(StringHash eventType, VariantMap& eventData);
	void handlePhysicsPreStep(StringHash eventType, VariantMap& eventData);
	void handleCustomEvent(StringHash eventType, VariantMap& eventData);
	void handleConnectedToServer(StringHash eventType, VariantMap& eventData);
	void handleClientSceneLoaded(StringHash eventType, VariantMap& eventData);
	unsigned clientObjectID_ = 0;
	HashMap<Connection*, WeakPtr<Node>> serverObjects_;
	void handleServerToClientObjectID(StringHash eventType, VariantMap& eventData);
	Controls FromClientToServerControls();

	void CreateUI();
	void CreateEndScreen();

    /// Touch utility object.
    SharedPtr<Touch> touch_;
	SharedPtr<Window> window_;
	SharedPtr<UIElement> uiRoot_;
    /// The controllable character component.
    WeakPtr<Character> character_;
    /// First person camera flag.
    bool firstPerson_;

	// General
	boidsSet bS;
	boidsSet sS;
	missle mis;
	ResourceCache* cache;
	menu* menu_;
	//Light* l_flashlight;
	//Node* n_sub; // player node
	Terrain* t_terrain;
	bool menuVisible = true;
	bool drawDebug_;
	float MOVE_SPEED = 20.0f;
	//int playerHealth = 100;
	String causeofdeath;

	unsigned playerNodeID = -1;

	// MISSILES
	const int MAX_MISSLES;
	int missileCount = 0;
	int missileLeftOrRight = 0;

	// Server
	gamestate gs;
	gamestate t = NONE;
	LineEdit* leIPAddress;
	static const unsigned short SERVER_PORT = 2345;

	// COUNTERS
	float warningTextCounter = 0.0f;
	int fishCaught = 0;
	int fishKilled = 0;
	Text* timerText;
	float countdowntimer = 300.0f;
	float fpsUpdateCounter = 1.0f;

	//DEBUG
	void UpdateFPS(StringHash eventType, VariantMap& eventData);

};
