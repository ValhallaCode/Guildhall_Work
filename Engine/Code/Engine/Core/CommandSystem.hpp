#pragma once
#include <string>
#include "Engine/Render/Rgba.hpp"
#include "Engine/Core/Event.hpp"
#include <map>
#include <vector>

typedef void(*ConsoleCommand)(void* args);
typedef std::vector<std::vector<std::string>>::iterator LogIterate;
class KerningFont;
class ShaderProgram;
class Mesh;
class Texture2D;
class UICanvas;
class UIEditableText;
class UIPanel;

struct arguments
{
	std::vector<std::string> arg_list;
	void Insert(const std::string& arg)
	{
		arg_list.push_back(arg);
	};
};

struct command_cb 
{
	std::string command;
	Rgba cmd_color;
	std::string short_desc;
	std::string long_desc;
	ConsoleCommand call_back;

	command_cb() {};
	command_cb(std::string string)
		:command(string)
		, cmd_color(Rgba(255,255,255,255))
	{}

	bool operator==(command_cb& cmd)
	{
		bool msg = this->command.compare(cmd.command) == 0;
		return msg;
	}

	bool operator<(command_cb& cmd)
	{
		bool msg = this->command.compare(cmd.command) < 0;
		return msg;
	}

	bool operator>=(command_cb& cmd)
	{
		return !(*this < cmd);
	}

	bool operator!=(command_cb& cmd)
	{
		return !(*this == cmd);
	}
};

class CommandSystem {
public:
	CommandSystem(KerningFont* consoleFont);
	~CommandSystem();
	void RegisterCommand(std::string commandText, ConsoleCommand commandFunc, Rgba cmd_color = Rgba(255,255,255,255), 
		std::string short_desc = "", std::string long_desc = "");
	void RunCommand(std::string commandText);
	bool FindCommand(std::string commandText);
	void ConsolePrintf(Rgba color, const char* msg, ...);//Rgba color, std::string description, std::string command);
	void Update(float deltaSeconds);
	void Render() const;
	void AddToInput(unsigned char charaterAdded);
	void RemoveLastCharacter();
	void ClearInput();
	void ClearHistory();
	bool IsActive();
	void ToggleActivity();
	void Execute();
	int GetInputSize();
	bool IsInputEmpty();
	void SetFontShader(const std::string& name, std::string file_path = "");
	void SetBackDropShader(const std::string& name, std::string file_path = "");
	void SetTextureForBackDrop(const std::string& name, std::string file_path = "");
	void CreateTextFromCommand(const command_cb& command);
public:
	std::vector<command_cb> m_logHistory;
	std::vector<std::vector<std::string>> m_commandLog;
	std::map<std::string, command_cb> m_commands;
	std::vector<Texture2D*> m_backDropTextures;
	LogIterate m_logTracker;
	Texture2D* m_currentTex;
	KerningFont* m_font;
	bool m_isActive;
	float m_totalTime;
	static Event<const std::string&> OnMessagePrint;

	//UI Stuff
	UICanvas* m_canvas;
	UIPanel* m_textPanel;
	UIEditableText* m_entryText;
};

//Event<const std::string&> CommandSystem::OnMessagePrint;