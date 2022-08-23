#pragma once



class IWindowWidget
{
public:
	IWindowWidget();
	virtual ~IWindowWidget();

	void Render();
	void Focus();
	void Close();
	bool WantsToClose() const;

private:
	virtual const char* GetWindowName() const = 0;
	virtual void RenderContents() = 0;

	bool m_wantsFocus = true;
	bool m_isOpen = true;
};