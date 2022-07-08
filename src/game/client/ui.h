/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_UI_H
#define GAME_CLIENT_UI_H

#include "ui_rect.h"

#include <engine/input.h>
#include <engine/textrender.h>

#include <chrono>
#include <string>
#include <vector>

struct SUIAnimator
{
	bool m_Active;
	bool m_ScaleLabel;
	bool m_RepositionLabel;

	std::chrono::nanoseconds m_Time;
	float m_Value;

	float m_XOffset;
	float m_YOffset;
	float m_WOffset;
	float m_HOffset;
};

class CUI;

class CUIElement
{
	friend class CUI;

	CUI *m_pUI;

	CUIElement(CUI *pUI, int RequestedRectCount) { Init(pUI, RequestedRectCount); }

public:
	struct SUIElementRect
	{
		CUIElement *m_pParent;

	public:
		int m_UIRectQuadContainer;
		int m_UITextContainer;

		float m_X;
		float m_Y;
		float m_Width;
		float m_Height;

		std::string m_Text;

		CTextCursor m_Cursor;

		ColorRGBA m_TextColor;
		ColorRGBA m_TextOutlineColor;

		SUIElementRect();

		ColorRGBA m_QuadColor;

		void Reset();
		void Draw(const CUIRect *pRect, ColorRGBA Color, int Corners, float Rounding);
	};

protected:
	CUI *UI() const { return m_pUI; }
	std::vector<SUIElementRect> m_vUIRects;

	// used for marquees or other user implemented things
	int64_t m_ElementTime;

public:
	CUIElement() = default;

	void Init(CUI *pUI, int RequestedRectCount);

	SUIElementRect *Get(size_t Index)
	{
		return &m_vUIRects[Index];
	}

	bool AreRectsInit()
	{
		return !m_vUIRects.empty();
	}

	void InitRects(int RequestedRectCount);
};

struct SLabelProperties
{
	float m_MaxWidth = -1;
	int m_AlignVertically = 1;
	bool m_StopAtEnd = false;
	class CTextCursor *m_pSelCursor = nullptr;
	bool m_EnableWidthCheck = true;
};

class CButtonContainer
{
};

class CUI
{
	bool m_Enabled;

	const void *m_pHotItem;
	const void *m_pActiveItem;
	const void *m_pLastActiveItem;
	const void *m_pBecomingHotItem;
	const void *m_pActiveTooltipItem;
	bool m_ActiveItemValid = false;

	float m_MouseX, m_MouseY; // in gui space
	float m_MouseDeltaX, m_MouseDeltaY; // in gui space
	float m_MouseWorldX, m_MouseWorldY; // in world space
	unsigned m_MouseButtons;
	unsigned m_LastMouseButtons;

	CUIRect m_Screen;

	std::vector<CUIRect> m_vClips;
	void UpdateClipping();

	class IInput *m_pInput;
	class IGraphics *m_pGraphics;
	class ITextRender *m_pTextRender;

	std::vector<CUIElement *> m_vpOwnUIElements; // ui elements maintained by CUI class
	std::vector<CUIElement *> m_vpUIElements;

public:
	static float ms_FontmodHeight;

	// TODO: Refactor: Fill this in
	void Init(class IInput *pInput, class IGraphics *pGraphics, class ITextRender *pTextRender);
	class IInput *Input() const { return m_pInput; }
	class IGraphics *Graphics() const { return m_pGraphics; }
	class ITextRender *TextRender() const { return m_pTextRender; }

	CUI();
	~CUI();

	void ResetUIElement(CUIElement &UIElement);

	CUIElement *GetNewUIElement(int RequestedRectCount);

	void AddUIElement(CUIElement *pElement);
	void OnElementsReset();
	void OnWindowResize();
	void OnLanguageChange();

	void SetEnabled(bool Enabled) { m_Enabled = Enabled; }
	bool Enabled() const { return m_Enabled; }
	void Update(float MouseX, float MouseY, float MouseWorldX, float MouseWorldY);

	float MouseDeltaX() const { return m_MouseDeltaX; }
	float MouseDeltaY() const { return m_MouseDeltaY; }
	float MouseX() const { return m_MouseX; }
	float MouseY() const { return m_MouseY; }
	float MouseWorldX() const { return m_MouseWorldX; }
	float MouseWorldY() const { return m_MouseWorldY; }
	int MouseButton(int Index) const { return (m_MouseButtons >> Index) & 1; }
	int MouseButtonClicked(int Index) const { return MouseButton(Index) && !((m_LastMouseButtons >> Index) & 1); }
	int MouseButtonReleased(int Index) const { return ((m_LastMouseButtons >> Index) & 1) && !MouseButton(Index); }

	void SetHotItem(const void *pID) { m_pBecomingHotItem = pID; }
	void SetActiveItem(const void *pID)
	{
		m_ActiveItemValid = true;
		m_pActiveItem = pID;
		if(pID)
			m_pLastActiveItem = pID;
	}
	bool CheckActiveItem(const void *pID)
	{
		if(m_pActiveItem == pID)
		{
			m_ActiveItemValid = true;
			return true;
		}
		return false;
	}
	void SetActiveTooltipItem(const void *pID) { m_pActiveTooltipItem = pID; }
	void ClearLastActiveItem() { m_pLastActiveItem = nullptr; }
	const void *HotItem() const { return m_pHotItem; }
	const void *NextHotItem() const { return m_pBecomingHotItem; }
	const void *ActiveItem() const { return m_pActiveItem; }
	const void *ActiveTooltipItem() const { return m_pActiveTooltipItem; }
	const void *LastActiveItem() const { return m_pLastActiveItem; }

	void StartCheck() { m_ActiveItemValid = false; }
	void FinishCheck()
	{
		if(!m_ActiveItemValid && m_pActiveItem != nullptr)
		{
			SetActiveItem(nullptr);
			m_pHotItem = nullptr;
			m_pBecomingHotItem = nullptr;
		}
	}

	bool MouseInside(const CUIRect *pRect) const;
	bool MouseInsideClip() const { return !IsClipped() || MouseInside(ClipArea()); }
	bool MouseHovered(const CUIRect *pRect) const { return MouseInside(pRect) && MouseInsideClip(); }
	void ConvertMouseMove(float *pX, float *pY, IInput::ECursorType CursorType) const;

	float ButtonColorMulActive() { return 0.5f; }
	float ButtonColorMulHot() { return 1.5f; }
	float ButtonColorMulDefault() { return 1.0f; }
	float ButtonColorMul(const void *pID);

	CUIRect *Screen();
	void MapScreen();
	float PixelSize();

	void ClipEnable(const CUIRect *pRect);
	void ClipDisable();
	const CUIRect *ClipArea() const;
	inline bool IsClipped() const { return !m_vClips.empty(); }

	int DoButtonLogic(const void *pID, int Checked, const CUIRect *pRect);
	int DoPickerLogic(const void *pID, const CUIRect *pRect, float *pX, float *pY);

	float DoTextLabel(float x, float y, float w, float h, const char *pText, float Size, int Align, const SLabelProperties &LabelProps = {});
	void DoLabel(const CUIRect *pRect, const char *pText, float Size, int Align, const SLabelProperties &LabelProps = {});

	void DoLabel(CUIElement::SUIElementRect &RectEl, const CUIRect *pRect, const char *pText, float Size, int Align, const SLabelProperties &LabelProps, int StrLen = -1, class CTextCursor *pReadCursor = nullptr);
	void DoLabelStreamed(CUIElement::SUIElementRect &RectEl, float x, float y, float w, float h, const char *pText, float Size, int Align, float MaxWidth = -1, int AlignVertically = 1, bool StopAtEnd = false, int StrLen = -1, class CTextCursor *pReadCursor = nullptr);
	void DoLabelStreamed(CUIElement::SUIElementRect &RectEl, const CUIRect *pRect, const char *pText, float Size, int Align, float MaxWidth = -1, int AlignVertically = 1, bool StopAtEnd = false, int StrLen = -1, class CTextCursor *pReadCursor = nullptr);
};

#endif
