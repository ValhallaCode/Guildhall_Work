#pragma once

class RHIOutput;

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

	RHIOutput* m_output;
	bool m_isQuitting;
private:
	float CalculateDeltaSeconds();
	void Update(float deltaSeconds);
	void Render();

};

