#pragma once


class App
{
public:
	App();
	~App();
	void RunFrame();
	bool IsQuitting() const;
	void OnExitRequested();
	void OnKeyDown(unsigned char keyCode);
	void OnKeyUp(unsigned char keyCode);

private:
	float CalculateDeltaSeconds();
	void Update(float deltaSeconds);
	void Render();

};

extern App* g_theApp;