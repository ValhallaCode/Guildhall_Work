#include "Engine/UI/UIText.hpp"
#include "Engine/Core/Interval.hpp"
typedef unsigned int uint;

class UIEditableText : public UIText
{
public:
	UIEditableText();
	virtual ~UIEditableText();
	void MoveCursorRight();
	void MoveCursorLeft();
	void SetBlinksPerSecond(float blinks);
	void SetCursorLocation(int index);
	void InsertCharacter(char new_char);
	void RemoveCharacter();
	virtual void Render() override;
	void Reset();
	int GetLength();
	bool IsEmpty();
	std::string GetString();
private:
	void GenerateCursorMesh();
	void SetTranslationBasedOnIndex(int new_index);
public:
	uint m_cursorIndex;
	uint m_cursorDrawIndex;
	Vector3 m_cursorTranslation;
	Mesh* m_cursorMesh;
	Interval m_cursorInterval;
	bool m_canDrawCursor;
	bool m_addedCharacterToEnd;
};