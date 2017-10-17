#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/DropDownList.h>

#include "stateManager/statesList.h"
#include "stateManager/gameStateEvents.h"
#include "utility/sharedData.h"

#include "mainMenu/menuControlsPropertiesState.h"

using namespace Urho3D;

MenuControlsPropertiesState::MenuControlsPropertiesState(Urho3D::Context * context)
	: IGameState(context)
{
}

void MenuControlsPropertiesState::Create()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
	XMLFile* layout = cache->GetResource<XMLFile>("UI/menuProperties/menuControlsProperties.xml");
	uiStateRoot_->LoadXML(layout->GetRoot(), style);

	window_ =           static_cast<Window*>(uiStateRoot_->GetChild("window_", true));
	UIElement* actionsBar = uiStateRoot_->GetChild("actionsBar_", true);

	Configuration* config = GetSubsystem<Configuration>();
	if (!config)
		return;

	for (Configuration::GameInputActions action = Configuration::GameInputActions::MoveForward;
		action != Configuration::GameInputActions::Count; action = (Configuration::GameInputActions)((U32)action + 1))
	{
		UIElement* actionUIElement = actionsBar->CreateChild<UIElement>();

		XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
		XMLFile* layout = cache->GetResource<XMLFile>("UI/parts/configControl.xml");
		actionUIElement->LoadXML(layout->GetRoot(), style);

		Text* actionText = static_cast<Text*>(actionUIElement->GetChild("actionName_", true));
		Button* primaryKeyButton = static_cast<Button*>(actionUIElement->GetChild("primaryKey_", true));
		primaryKeyButton->SetVar("action", static_cast<U32>(action));
		primaryKeyButton->SetVar("unitNumber", 0);
		SubscribeToEvent(primaryKeyButton, E_PRESSED, URHO3D_HANDLER(MenuControlsPropertiesState, HandleButtonPressed));

		Text* primaryKeyText = static_cast<Text*>(actionUIElement->GetChild("primaryKeyName_", true));

		Button* secondaryKeyButton = static_cast<Button*>(actionUIElement->GetChild("secondaryKey_", true));
		secondaryKeyButton->SetVar("action", static_cast<U32>(action));
		secondaryKeyButton->SetVar("unitNumber", 1);
		SubscribeToEvent(secondaryKeyButton, E_PRESSED, URHO3D_HANDLER(MenuControlsPropertiesState, HandleButtonPressed));

		Text* secondaryKeyText = static_cast<Text*>(actionUIElement->GetChild("secondaryKeyName_", true));

		actionText->SetText(Configuration::StringFromEnumActions(action));
		primaryKeyText->SetText(config->GetActionKeyName(action, 0));
		secondaryKeyText->SetText(config->GetActionKeyName(action, 1));
	}

	returnToMenu_ =     static_cast<Button*>(uiStateRoot_->GetChild("returnToMenu_", true));
	applyChanges_ =     static_cast<Button*>(uiStateRoot_->GetChild("applyChanges_", true));
}

void MenuControlsPropertiesState::Enter()
{
	uiStateRoot_->SetVisible(true);
	uiStateRoot_->UpdateLayout();

	SubscribeToEvents();
}

void MenuControlsPropertiesState::SubscribeToEvents()
{
	SubscribeToEvent(returnToMenu_, E_PRESSED, URHO3D_HANDLER(MenuControlsPropertiesState, HandleBackButtonClick));
	SubscribeToEvent(applyChanges_, E_PRESSED, URHO3D_HANDLER(MenuControlsPropertiesState, HandleApplyButtonClick));

	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MenuControlsPropertiesState, HandleUpdate));
}

void MenuControlsPropertiesState::HandleBackButtonClick(StringHash eventType, VariantMap & eventData)
{
	bool isFromGame = GetSubsystem<SharedData>()->inGame_;

	GameStates::GameState targetState = isFromGame ?
		GameStates::TSPACE :
		GameStates::MENU_PROPERTIES;

	SendEvent(G_STATE_CHANGE,
		GameChangeStateEvent::P_STATE, targetState);
}

void MenuControlsPropertiesState::HandleApplyButtonClick(StringHash eventType, VariantMap & eventData)
{
	Configuration* config = GetSubsystem<Configuration>();
	config->Save();

	uiStateRoot_->UpdateLayout();
}

void MenuControlsPropertiesState::HandleUpdate(StringHash eventType, VariantMap & eventData)
{
	if (selectedButton_)
	{
		SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(MenuControlsPropertiesState, HandleKeyDown));
		SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(MenuControlsPropertiesState, HandleMouseKeyDown));
	}
}

void MenuControlsPropertiesState::HandleButtonPressed(StringHash eventType, VariantMap & eventData)
{
	if (selectedButton_)  // already wait for control press
		return;

	UIElement* element = static_cast<UIElement*>(eventData[Pressed::P_ELEMENT].GetPtr());

	Button* button = dynamic_cast<Button*>(element);
	if (!button)
		return;

	selectedButton_ = button;

	Text* buttonText = static_cast<Text*>(button->GetChild(0));
	buttonText->SetText("?");
}

void MenuControlsPropertiesState::HandleKeyDown(StringHash eventType, VariantMap & eventData)
{
	U32 key = eventData[KeyDown::P_KEY].GetInt();
	SetNewKey(Configuration::InputDeviceType::Keyboard, key);
}

void MenuControlsPropertiesState::HandleMouseKeyDown(StringHash eventType, VariantMap & eventData)
{
	U32 mouseKey = eventData[MouseButtonDown::P_BUTTON].GetInt();
	SetNewKey(Configuration::InputDeviceType::Mouse, mouseKey);
}

void MenuControlsPropertiesState::SetNewKey(Configuration::InputDeviceType device, U32 key)
{
	if (selectedButton_)
	{
		U32 actionNumber = selectedButton_->GetVar("action").GetUInt();
		U32 unitNumber = selectedButton_->GetVar("unitNumber").GetUInt();

		Configuration::GameInputActions action = static_cast<Configuration::GameInputActions>(actionNumber);

		Configuration* config = GetSubsystem<Configuration>();
		config->SetActionKey(action, device, key, unitNumber);

		Text* buttonText = static_cast<Text*>(selectedButton_->GetChild(0));
		buttonText->SetText(config->GetActionKeyName(action, unitNumber));
	}

	selectedButton_ = nullptr;
	UnsubscribeFromEvent(E_KEYDOWN);
	UnsubscribeFromEvent(E_MOUSEBUTTONDOWN);
}

void MenuControlsPropertiesState::Exit()
{
	uiStateRoot_->SetVisible(false);

	UnsubscribeFromAllEvents();
}

void MenuControlsPropertiesState::Pause()
{
}

void MenuControlsPropertiesState::Resume()
{
}
