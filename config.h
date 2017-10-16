#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/JSONFile.h>

#include "utility/simpleTypes.h"

#include <unordered_map>
#include <map>

using namespace Urho3D;

class Configuration : public Object
{
	URHO3D_OBJECT(Configuration, Object);

public:

	struct EnumClassHash
	{
		template <typename T>
		std::size_t operator()(T t) const
		{
			return static_cast<U32>(t);
		}
	};

	enum class GameInputActions : U32
	{
		MoveForward = 0,
		MoveBackward,
		MoveLeft,
		MoveRight,
		FirePrimary,
		FireSecondary,
		FireThird,
		FireUltimate,
		Count
	};

	enum class InputDeviceType
	{
		No_Device = -1,
		Mouse = 0,
		Keyboard,
		Count
	};

	struct ActionUnit
	{
		InputDeviceType deviceType_ = InputDeviceType::No_Device;
		S32             key_        = KEY_UNKNOWN;

		ActionUnit() = default;

		ActionUnit(InputDeviceType deviceType, S32 key)
			: deviceType_(deviceType)
			, key_(key)
		{ }
	};

	using ActionsMap = std::unordered_map<GameInputActions, std::map<U32, ActionUnit>, EnumClassHash>;
	static ActionsMap DefaultActionsMap;

	static String StringFromEnumActions(GameInputActions inputAction);
	static String StringFromDeviceType(InputDeviceType deviceType);

	static String MouseKeyName(S32 key);
	static S32 MouseKeyFromName(const String& name);

	String StringFromKey(InputDeviceType device, S32 key) const;

	/// Construct.
	Configuration(Context* context);

	void Load();
	void Save();

	void LoadActionMap();
	void SaveUserActionMap();

	void SetValue(const String& name, Variant value);
	Variant GetValue(const String& name) const;

	bool GetActionKeyInput(GameInputActions action) const;

	/**
	 * unitNumber == 0 for primary key
	 * unitNumber == 1 for secondary key
	 */
	String GetActionKeyName(GameInputActions action, U32 unitNumber) const;
	void SetActionKey(GameInputActions action, InputDeviceType device, S32 key, U32 unitNumber);
private:

	JSONFile jsonFile_;
	String configFileName_;

	ActionsMap userActionMap_;
};
