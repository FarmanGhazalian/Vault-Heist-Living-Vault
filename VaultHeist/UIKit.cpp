#include "UIKit.h"
#include <cmath>

Button::Button()
	: bounds(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(100.0f, 40.0f)),
	  label("BUTTON"), style(BTN_PRIMARY), hovered(false), pressed(false), clicked(false), enabled(true)
{
}

Button::Button(const std::string& text, const sf::FloatRect& rect, int btnStyle)
	: bounds(rect), label(text), style(btnStyle), hovered(false), pressed(false), clicked(false), enabled(true)
{
}

void Button::handleEvent(const sf::Event& event)
{
	if (!enabled)
	{
		hovered = false;
		pressed = false;
		clicked = false;
		return;
	}

	if (const auto* mm = event.getIf<sf::Event::MouseMoved>())
	{
		sf::Vector2f mpos(static_cast<float>(mm->position.x), static_cast<float>(mm->position.y));
		hovered = bounds.contains(mpos);
	}
	else if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
	{
		if (mb->button == sf::Mouse::Button::Left)
		{
			sf::Vector2f mpos(static_cast<float>(mb->position.x), static_cast<float>(mb->position.y));
			if (bounds.contains(mpos))
			{
				pressed = true;
			}
		}
	}
	else if (const auto* mr = event.getIf<sf::Event::MouseButtonReleased>())
	{
		if (mr->button == sf::Mouse::Button::Left)
		{
			sf::Vector2f mpos(static_cast<float>(mr->position.x), static_cast<float>(mr->position.y));
			if (pressed && bounds.contains(mpos))
			{
				clicked = true;
			}
			pressed = false;
		}
	}
}

void Button::draw(sf::RenderTarget& target, const Theme& theme) const
{
	sf::Color borderColor = Theme::BorderCyan;
	sf::Color textColor = Theme::TextPrimary;
	sf::Color fillColor = sf::Color(17, 24, 39, 210);

	if (style == BTN_SECONDARY)
	{
		borderColor = Theme::BorderMuted;
		textColor = Theme::TextMuted;
	}
	else if (style == BTN_DANGER)
	{
		borderColor = Theme::CrimsonAlarm;
		textColor = Theme::CrimsonAlarm;
	}
	else if (style == BTN_BACK)
	{
		borderColor = Theme::BorderMuted;
		textColor = Theme::BorderCyan;
	}

	if (!enabled)
	{
		borderColor = sf::Color(55, 65, 81, 100);
		textColor = sf::Color(156, 163, 175, 100);
		fillColor = sf::Color(10, 14, 23, 150);
	}
	else if (pressed)
	{
		fillColor = sf::Color(10, 14, 23, 240);
	}
	else if (hovered)
	{
		fillColor = sf::Color(30, 41, 59, 230);
		if (style == BTN_PRIMARY)
		{
			theme.drawGlowOutline(target, bounds, Theme::BorderCyan, 0.8f);
		}
		else if (style == BTN_DANGER)
		{
			theme.drawGlowOutline(target, bounds, Theme::CrimsonAlarm, 0.8f);
		}
		else if (style == BTN_BACK)
		{
			theme.drawGlowOutline(target, bounds, Theme::BorderCyan, 0.5f);
		}
	}

	// Button background rect
	sf::RectangleShape rect(bounds.size);
	rect.setPosition(bounds.position);
	rect.setFillColor(fillColor);
	rect.setOutlineColor(borderColor);
	rect.setOutlineThickness(hovered ? 1.5f : 1.0f);
	target.draw(rect);

	// Label text
	unsigned int fontSize = (style == BTN_BACK) ? 14 : 15;
	sf::Text txt(theme.fontUI, label, fontSize);
	txt.setFillColor(hovered ? sf::Color::White : textColor);

	// Center text inside button bounds
	sf::FloatRect tb = txt.getLocalBounds();
	float tx = bounds.position.x + (bounds.size.x - tb.size.x) * 0.5f - tb.position.x;
	float ty = bounds.position.y + (bounds.size.y - tb.size.y) * 0.5f - tb.position.y;
	txt.setPosition(sf::Vector2f(std::round(tx), std::round(ty)));
	target.draw(txt);
}

Panel::Panel()
	: bounds(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(200.0f, 150.0f)),
	  title(""), borderColor(Theme::BorderCyan), borderWidth(1.0f)
{
}

Panel::Panel(const sf::FloatRect& rect, const std::string& panelTitle, const sf::Color& bColor, float bWidth)
	: bounds(rect), title(panelTitle), borderColor(bColor), borderWidth(bWidth)
{
}

void Panel::draw(sf::RenderTarget& target, const Theme& theme) const
{
	theme.drawPanel(target, bounds, borderColor, borderWidth);

	if (!title.empty())
	{
		sf::Text tText(theme.fontUI, title, 16);
		tText.setFillColor(Theme::BorderCyan);
		tText.setPosition(sf::Vector2f(bounds.position.x + 16.0f, bounds.position.y + 12.0f));
		target.draw(tText);

		// Divider line under title
		sf::VertexArray divider(sf::PrimitiveType::Lines, 2);
		divider[0].position = sf::Vector2f(bounds.position.x + 16.0f, bounds.position.y + 36.0f);
		divider[0].color = sf::Color(0, 240, 255, 60);
		divider[1].position = sf::Vector2f(bounds.position.x + bounds.size.x - 16.0f, bounds.position.y + 36.0f);
		divider[1].color = sf::Color(0, 240, 255, 60);
		target.draw(divider);
	}
}

namespace Tween
{
	float lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
	}

	float easeOutQuad(float t)
	{
		if (t < 0.0f) return 0.0f;
		if (t > 1.0f) return 1.0f;
		return t * (2.0f - t);
	}

	float easeInOutQuad(float t)
	{
		if (t < 0.0f) return 0.0f;
		if (t > 1.0f) return 1.0f;
		return (t < 0.5f) ? (2.0f * t * t) : (-1.0f + (4.0f - 2.0f * t) * t);
	}

	float pulse(float time, float speed)
	{
		return 0.5f * (1.0f + std::sin(time * speed));
	}

	sf::Vector2f shake(float time, float intensity)
	{
		float ox = std::sin(time * 35.0f) * intensity;
		float oy = std::cos(time * 28.0f) * intensity;
		return sf::Vector2f(ox, oy);
	}
}

// ToastManager Implementation
ToastManager::ToastManager()
{
}

void ToastManager::addToast(const std::string& message, int type)
{
	Toast t;
	t.message = message;
	t.type = type;
	t.totalTime = ToastDuration;
	t.timeRemaining = ToastDuration;
	t.animProgress = 0.0f;

	if (toasts.size() >= 3)
	{
		toasts.erase(toasts.begin());
	}
	toasts.push_back(t);
}

void ToastManager::update(float dt)
{
	for (auto it = toasts.begin(); it != toasts.end(); )
	{
		it->timeRemaining -= dt;

		// Slide down (first 0.25s) or slide up (last 0.25s)
		if (it->timeRemaining > it->totalTime - 0.25f)
		{
			it->animProgress = (it->totalTime - it->timeRemaining) / 0.25f;
		}
		else if (it->timeRemaining < 0.25f)
		{
			it->animProgress = it->timeRemaining / 0.25f;
		}
		else
		{
			it->animProgress = 1.0f;
		}

		if (it->timeRemaining <= 0.0f)
		{
			it = toasts.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void ToastManager::handleEvent(const sf::Event& event, float windowWidth)
{
	if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
	{
		if (mb->button == sf::Mouse::Button::Left)
		{
			sf::Vector2f mpos(static_cast<float>(mb->position.x), static_cast<float>(mb->position.y));
			float startX = (windowWidth - ToastWidth) * 0.5f;

			for (size_t i = 0; i < toasts.size(); i++)
			{
				float startY = 16.0f + static_cast<float>(i) * (ToastHeight + 8.0f);
				sf::FloatRect rect(sf::Vector2f(startX, startY), sf::Vector2f(ToastWidth, ToastHeight));
				if (rect.contains(mpos))
				{
					toasts.erase(toasts.begin() + i);
					break;
				}
			}
		}
	}
}

void ToastManager::draw(sf::RenderTarget& target, const Theme& theme, float windowWidth) const
{
	float startX = (windowWidth - ToastWidth) * 0.5f;

	for (size_t i = 0; i < toasts.size(); i++)
	{
		const auto& t = toasts[i];
		float targetY = 16.0f + static_cast<float>(i) * (ToastHeight + 8.0f);
		float curY = targetY - (1.0f - Tween::easeOutQuad(t.animProgress)) * 50.0f;

		sf::FloatRect bounds(sf::Vector2f(startX, curY), sf::Vector2f(ToastWidth, ToastHeight));

		sf::Color accentColor = Theme::BorderCyan;
		std::string tag = "[INFO] ";
		if (t.type == TOAST_HINT)
		{
			accentColor = Theme::AccentYellow;
			tag = "[HINT] ";
		}
		else if (t.type == TOAST_CORRECT)
		{
			accentColor = Theme::SuccessEmerald;
			tag = "[SUCCESS] ";
		}
		else if (t.type == TOAST_ERROR)
		{
			accentColor = Theme::CrimsonAlarm;
			tag = "[ALERT] ";
		}

		// Dark glass card
		sf::RectangleShape card(bounds.size);
		card.setPosition(bounds.position);
		card.setFillColor(sf::Color(17, 24, 39, 240));
		card.setOutlineColor(sf::Color(accentColor.r, accentColor.g, accentColor.b, 160));
		card.setOutlineThickness(1.0f);
		target.draw(card);

		// Left accent bar (4px)
		sf::RectangleShape bar(sf::Vector2f(4.0f, bounds.size.y));
		bar.setPosition(bounds.position);
		bar.setFillColor(accentColor);
		target.draw(bar);

		// Tag + message text
		std::string fullText = tag + t.message;
		sf::Text txt(theme.fontUI, fullText, 14);
		txt.setFillColor(Theme::TextPrimary);
		txt.setPosition(sf::Vector2f(bounds.position.x + 14.0f, bounds.position.y + 13.0f));
		target.draw(txt);

		// Remaining time thin bar at bottom
		float timerW = bounds.size.x * (t.timeRemaining / t.totalTime);
		if (timerW > 0.0f)
		{
			sf::RectangleShape timeLine(sf::Vector2f(timerW, 2.0f));
			timeLine.setPosition(sf::Vector2f(bounds.position.x, bounds.position.y + bounds.size.y - 2.0f));
			timeLine.setFillColor(accentColor);
			target.draw(timeLine);
		}
	}
}

// ModalPopup Implementation
ModalPopup::ModalPopup()
	: active(false), type(MODAL_INFO)
{
	btnConfirm = Button("OK", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(120.0f, 38.0f)), BTN_PRIMARY);
	btnCancel  = Button("CANCEL", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(120.0f, 38.0f)), BTN_SECONDARY);
}

void ModalPopup::showInfo(const std::string& modalTitle, const std::string& modalMsg, std::function<void()> onOk)
{
	active = true;
	type = MODAL_INFO;
	title = modalTitle;
	message = modalMsg;
	onConfirm = onOk;
	onCancel = nullptr;
	btnConfirm.label = "DISMISS";
}

void ModalPopup::showConfirm(const std::string& modalTitle, const std::string& modalMsg, std::function<void()> onYes, std::function<void()> onNo)
{
	active = true;
	type = MODAL_CONFIRM;
	title = modalTitle;
	message = modalMsg;
	onConfirm = onYes;
	onCancel = onNo;
	btnConfirm.label = "CONFIRM";
	btnCancel.label = "CANCEL";
}

void ModalPopup::close()
{
	active = false;
	if (onCancel)
	{
		onCancel();
	}
}

void ModalPopup::handleEvent(const sf::Event& event)
{
	if (!active) return;

	if (event.is<sf::Event::KeyPressed>())
	{
		if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
		{
			if (kp->code == sf::Keyboard::Key::Escape)
			{
				close();
				return;
			}
		}
	}

	btnConfirm.handleEvent(event);
	if (type == MODAL_CONFIRM)
	{
		btnCancel.handleEvent(event);
	}

	if (btnConfirm.isClicked())
	{
		btnConfirm.resetClick();
		active = false;
		if (onConfirm)
		{
			onConfirm();
		}
	}
	else if (type == MODAL_CONFIRM && btnCancel.isClicked())
	{
		btnCancel.resetClick();
		close();
	}
}

void ModalPopup::update(float dt)
{
	(void)dt;
}

void ModalPopup::draw(sf::RenderTarget& target, const Theme& theme, const sf::Vector2f& windowSize) const
{
	if (!active) return;

	// Dimmed dark background overlay
	sf::RectangleShape overlay(windowSize);
	overlay.setFillColor(sf::Color(10, 14, 23, 200));
	target.draw(overlay);

	// Modal dimensions
	float mw = std::min(480.0f, windowSize.x * 0.85f);
	float mh = 220.0f;
	float mx = (windowSize.x - mw) * 0.5f;
	float my = (windowSize.y - mh) * 0.5f;

	sf::FloatRect bounds(sf::Vector2f(mx, my), sf::Vector2f(mw, mh));

	// Modal body panel
	theme.drawPanel(target, bounds, Theme::BorderCyan, 1.5f);
	theme.drawGlowOutline(target, bounds, Theme::BorderCyan, 0.7f);

	// Title
	sf::Text tText(theme.fontUI, title, 18);
	tText.setFillColor(Theme::BorderCyan);
	tText.setPosition(sf::Vector2f(mx + 20.0f, my + 16.0f));
	target.draw(tText);

	// Divider
	sf::VertexArray div(sf::PrimitiveType::Lines, 2);
	div[0].position = sf::Vector2f(mx + 20.0f, my + 44.0f);
	div[0].color = sf::Color(0, 240, 255, 70);
	div[1].position = sf::Vector2f(mx + mw - 20.0f, my + 44.0f);
	div[1].color = sf::Color(0, 240, 255, 70);
	target.draw(div);

	// Message
	sf::Text msgText(theme.fontUI, message, 15);
	msgText.setFillColor(Theme::TextPrimary);
	msgText.setPosition(sf::Vector2f(mx + 20.0f, my + 60.0f));
	target.draw(msgText);

	// Buttons placement
	float btnY = my + mh - 54.0f;
	if (type == MODAL_INFO)
	{
		Button okBtn = btnConfirm;
		float bw = 140.0f;
		okBtn.bounds = sf::FloatRect(sf::Vector2f(mx + (mw - bw) * 0.5f, btnY), sf::Vector2f(bw, 38.0f));
		okBtn.draw(target, theme);
	}
	else
	{
		Button cBtn = btnConfirm;
		Button cancelBtn = btnCancel;
		float bw = 120.0f;
		float totalW = bw * 2.0f + 16.0f;
		float startBtnX = mx + (mw - totalW) * 0.5f;

		cBtn.bounds = sf::FloatRect(sf::Vector2f(startBtnX, btnY), sf::Vector2f(bw, 38.0f));
		cancelBtn.bounds = sf::FloatRect(sf::Vector2f(startBtnX + bw + 16.0f, btnY), sf::Vector2f(bw, 38.0f));

		cBtn.draw(target, theme);
		cancelBtn.draw(target, theme);
	}
}

// -------------------------------------------------------------
// LevelResultModal implementation
// -------------------------------------------------------------
LevelResultModal::LevelResultModal()
	: active(false)
	, won(false)
	, stars(0)
	, steps(0)
	, optimalSteps(0)
	, penalties(0)
	, finalScore(0)
	, btnNext("NEXT LEVEL", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(130.0f, 38.0f)), BTN_PRIMARY)
	, btnRetry("RETRY", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(100.0f, 38.0f)), BTN_SECONDARY)
	, btnBack("CAMPAIGN", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(110.0f, 38.0f)), BTN_BACK)
{
}

void LevelResultModal::show(bool victory, int starCount, int stepCount, int optSteps, int penaltyPoints, int totalScore,
                            std::function<void()> nextCallback, std::function<void()> retryCallback, std::function<void()> backCallback)
{
	active = true;
	won = victory;
	stars = std::clamp(starCount, 0, 3);
	steps = stepCount;
	optimalSteps = optSteps;
	penalties = penaltyPoints;
	finalScore = totalScore;

	onNext = nextCallback;
	onRetry = retryCallback;
	onBack = backCallback;

	title = won ? "MISSION ACCOMPLISHED" : "VAULT LOCKDOWN";
	message = won ? "Security quadrant bypassed successfully!" : "Alarm threshold exceeded. Protocol aborted.";

	// Default center positions (assumes standard 1280x720 window, updated dynamically on draw)
	float mw = 480.0f;
	float mh = 330.0f;
	float mx = std::round((1280.0f - mw) * 0.5f);
	float my = std::round((720.0f - mh) * 0.5f);
	float btnY = my + mh - 56.0f;

	if (won && onNext)
	{
		float totalBtnW = 130.0f + 105.0f + 115.0f + 20.0f;
		float startBX = mx + (mw - totalBtnW) * 0.5f;
		btnNext.bounds = sf::FloatRect(sf::Vector2f(startBX, btnY), sf::Vector2f(130.0f, 38.0f));
		btnRetry.bounds = sf::FloatRect(sf::Vector2f(startBX + 140.0f, btnY), sf::Vector2f(105.0f, 38.0f));
		btnBack.bounds = sf::FloatRect(sf::Vector2f(startBX + 255.0f, btnY), sf::Vector2f(115.0f, 38.0f));
	}
	else
	{
		float totalBtnW = 120.0f + 130.0f + 16.0f;
		float startBX = mx + (mw - totalBtnW) * 0.5f;
		btnRetry.bounds = sf::FloatRect(sf::Vector2f(startBX, btnY), sf::Vector2f(120.0f, 38.0f));
		btnBack.bounds = sf::FloatRect(sf::Vector2f(startBX + 136.0f, btnY), sf::Vector2f(130.0f, 38.0f));
	}
}

void LevelResultModal::close()
{
	active = false;
}

void LevelResultModal::handleEvent(const sf::Event& event)
{
	if (!active) return;

	if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
	{
		sf::Vector2f mpos(static_cast<float>(mb->position.x), static_cast<float>(mb->position.y));
		if (won && onNext && btnNext.bounds.contains(mpos))
		{
			close();
			onNext();
			return;
		}
		if (btnRetry.bounds.contains(mpos))
		{
			close();
			if (onRetry) onRetry();
			return;
		}
		if (btnBack.bounds.contains(mpos))
		{
			close();
			if (onBack) onBack();
			return;
		}
	}

	if (won && onNext)
	{
		btnNext.handleEvent(event);
		if (btnNext.isClicked())
		{
			btnNext.resetClick();
			close();
			onNext();
			return;
		}
	}

	btnRetry.handleEvent(event);
	if (btnRetry.isClicked())
	{
		btnRetry.resetClick();
		close();
		if (onRetry) onRetry();
		return;
	}

	btnBack.handleEvent(event);
	if (btnBack.isClicked())
	{
		btnBack.resetClick();
		close();
		if (onBack) onBack();
		return;
	}

	if (event.is<sf::Event::KeyPressed>())
	{
		if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
		{
			if (kp->code == sf::Keyboard::Key::Escape)
			{
				close();
				if (onBack) onBack();
			}
			else if (kp->code == sf::Keyboard::Key::Enter)
			{
				close();
				if (won && onNext) onNext();
				else if (onRetry) onRetry();
			}
		}
	}
}

void LevelResultModal::update(float dt)
{
	(void)dt;
}

void LevelResultModal::draw(sf::RenderTarget& target, const Theme& theme, const sf::Vector2f& windowSize) const
{
	if (!active) return;

	// Dim backdrop
	sf::RectangleShape backdrop(windowSize);
	backdrop.setFillColor(sf::Color(10, 14, 23, 210));
	target.draw(backdrop);

	// Modal window dimensions
	float mw = 480.0f;
	float mh = 330.0f;
	float mx = std::round((windowSize.x - mw) * 0.5f);
	float my = std::round((windowSize.y - mh) * 0.5f);
	sf::FloatRect bounds(sf::Vector2f(mx, my), sf::Vector2f(mw, mh));

	// Draw glass panel
	sf::Color borderColor = won ? Theme::SuccessEmerald : Theme::CrimsonAlarm;
	theme.drawGlowOutline(target, bounds, borderColor, 1.2f);
	theme.drawPanel(target, bounds, borderColor, 1.5f);

	// Header
	sf::Text titleText(theme.fontUI, title, 22);
	titleText.setFillColor(borderColor);
	titleText.setStyle(sf::Text::Bold);
	sf::FloatRect tb = titleText.getLocalBounds();
	titleText.setPosition(sf::Vector2f(std::round(mx + (mw - tb.size.x) * 0.5f - tb.position.x), my + 20.0f));
	target.draw(titleText);

	// Sub-message
	sf::Text msgText(theme.fontUI, message, 13);
	msgText.setFillColor(Theme::TextMuted);
	sf::FloatRect mb = msgText.getLocalBounds();
	msgText.setPosition(sf::Vector2f(std::round(mx + (mw - mb.size.x) * 0.5f - mb.position.x), my + 52.0f));
	target.draw(msgText);

	// Draw Stars [ ★ ★ ★ ]
	std::string starStr = "";
	for (int i = 0; i < 3; i++)
	{
		starStr += (i < stars) ? "[*] " : "[ ] ";
	}
	sf::Text starText(theme.fontMono, won ? ("RATING: " + starStr) : "RATING: FAILED", 15);
	starText.setFillColor(won ? Theme::AccentYellow : Theme::CrimsonAlarm);
	starText.setStyle(sf::Text::Bold);
	sf::FloatRect stb = starText.getLocalBounds();
	starText.setPosition(sf::Vector2f(std::round(mx + (mw - stb.size.x) * 0.5f - stb.position.x), my + 82.0f));
	target.draw(starText);

	// Stats Box
	float sbW = mw - 50.0f;
	float sbH = 95.0f;
	float sbX = mx + 25.0f;
	float sbY = my + 118.0f;

	sf::RectangleShape statsBox(sf::Vector2f(sbW, sbH));
	statsBox.setPosition(sf::Vector2f(sbX, sbY));
	statsBox.setFillColor(sf::Color(13, 19, 33, 220));
	statsBox.setOutlineThickness(1.0f);
	statsBox.setOutlineColor(Theme::BorderMuted);
	target.draw(statsBox);

	// Stats Content (Two columns)
	std::string col1 = "STEPS TAKEN:  " + std::to_string(steps) + "\n" +
	                   "OPTIMAL STEPS:" + std::to_string(optimalSteps);
	sf::Text tCol1(theme.fontMono, col1, 12);
	tCol1.setFillColor(Theme::TextPrimary);
	tCol1.setPosition(sf::Vector2f(sbX + 20.0f, sbY + 22.0f));
	target.draw(tCol1);

	std::string col2 = "PENALTIES:   -" + std::to_string(penalties) + " pts\n" +
	                   "FINAL SCORE:  " + std::to_string(finalScore) + " pts";
	sf::Text tCol2(theme.fontMono, col2, 12);
	tCol2.setFillColor(won ? Theme::SuccessEmerald : Theme::CrimsonAlarm);
	tCol2.setStyle(sf::Text::Bold);
	tCol2.setPosition(sf::Vector2f(sbX + sbW * 0.52f, sbY + 22.0f));
	target.draw(tCol2);

	// Draw Actual Member Buttons (Const-cast to draw directly so positions match 100%)
	auto* nonConstThis = const_cast<LevelResultModal*>(this);
	float btnY = my + mh - 56.0f;

	if (won && onNext)
	{
		float totalBtnW = 130.0f + 105.0f + 115.0f + 20.0f;
		float startBX = mx + (mw - totalBtnW) * 0.5f;

		nonConstThis->btnNext.bounds = sf::FloatRect(sf::Vector2f(startBX, btnY), sf::Vector2f(130.0f, 38.0f));
		nonConstThis->btnRetry.bounds = sf::FloatRect(sf::Vector2f(startBX + 140.0f, btnY), sf::Vector2f(105.0f, 38.0f));
		nonConstThis->btnBack.bounds = sf::FloatRect(sf::Vector2f(startBX + 255.0f, btnY), sf::Vector2f(115.0f, 38.0f));

		nonConstThis->btnNext.draw(target, theme);
		nonConstThis->btnRetry.draw(target, theme);
		nonConstThis->btnBack.draw(target, theme);
	}
	else
	{
		float totalBtnW = 120.0f + 130.0f + 16.0f;
		float startBX = mx + (mw - totalBtnW) * 0.5f;

		nonConstThis->btnRetry.bounds = sf::FloatRect(sf::Vector2f(startBX, btnY), sf::Vector2f(120.0f, 38.0f));
		nonConstThis->btnBack.bounds = sf::FloatRect(sf::Vector2f(startBX + 136.0f, btnY), sf::Vector2f(130.0f, 38.0f));

		nonConstThis->btnRetry.draw(target, theme);
		nonConstThis->btnBack.draw(target, theme);
	}
}
