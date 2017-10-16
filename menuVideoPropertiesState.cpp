#include <Urho3D/UI/Window.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/Localization.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/DropDownList.h>
#include <Urho3D/UI/ListView.h>

#include "stateManager/statesList.h"
#include "stateManager/gameStateEvents.h"
#include "utility/sharedData.h"
#include "config.h"

#include "mainMenu/menuVideoPropertiesState.h"

using namespace Urho3D;

String MenuVideoPropertiesState::FullscreenToString(FullscreenMode mode) const
{
	switch (mode)
	{
		case FullscreenMode::Windowed:
			return "Windowed";
		case FullscreenMode::Fullscreen:
			return "Fullscreen";
		case FullscreenMode::Borderless:
			return "Borderless";
	}

	return "";
}

MenuVideoPropertiesState::MenuVideoPropertiesState(Urho3D::Context * context) :
	IGameState(context),
	resolution_(0),
	fullscreen_(FullscreenMode::Windowed),
	languageIndex_(0)
{
	// TODO: take into account multiple monitors
	Graphics* graphics = GetSubsystem<Graphics>();
	PODVector<IntVector3> supportedResolutions = graphics->GetResolutions(0);
	for (U32 i = 0; i < supportedResolutions.Size(); i++)
	{
		resolutions_.Push(Resolution(supportedResolutions[i].x_, supportedResolutions[i].y_, supportedResolutions[i].z_));
	}
}

void MenuVideoPropertiesState::Create()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
	XMLFile* layout = cache->GetResource<XMLFile>("UI/menuProperties/menuVideoProperties.xml");
	uiStateRoot_->LoadXML(layout->GetRoot(), style);

	window_ =           static_cast<Window*>(uiStateRoot_->GetChild("window_", true));
	resolutionList_ =   static_cast<DropDownList*>(uiStateRoot_->GetChild("resolutionList_", true));
	fullScreenList_ =   static_cast<DropDownList*>(uiStateRoot_->GetChild("fullScreenList_", true));
	languageList_ =     static_cast<DropDownList*>(uiStateRoot_->GetChild("languageList_", true));
	returnToMenu_ =     static_cast<Button*>(uiStateRoot_->GetChild("returnToMenu_", true));
	applyChanges_ =     static_cast<Button*>(uiStateRoot_->GetChild("applyChanges_", true));
}

void MenuVideoPropertiesState::Enter()
{
	Graphics* graphics = GetSubsystem<Graphics>();
	for (U32 i = 0; i < resolutions_.Size(); i++)
	{
		Text* resolutionItem = new Text(context_);
		resolutionItem->SetText(resolutions_[i].ToString());
		resolutionItem->SetStyleAuto();

		resolutionList_->AddItem(resolutionItem);

		if (graphics->GetWidth() == resolutions_[i].width && graphics->GetHeight() == resolutions_[i].height)
		{
			resolution_ = i;
		}
	}

	resolutionList_->GetListView()->SetSelection(resolution_);

	fullscreen_ = FullscreenMode::Windowed;
	if (graphics->GetFullscreen())
	{
		fullscreen_ = graphics->GetBorderless() ? FullscreenMode::Borderless : fullscreen_ = FullscreenMode::Fullscreen;
	}

	for (U32 i = 0; i < static_cast<unsigned>(FullscreenMode::Count); i++)
	{
		FullscreenMode mode = static_cast<FullscreenMode>(i);
		Text* fullscreenItem = new Text(context_);
		fullscreenItem->SetText(FullscreenToString(mode));
		fullscreenItem->SetStyleAuto();

		fullScreenList_->AddItem(fullscreenItem);
	}

	fullScreenList_->GetListView()->SetSelection(static_cast<U32>(fullscreen_));

	Localization* l10n = GetSubsystem<Localization>();
	for (S32 i = 0; i < l10n->GetNumLanguages(); i++)
	{
		Text* languageItem = new Text(context_);
		languageItem->SetText(l10n->GetLanguage(i));
		languageItem->SetStyleAuto();

		languageList_->AddItem(languageItem);
	}

	languageIndex_ = l10n->GetLanguageIndex();
	languageList_->GetListView()->SetSelection(languageIndex_);

	uiStateRoot_->SetVisible(true);
	uiStateRoot_->UpdateLayout();

	SubscribeToEvents();
}

void MenuVideoPropertiesState::SubscribeToEvents()
{
	SubscribeToEvent(resolutionList_, E_ITEMSELECTED, URHO3D_HANDLER(MenuVideoPropertiesState, HandleSelectResolution));
	SubscribeToEvent(fullScreenList_, E_ITEMSELECTED, URHO3D_HANDLER(MenuVideoPropertiesState, HandleSelectFullscreen));
	SubscribeToEvent(languageList_, E_ITEMSELECTED, URHO3D_HANDLER(MenuVideoPropertiesState, HandleSelectLanguage));
	SubscribeToEvent(returnToMenu_, E_PRESSED, URHO3D_HANDLER(MenuVideoPropertiesState, HandleBackButtonClick));
	SubscribeToEvent(applyChanges_, E_PRESSED, URHO3D_HANDLER(MenuVideoPropertiesState, HandleApplyButtonClick));
}

void MenuVideoPropertiesState::HandleSelectFullscreen(StringHash eventType, VariantMap & eventData)
{
	fullscreen_ = static_cast<FullscreenMode>(eventData[ItemSelected::P_SELECTION].GetInt());
}

void MenuVideoPropertiesState::HandleSelectResolution(StringHash eventType, VariantMap & eventData)
{
	resolution_ = eventData[ItemSelected::P_SELECTION].GetInt();
}

void MenuVideoPropertiesState::HandleSelectLanguage(StringHash eventType, VariantMap & eventData)
{
	languageIndex_ = eventData[ItemSelected::P_SELECTION].GetInt();
}

void MenuVideoPropertiesState::HandleBackButtonClick(StringHash eventType, VariantMap & eventData)
{
	bool isFromGame = GetSubsystem<SharedData>()->inGame_;

	GameStates::GameState targetState = isFromGame ?
		GameStates::TSPACE :
		GameStates::MENU_PROPERTIES;

	SendEvent(G_STATE_CHANGE,
		GameChangeStateEvent::P_STATE, targetState);
}

void MenuVideoPropertiesState::HandleApplyButtonClick(StringHash eventType, VariantMap & eventData)
{
	Graphics* graphics = GetSubsystem<Graphics>();
	Configuration* config = GetSubsystem<Configuration>();

	Resolution res = resolutions_[resolution_];
	bool fullscreen = (fullscreen_ != FullscreenMode::Windowed);
	bool borderless = (fullscreen_ == FullscreenMode::Borderless);

	graphics->SetMode(res.width, res.height, fullscreen, borderless, false, false, false, false, 
		0, 0, res.refreshRate);

	config->SetValue("width", graphics->GetWidth());
	config->SetValue("height", graphics->GetHeight());
	config->SetValue("fullscreen", graphics->GetFullscreen());
	config->SetValue("borderless", graphics->GetBorderless());

	Localization* l10n = GetSubsystem<Localization>();
	if (languageIndex_ != l10n->GetLanguageIndex())
	{
		l10n->SetLanguage(languageIndex_);
		config->SetValue("lang", l10n->GetLanguage());
	}

	config->Save();

	uiStateRoot_->UpdateLayout();
}

void MenuVideoPropertiesState::Exit()
{
	uiStateRoot_->SetVisible(false);

	UnsubscribeFromAllEvents();
}

void MenuVideoPropertiesState::Pause()
{
}

void MenuVideoPropertiesState::Resume()
{
}
