#include "Engine/Core/CommandSystem.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Render/KerningFont.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/UI/UICanvas.hpp"
#include "Engine/UI/UIPanel.hpp"
#include "Engine/UI/UIText.hpp"
#include "Engine/UI/UIEditableText.hpp"
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <stdarg.h>
#include <algorithm>

Event<const std::string&> CommandSystem::OnMessagePrint;

//*******************************************************************
CommandSystem::~CommandSystem()
{

}

//*******************************************************************
CommandSystem::CommandSystem(KerningFont* consoleFont) 
	:m_isActive(false)
	, m_totalTime(0.0f)
	, m_logTracker(m_commandLog.end())
	, m_font(consoleFont)
{
	m_canvas = new UICanvas();
	m_canvas->SetTargetResolution(720.0f);
	m_canvas->SetTint(Rgba(255, 255, 255, 128));

	m_entryText = new UIEditableText();
	m_canvas->AddChild(m_entryText);
	m_entryText->SetSizeRatio(Vector2(0.9f, 0.1f));
	m_entryText->SetPivot(Vector2(0.0f, 0.0f));
	m_entryText->SetPosition(Vector2(0.0f, 0.0f), Vector2(60.0f, 20.0f));
	m_entryText->SetScale(1.25f);
	m_entryText->SetAspectRatio(1.6f);
	m_entryText->SetLineSpacing(0.0f);
	m_entryText->SetKerningFont(consoleFont);
	m_entryText->SetBlinksPerSecond(3.0f);

	m_textPanel = new UIPanel();
	m_canvas->AddChild(m_textPanel);
	m_textPanel->SetSizeRatio(Vector2(0.9f, 0.75f));
	m_textPanel->SetPivot(Vector2(0.0f, 0.0f));
	m_textPanel->SetPosition(Vector2(0.0f, 0.15f), Vector2(60.0f, 20.0f));
	m_textPanel->SetTint(Rgba(255, 255, 255, 0));
}

//RegisterCommand("clear", ConsoleClear);
//RegisterCommand("save_console_to_file", SaveConsoleToFile);
//*******************************************************************
void CommandSystem::RegisterCommand(std::string commandText, ConsoleCommand commandFunc, Rgba cmd_color /*= Rgba(255,255,255,255)*/, 
	std::string short_desc /*= ""*/, std::string long_desc /*= ""*/)
{
	command_cb command;
	command.command = commandText;
	command.cmd_color = cmd_color;
	command.short_desc = short_desc;
	command.long_desc = long_desc;
	command.call_back = commandFunc;

	m_commands.insert_or_assign(commandText, command);
}

//char const *command = "save_console_to_file console.log";
//*******************************************************************
void CommandSystem::RunCommand(std::string commandText)
{
	if (commandText.empty())
		return;

	std::vector<std::string> parts;
	bool is_in_quotes = false;
	std::string current_string;
	commandText += " ";
	for (uint index = 0; index < commandText.size(); ++index)
	{
		char current_char = commandText[index];

		if (current_char == '"')
		{
			is_in_quotes = !is_in_quotes;
		}

		if (current_char == ' ' && !is_in_quotes) 
		{
			if(current_string.empty())
				continue;

			parts.push_back(current_string);
			current_string.clear();
			continue;
		}

		if(current_char != '"')
			current_string.push_back(current_char);
	}

	commandText.pop_back();

	if (FindCommand(parts[0]))
	{
		auto iterate = m_commands.find(parts[0]);

		command_cb commandInfo;
		commandInfo.command = commandText;
		commandInfo.cmd_color = iterate->second.cmd_color;
		m_logHistory.push_back(commandInfo);
		CreateTextFromCommand(commandInfo);
		m_commandLog.push_back(parts);

		// Part to change to support multiple arguments
		if (parts.size() < 2) 
		{
			arguments default;
			iterate->second.call_back(&default);
		}
		else {
			arguments args;
			for (int index = 1; index < (int)parts.size(); ++index)
			{	
				args.Insert(parts[index]);
			}

			iterate->second.call_back(&args);
		}

		m_logTracker = m_commandLog.end();
	}
	else
	{
		ConsolePrintf(Rgba(255, 0, 0, 255), "Failed to find commands: %s", parts[0].c_str());
	}
}

//*******************************************************************
bool CommandSystem::FindCommand(std::string commandText)
{
	return m_commands.find(commandText) != m_commands.end();
}

//*******************************************************************
void CommandSystem::ConsolePrintf(Rgba color, const char* msg, ...)//*Rgba color, std::string description, std::string command*/)
{
	va_list variableArgumentList;
	va_start(variableArgumentList, msg);

	const int MESSAGE_MAX_LENGTH = 2048;
	char messageLiteral[MESSAGE_MAX_LENGTH];
	vsnprintf_s(messageLiteral, MESSAGE_MAX_LENGTH, _TRUNCATE, msg, variableArgumentList);
	va_end(variableArgumentList);
	messageLiteral[MESSAGE_MAX_LENGTH - 1] = '\0';

	command_cb commandInfo;
	commandInfo.command = messageLiteral;
	commandInfo.cmd_color = color;
	m_logHistory.push_back(commandInfo);
	CreateTextFromCommand(commandInfo);

	CommandSystem::OnMessagePrint.trigger(commandInfo.command);
}

void CommandSystem::Update(float deltaSeconds)
{
	if (!IsActive())
		return;

	if (g_theInputSystem->WasKeyJustPressed(KEY_TILDE))
	{
		ClearInput();
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_UPARROW))
	{
		ClearInput();

		if (m_logTracker != m_commandLog.begin())
		{
			m_logTracker -= 1;
		}

		if (m_logTracker != m_commandLog.end())
		{
			std::vector<std::string> user_input = *m_logTracker;

			std::string new_input;
			for (uint index = 0; index < user_input.size(); ++index)
			{
				new_input.append(user_input[index]);
				if(index != user_input.size() - 1)
					new_input.append(" ");
			}

			m_entryText->SetString(new_input);
			m_entryText->SetCursorLocation(new_input.size());
		}
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_DOWNARROW))
	{
		ClearInput();

		if (m_logTracker != m_commandLog.end())
		{
			m_logTracker += 1;
		}

		if (m_logTracker != m_commandLog.end())
		{
			std::vector<std::string> user_input = *m_logTracker;

			std::string new_input;
			for (uint index = 0; index < user_input.size(); ++index)
			{
				new_input.append(user_input[index]);
				if (index != user_input.size() - 1)
					new_input.append(" ");
			}

			m_entryText->SetString(new_input);
			m_entryText->SetCursorLocation(new_input.size());
		}
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_LEFTARROW))
	{
		m_entryText->MoveCursorLeft();
	}
	
	if (g_theInputSystem->WasKeyJustPressed(KEY_RIGHTARROW))
	{
		m_entryText->MoveCursorRight();
	}

	m_totalTime += deltaSeconds;

	float erase_remainder = std::fmod(m_totalTime, 0.05f);
	if (g_theInputSystem->IsKeyDown(KEY_BACK) && !IsInputEmpty() && erase_remainder <= deltaSeconds)
	{
		g_console->RemoveLastCharacter();
	}

	if (m_backDropTextures.empty())
	{
		m_currentTex = g_simpleRenderer->m_whiteTexture;
	}
	else
	{
		float frame_rate = 0.0417f;
		float duration; 

		if (m_backDropTextures.size() > 1)
		{
			duration = (m_backDropTextures.size() - 1) / frame_rate;
		}
		else
		{
			duration = (m_backDropTextures.size()) / frame_rate;
		}

		float eval_time = std::fmod(m_totalTime, duration * frame_rate);
		int tex_index = (int)floor(eval_time);

		m_currentTex = m_backDropTextures[tex_index];

		m_canvas->SetTexture(m_currentTex);
	}
}

void CommandSystem::Render() const
{
	if (m_isActive)
	{
		m_canvas->Render();
	}
}

void CommandSystem::AddToInput(unsigned char charaterAdded)
{
	m_entryText->InsertCharacter(charaterAdded);
}

void CommandSystem::RemoveLastCharacter()
{
	m_entryText->RemoveCharacter();
}

void CommandSystem::ClearInput()
{
	m_entryText->Reset();
}

void CommandSystem::ClearHistory()
{
	m_logHistory.clear();
}

bool CommandSystem::IsActive()
{
	return m_isActive;
}

void CommandSystem::ToggleActivity()
{
	m_isActive = !m_isActive;
}

void CommandSystem::Execute()
{
	RunCommand(m_entryText->GetString());
}

int CommandSystem::GetInputSize()
{
	return m_entryText->GetLength();
}

bool CommandSystem::IsInputEmpty()
{
	return m_entryText->IsEmpty();
}

void CommandSystem::SetFontShader(const std::string& name, std::string file_path /*= ""*/)
{
	ShaderProgram* shader = CreateOrGetShaderProgram(name, file_path, g_simpleRenderer);
	m_entryText->SetShader(shader);
}

void CommandSystem::SetBackDropShader(const std::string& name, std::string file_path /*= ""*/)
{
	ShaderProgram* shader = CreateOrGetShaderProgram(name, file_path, g_simpleRenderer);
	m_canvas->SetShader(shader);
}

void CommandSystem::SetTextureForBackDrop(const std::string& name, std::string file_path /*= ""*/)
{
	Texture2D* tex = CreateOrGetTexture2D(name, g_simpleRenderer, file_path);
	m_backDropTextures.push_back(tex);
}

void CommandSystem::CreateTextFromCommand(const command_cb& command)
{
	UIText* text = new UIText();

	m_textPanel->AddChild(text);
	text->SetPivot(Vector2(0.0f, 0.0f));
	text->SetPosition(Vector2(0.0f, 0.0f), Vector2(0.0f, 0.0f));
	text->SetSizeRatio(Vector2(1.0f, 0.018f));
	text->SetString(command.command);
	text->SetScale(1.0f);
	text->SetAspectRatio(1.0f);
	text->SetLineSpacing(10.0f);
	text->SetKerningFont(m_font);
	text->SetFontColor(command.cmd_color);
	text->SetTextWrapping(true);
	text->EnableBoundsFitToText(true);
	text->SetBoundsCutOff(false);

	float bound_height = text->GetBounds().maxs.y - text->GetBounds().mins.y;

	for (UIElement* elem : m_textPanel->m_children)
	{
		elem->m_position.unit.y += bound_height;
	}
}
