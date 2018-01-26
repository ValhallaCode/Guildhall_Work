#pragma once
#include <string>

class RHIOutput;
class KerningFont;

enum eGameState
{
	MAIN_MENU,
	INFO_MENU,
	GAME,
};


class App
{
public:
	App();
	~App();
	void RunFrame();
	bool IsQuitting() const;
	void OnExitRequested();
	void OnKeyDown(unsigned char keyCode);
	void CaptureConsole(unsigned char keyCode);
	void OnKeyUp(unsigned char keyCode);
	void Initialize(RHIOutput* output);
	void RenderMainMenu() const;
	void UpdateMainMenu();
	void RenderInfoMenu() const;
	void UpdateInfoMenu();
public:
	RHIOutput* m_output;
	bool m_isQuitting;
	eGameState m_currentState;
	KerningFont* m_font;
	int m_mainMenuSelectIndex;
	int m_infoMenuSelectIndex;
	std::string m_username;
	std::string m_addrInput;
	bool m_tryingToConnect;

private:
	float CalculateDeltaSeconds();
	void Update(float deltaSeconds);
	void Render();

};

