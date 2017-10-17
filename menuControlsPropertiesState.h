#pragma once

#include "stateManager/gameStates.h"
#include "utility/simpleTypes.h"

#include "config.h"

namespace Urho3D
{
	class Button;
	class UIElement;
}

class MenuControlsPropertiesState : public IGameState
{
	URHO3D_OBJECT(MenuControlsPropertiesState, IGameState);

public:

	MenuControlsPropertiesState(Urho3D::Context * context);
	virtual ~MenuControlsPropertiesState() = default;

	virtual void Create();
	virtual void Enter();
	virtual void Exit();
	virtual void Pause();
	virtual void Resume();

private:
	/// UI elements
	WeakPtr<UIElement>    window_;

	WeakPtr<Button>       returnToMenu_;
	WeakPtr<Button>       applyChanges_;

	WeakPtr<Button>       selectedButton_;

	// event related functions
	void SubscribeToEvents();

	void HandleBackButtonClick(StringHash eventType, VariantMap& eventData);
	void HandleApplyButtonClick(StringHash eventType, VariantMap& eventData);

	void HandleUpdate(StringHash eventType, VariantMap& eventData);

	void HandleButtonPressed(StringHash eventType, VariantMap& eventData);

	void HandleKeyDown(StringHash eventType, VariantMap& eventData);
	void HandleMouseKeyDown(StringHash eventType, VariantMap& eventData);

	void SetNewKey(Configuration::InputDeviceType device, U32 key);
};
