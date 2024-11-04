#include "Console.h"

#include <Common/Debug/Logger.h>
#include <Renderer/GUI/Icon.h>
#include <Renderer/Wrapper/imgui.h>

namespace Cosmos::Editor
{
	Console::Console()
		: Widget("Console")
	{
	}

	void Console::OnUpdate()
	{
		if (mOpened) {
			ImGui::Begin(ICON_FA_TERMINAL " Console", nullptr, ImGuiWindowFlags_HorizontalScrollbar);

			char buffer[32] = {};
			std::string command = {};
			if (ImGui::InputTextWithHint("##ConsoleInputText", "type a command", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll)) {
				command = std::string(buffer);
			}

			ImGui::SameLine();

			if (ImGui::Button("Enter ##ConsoleInputText:Enter")) {
				COSMOS_LOG(Logger::Warn, "Not yet Implemented");
			}

			ImGui::Separator();

			for (size_t i = Logger::GetInstance().GetMessages().size() - 1; i > 0 ; i--) {
				Logger::ConsoleMessage& msg = Logger::GetInstance().GetMessages()[i];
				ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

				switch (msg.severity) {
					case Logger::Severity::Trace: {
						color = ImVec4(0.0f, 0.5f, 0.6f, 1.0f);
						ImGui::TextColored(color, ICON_FA_INFO_CIRCLE " %s", msg.message.c_str());
						break;
					}

					case Logger::Severity::Info: {
						color = ImVec4(0.0f, 0.86f, 1.0f, 1.0f);
						ImGui::TextColored(color, ICON_FA_INFO_CIRCLE " %s", msg.message.c_str());
						break;
					}

					case Logger::Severity::Warn: {
						color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
						ImGui::TextColored(color, ICON_FA_QUESTION_CIRCLE " %s", msg.message.c_str());
						break;
					}

					case Logger::Severity::Error: {
						color = ImVec4(1.0f, 0.65f, 0.0f, 1.0f);
						ImGui::TextColored(color, ICON_FA_QUESTION_CIRCLE " %s", msg.message.c_str());
						break;
					}
				}
			}

			ImGui::End();
		}
	}
}