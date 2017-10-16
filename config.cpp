#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/File.h>

#include "config.h"

#ifdef _DEBUG
static const U32 DEFAULT_WIDTH = 1280;
static const U32 DEFAULT_HEIGHT = 720;
static const bool DEFAULT_FULLSCREEN = false;
static const bool DEFAULT_BORDERLESS = false;
static const F32 DEFAULT_SOUND_VOLUME = 1.0f;
#else
// use desktop resolution
static const U32 DEFAULT_WIDTH = 0;
static const U32 DEFAULT_HEIGHT = 0;
static const bool DEFAULT_FULLSCREEN = true;
static const bool DEFAULT_BORDERLESS = true;
static const F32 DEFAULT_SOUND_VOLUME = 1.0f;
#endif // _DEBUG

static String DefaultLang = "en";
static String DefaultServerAddress = "localhost";
static const U32 DEFAULT_SERVER_PORT = 23450;

HashMap<String, Variant> DefaultParameterValues =
{
	{ "width", DEFAULT_WIDTH },
	{ "height", DEFAULT_HEIGHT },
	{ "fullscreen", DEFAULT_FULLSCREEN },
	{ "borderless", DEFAULT_BORDERLESS },
	{ "sound", DEFAULT_SOUND_VOLUME },
	{ "address", DefaultServerAddress },
	{ "port", DEFAULT_SERVER_PORT },
	{ "lang", DefaultLang }
};

Configuration::ActionsMap Configuration::DefaultActionsMap =
{
	{ Configuration::GameInputActions::MoveForward,  {{0, {Configuration::InputDeviceType::Keyboard, KEY_W}}, {1, {Configuration::InputDeviceType::Keyboard, KEY_UP}}}},
	{ Configuration::GameInputActions::MoveBackward, {{0, {Configuration::InputDeviceType::Keyboard, KEY_S}}, {1, {Configuration::InputDeviceType::Keyboard, KEY_DOWN}}}},
	{ Configuration::GameInputActions::MoveLeft,     {{0, {Configuration::InputDeviceType::Keyboard, KEY_A}}, {1, {Configuration::InputDeviceType::Keyboard, KEY_LEFT}}}},
	{ Configuration::GameInputActions::MoveRight,    {{0, {Configuration::InputDeviceType::Keyboard, KEY_D}}, {1, {Configuration::InputDeviceType::Keyboard, KEY_RIGHT}}}},
	{ Configuration::GameInputActions::FirePrimary,  {{0, {Configuration::InputDeviceType::Mouse, MOUSEB_LEFT}}}},
	{ Configuration::GameInputActions::FireSecondary,{{0, {Configuration::InputDeviceType::Mouse, MOUSEB_RIGHT}}}},
	{ Configuration::GameInputActions::FireThird,    {{0, {Configuration::InputDeviceType::Keyboard, KEY_Q}}}},
	{ Configuration::GameInputActions::FireUltimate, {{0, {Configuration::InputDeviceType::Keyboard, KEY_E}}}}
};

String Configuration::StringFromEnumActions(GameInputActions inputAction)
{
	switch (inputAction)
	{
		case GameInputActions::MoveForward:
			return "MoveForward";
		case GameInputActions::MoveBackward:
			return "MoveBackward";
		case GameInputActions::MoveLeft:
			return "MoveLeft";
		case GameInputActions::MoveRight:
			return "MoveRight";
		case GameInputActions::FirePrimary:
			return "FirePrimary";
		case GameInputActions::FireSecondary:
			return "FireSecondary";
		case GameInputActions::FireThird:
			return "FireThird";
		case GameInputActions::FireUltimate:
			return "FireUltimate";
		default:
			return "";
	}
}

String Configuration::StringFromDeviceType(InputDeviceType deviceType)
{
	switch (deviceType)
	{
		case InputDeviceType::Keyboard:
			return "Keyboard";
		case InputDeviceType::Mouse:
			return "Mouse";
		default:
			return "";
	}
}

String Configuration::MouseKeyName(S32 key)
{
	switch (key)
	{
		case MOUSEB_LEFT:
			return "MLEFT";
		case MOUSEB_MIDDLE:
			return "MMIDDLE";
		case MOUSEB_RIGHT:
			return "MRIGHT";
		case MOUSEB_X1:
			return "MX1";
		case MOUSEB_X2:
			return "MX2";
	}

	return String::EMPTY;
}

S32 Configuration::MouseKeyFromName(const String& name)
{
	S32 key = 0;
	if (name == "MLEFT")
		key = MOUSEB_LEFT;
	else if (name == "MMIDDLE")
		key = MOUSEB_MIDDLE;
	else if (name == "MRIGHT")
		key = MOUSEB_RIGHT;
	else if (name == "MX1")
		key = MOUSEB_X1;
	else if (name == "MX2")
		key = MOUSEB_X2;

	return key;
}

String Configuration::StringFromKey(InputDeviceType deviceType, S32 key) const
{
	Input* input = GetSubsystem<Input>();
	if (!input)
		return String::EMPTY;

	switch (deviceType)
	{
		case InputDeviceType::Keyboard:
		{
			return input->GetKeyName(key);
		}
		case InputDeviceType::Mouse:
		{
			return MouseKeyName(key);
		}
		default:
			return "";
	}
}

Configuration::Configuration(Context* context)
	: Object(context)
	, jsonFile_(context)
{
	FileSystem* filesystem = GetSubsystem<FileSystem>();

#ifdef _DEBUG
	configFileName_ = filesystem->GetProgramDir() + "config_d.json";
#else
	configFileName_ = filesystem->GetProgramDir() + "config.json";
#endif // _DEBUG
}

void Configuration::Load()
{
	bool loadingFailed = true;
	bool needStoring = false;
	FileSystem* filesystem = GetSubsystem<FileSystem>();
	if (filesystem->FileExists(configFileName_))
	{
		File configFile(context_, configFileName_, FILE_READ);
		if (jsonFile_.BeginLoad(configFile))
		{
			loadingFailed = false;

			JSONValue& root = jsonFile_.GetRoot();
			for (auto& defaultValue : DefaultParameterValues)
			{
				if (!root.Contains(defaultValue.first_))
				{
					SetValue(defaultValue.first_, defaultValue.second_);
					needStoring = true;
				}
			}

			userActionMap_ = DefaultActionsMap;
			if (!root.Contains("controls"))
			{
				SaveUserActionMap();
				needStoring = true;
			}
			else
			{
				LoadActionMap();
			}
		}
	}
	

	if(loadingFailed)
	{
		JSONValue& root = jsonFile_.GetRoot();
		for (auto& defaultValue : DefaultParameterValues)
		{
			SetValue(defaultValue.first_, defaultValue.second_);
		}

		userActionMap_ = DefaultActionsMap;
		SaveUserActionMap();

		needStoring = true;
	}

	if (needStoring)
	{
		Save();
	}
}

void Configuration::Save()
{
	File configFile(context_, configFileName_, FILE_WRITE);
	jsonFile_.Save(configFile);
}

void Configuration::LoadActionMap()
{
	Input* input = GetSubsystem<Input>();
	if (!input)
		return;

	JSONValue& controlsJson = jsonFile_.GetRoot()["controls"];

	for (U32 action = static_cast<U32>(GameInputActions::MoveForward); action < static_cast<U32>(GameInputActions::Count); action++)
	{
		String actionName = StringFromEnumActions(static_cast<GameInputActions>(action));
		if (controlsJson.Contains(actionName))
		{
			auto& userAction = userActionMap_[static_cast<GameInputActions>(action)];

			auto controlSetJson = controlsJson[actionName].GetArray();

			for (U32 actionUnitNumber = 0; actionUnitNumber < 2; actionUnitNumber++)
			{
				if (!controlsJson[actionName].Contains(String(actionUnitNumber)))
					continue;

				auto& controlUnitJson = controlsJson[actionName][String(actionUnitNumber)];
				String deviceStr = controlUnitJson["device"].GetString();
				String keyStr = controlUnitJson["key"].GetString();

				S32 key = KEY_UNKNOWN;
				if (deviceStr == "Keyboard")
				{
					key = input->GetKeyFromName(keyStr);
					userAction[actionUnitNumber].deviceType_ = InputDeviceType::Keyboard;
				}
				else if (deviceStr == "Mouse")
				{
					key = MouseKeyFromName(keyStr);
					userAction[actionUnitNumber].deviceType_ = InputDeviceType::Mouse;
				}

				userAction[actionUnitNumber].key_ = key;
			}
		}
	}
}

void Configuration::SaveUserActionMap()
{
	Input* input = GetSubsystem<Input>();
	if (!input)
		return;

	JSONValue& root = jsonFile_.GetRoot();
	root["controls"] = JSONValue();

	JSONValue& controlsJson = root["controls"];
	for (auto& userAction : userActionMap_)
	{
		String actionName = StringFromEnumActions(userAction.first);

		for (auto& userActionUnit : userAction.second)
		{
			controlsJson[actionName][String(userActionUnit.first)] = JSONValue();
			JSONValue& actionUnitJson = controlsJson[actionName][String(userActionUnit.first)];

			actionUnitJson["device"] = StringFromDeviceType(userActionUnit.second.deviceType_);
			if (userActionUnit.second.deviceType_ == InputDeviceType::Keyboard)
				actionUnitJson["key"] = input->GetKeyName(userActionUnit.second.key_);
			else if (userActionUnit.second.deviceType_ == InputDeviceType::Mouse)
				actionUnitJson["key"] = MouseKeyName(userActionUnit.second.key_);
		}
	}
}

void Configuration::SetValue(const String& name, Variant value)
{
	jsonFile_.GetRoot()[name].SetVariant(value);
}

Variant Configuration::GetValue(const String& name) const
{
	return jsonFile_.GetRoot()[name].GetVariant();
}

bool Configuration::GetActionKeyInput(GameInputActions action) const
{
	Input* input = GetSubsystem<Input>();
	if (!input)
		return false;

	auto userActionIt = userActionMap_.find(action);
	if (userActionIt == userActionMap_.end())
		return false;

	bool keyInputWorked = false;
	auto& userAction = userActionIt->second;

	for (auto& userActionUnit : userAction)
	{
		if (userActionUnit.second.deviceType_ == InputDeviceType::Keyboard)
		{
			keyInputWorked = keyInputWorked || input->GetKeyDown(userActionUnit.second.key_);
		}
		else if (userActionUnit.second.deviceType_ == InputDeviceType::Mouse)
		{
			keyInputWorked = keyInputWorked || input->GetMouseButtonDown(userActionUnit.second.key_);
		}
	}

	return keyInputWorked;
}

String Configuration::GetActionKeyName(GameInputActions action, U32 unitNumber) const
{
	auto userActionIt = userActionMap_.find(action);
	if (userActionIt == userActionMap_.end())
		return String::EMPTY;

	auto& userAction = userActionIt->second;

	auto userActionUnitIt = userAction.find(unitNumber);
	if (userActionUnitIt == userAction.end())
		return String::EMPTY;

	return StringFromKey(userActionUnitIt->second.deviceType_, userActionUnitIt->second.key_);
}

void Configuration::SetActionKey(GameInputActions action, InputDeviceType device, S32 key, U32 unitNumber)
{
	auto& userAction = userActionMap_[action];

	userAction[unitNumber].deviceType_ = device;
	userAction[unitNumber].key_ = key;

	SaveUserActionMap();
}

