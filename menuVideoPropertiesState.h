#pragma once

#include "stateManager/gameStates.h"
#include "utility/simpleTypes.h"

namespace Urho3D
{
	class DropDownList;
	class Button;
	class UIElement;
}

class MenuVideoPropertiesState : public IGameState
{
	URHO3D_OBJECT(MenuVideoPropertiesState, IGameState);

public:

	MenuVideoPropertiesState(Urho3D::Context * context);
	virtual ~MenuVideoPropertiesState() = default;

	virtual void Create();
	virtual void Enter();
	virtual void Exit();
	virtual void Pause();
	virtual void Resume();

private:
	// options list
	S32 resolution_;
	S32 languageIndex_;
	enum class FullscreenMode
	{
		Windowed = 0,
		Fullscreen = 1,
		Borderless = 2,
		Count
	};

	String FullscreenToString(FullscreenMode mode) const;

	struct Resolution
	{
		S32 width;
		S32 height;
		S32 refreshRate;

		Resolution() : width(0), height(0), refreshRate(0) { }
		Resolution(S32 width_, S32 height_, S32 refreshRate_) : width(width_), height(height_), refreshRate(refreshRate_) { }

		String ToString()
		{
			return String(width) + "x" + String(height);
		}
	};

	FullscreenMode fullscreen_;
	Vector<Resolution> resolutions_;

	/// UI elements
	WeakPtr<UIElement>    window_;
	WeakPtr<DropDownList> resolutionList_;
	WeakPtr<DropDownList> fullScreenList_;
	WeakPtr<DropDownList> languageList_;
	WeakPtr<Button>       returnToMenu_;
	WeakPtr<Button>       applyChanges_;

	// event related functions
	void SubscribeToEvents();

	void HandleSelectFullscreen(StringHash eventType, VariantMap& eventData);
	void HandleSelectResolution(StringHash eventType, VariantMap& eventData);
	void HandleSelectLanguage(StringHash eventType, VariantMap& eventData);

	void HandleBackButtonClick(StringHash eventType, VariantMap& eventData);
	void HandleApplyButtonClick(StringHash eventType, VariantMap& eventData);
};
