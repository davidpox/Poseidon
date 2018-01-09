#pragma once

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/Slider.h>
#include <Urho3D/UI/ToolTip.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>


namespace Urho3D
{
	class Node;
	class Scene;
	class Window;
	class ResourceCache;
}
// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;



class menu {
public: 


	menu();
	~menu();
	 
	void Initialise(ResourceCache* pRc, Scene* pSc);
	void Update(float delta);

	Button* CreateButton(const String& text, int pHeight, Urho3D::Window* whichWindow, Font* font);
	LineEdit* CreateLineEdit(const String& text, int pHeight, Urho3D::Window* whichWindow, Font* font);
	CheckBox* CreateCheckBox(int pHeight, Urho3D::Window* whichWindow);
	Text* CreateText(const String& text, int pHeight, Urho3D::Window* whichWindow, Font* font, int fontSize);
	Slider* CreateSlider(float RangeMax, int width, int height, Urho3D::Window* whichWindow);
	ToolTip* CreateToolTip(const String& text, Font* font, int fontSize, Urho3D::Window* whichWindow);


};