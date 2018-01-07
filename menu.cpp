#include "menu.h"

menu::menu() {

}

menu::~menu() {

}

void menu::Initialise(ResourceCache* pRc, Scene* pSc) {

}

void menu::Update(float delta) {

}

Button* menu::CreateButton(const String& text, int pHeight, Urho3D::Window* whichWindow, Font* font) {
	Button* button = whichWindow->CreateChild<Button>();
	button->SetMinHeight(pHeight);
	button->SetStyleAuto();
	Text* buttonText = button->CreateChild<Text>();
	buttonText->SetFont(font, 12);
	buttonText->SetAlignment(HA_CENTER, VA_CENTER);
	buttonText->SetText(text);
	whichWindow->AddChild(button);
	return button;
}

LineEdit* menu::CreateLineEdit(const String& text, int pHeight, Urho3D::Window* whichWindow, Font* font) {
	LineEdit* lineEdit = whichWindow->CreateChild<LineEdit>();
	lineEdit->SetMinHeight(pHeight);
	lineEdit->SetAlignment(HA_CENTER, VA_CENTER);
	lineEdit->SetText(text);
	lineEdit->SetStyleAuto();

	whichWindow->AddChild(lineEdit);
	return lineEdit;
}

CheckBox* menu::CreateCheckBox(int pHeight, Urho3D::Window* whichWindow) {
	CheckBox* cb = whichWindow->CreateChild<CheckBox>();
	cb->SetMinHeight(pHeight);
	cb->SetStyleAuto();
	cb->SetAlignment(HA_CENTER, VA_CENTER);

	whichWindow->AddChild(cb);
	return cb;
}

Text* menu::CreateText(const String& text, int pHeight, Urho3D::Window* whichWindow, Font* font, int fontSize) {
	Text* txt = whichWindow->CreateChild<Text>();
	txt->SetMinHeight(16);
	txt->SetText(text);
	txt->SetFont(font);
	txt->SetFontSize(fontSize);
	txt->SetStyleAuto();
	txt->SetAlignment(HA_CENTER, VA_CENTER);

	whichWindow->AddChild(txt);
	return txt;
}

Slider* menu::CreateSlider(float RangeMax, int pHeight, Urho3D::Window* whichWindow) {
	Slider* slider = whichWindow->CreateChild<Slider>();
	slider->SetRange(RangeMax);
	slider->SetStyleAuto();
	slider->SetSize(150, 16);
	whichWindow->AddChild(slider);
	return slider;
}

ToolTip* menu::CreateToolTip(const String& text, Font* font, int fontSize, Urho3D::Window* whichWindow) {
	ToolTip* tp = whichWindow->CreateChild<ToolTip>();
	Text* tooltip_text = new Text(whichWindow->GetContext());
	tooltip_text->SetFont(font);
	tooltip_text->SetFontSize(fontSize);
	tooltip_text->SetText(text);
	tp->SetStyleAuto();

	tp->AddChild(tooltip_text);
	return tp;
}