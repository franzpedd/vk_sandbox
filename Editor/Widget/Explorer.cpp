#include "Explorer.h"

#include "Core/Application.h"
#include <Common/File/Datafile.h>
#include <Common/File/Filesystem.h>
#include <Common/Util/Algorithm.h>
#include <Engine/Core/Scene.h>
#include <Renderer/GUI/Icon.h>
#include <Renderer/Vulkan/Texture.h>

#include <filesystem>

namespace Cosmos::Editor
{
	Explorer::Explorer(Application* application)
		: Widget("Explorer"), mApplication(application)
	{
		// create used resources used by the explorer
		mCurrentDir = GetAssetsDir();

		// default path for the resources
		mAssetsPath[Asset::Type::Undefined] = GetAssetSubDir("Texture/Editor/undef.png");
		mAssetsPath[Asset::Type::Folder] = GetAssetSubDir("Texture/Editor/folder.png");
		mAssetsPath[Asset::Type::Text] = GetAssetSubDir("Texture/Editor/text.png");
		mAssetsPath[Asset::Type::Scene] = GetAssetSubDir("Texture/Editor/scene.png");
		mAssetsPath[Asset::Type::Vert] = GetAssetSubDir("Texture/Editor/vert.png");
		mAssetsPath[Asset::Type::Frag] = GetAssetSubDir("Texture/Editor/frag.png");
		mAssetsPath[Asset::Type::Spv] = GetAssetSubDir("Texture/Editor/spv.png");
		mAssetsPath[Asset::Type::Mesh] = GetAssetSubDir("Texture/Editor/mesh.png");
		mAssetsPath[Asset::Type::Sound] = GetAssetSubDir("Texture/Editor/sound.png");

		// create default resources for each supported format
		for (uint32_t i = 0; i < Asset::Type::ASSET_TYPE_MAX; i++) 
		{
			// skip image, it has it's own asset view type
			if (i == Asset::Type::Image) {
				continue;
			}

			mAssets[i].view.texture = CreateShared<Renderer::Vulkan::Texture2D>(mAssetsPath[i], true);
			mAssets[i].view.descriptor = (VkDescriptorSet)Renderer::GUI::GetRef().AddTexture(mAssets[i].view.texture);
		}

		// create default resource for parent folder
		mParentFolder.path = GetAssetsDir();
		mParentFolder.name = "...";
		mParentFolder.view.descriptor = mAssets[Asset::Type::Folder].view.descriptor;
		mParentFolder.view.texture = mAssets[Asset::Type::Folder].view.texture;
	}

	Explorer::~Explorer()
	{
		mCurrentDirAssets.clear();
	}

	void Explorer::OnUpdate()
	{
		if (mOpened)
		{
			ImGui::Begin(ICON_FA_FOLDER " Explorer", nullptr);

			// search box part
			{
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				std::strncpy(buffer, mSearchboxText.c_str(), sizeof(buffer));

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 2.0f));
				if (ImGui::InputTextWithHint("##SearchAsset", ICON_LC_SEARCH " Search", buffer, sizeof(buffer))) {
					mSearchboxText = buffer;
					mRefreshExplorer = true;
				}
				ImGui::PopStyleVar();

				ImGui::SameLine();

				if (ImGui::Checkbox("Include sub-folders", &mRecursiveSearch)) {
					mRefreshExplorer = true;
				}				
			}

			// separator
			std::string separatorText = ICON_FA_CHEVRON_RIGHT;
			separatorText.append(mCurrentDir);
			ImGui::SeparatorText(separatorText.c_str());

			ImGui::NewLine();

			// check if contents should be refreshed
			if (mRefreshExplorer) {
				Refresh(mCurrentDir);
			}

			// assets part
			{
				const ImVec2 size = ImVec2(64.0f, 64.0f);
				ImVec2 lastItemPosition = ImGui::GetCursorPos();

				// draw the parent folder if searchbox is empty (not on search mode)
				if (mSearchboxText.empty()) {
					DrawAsset(mParentFolder, lastItemPosition, size);
				}

				// draw assets
				for (size_t i = 0; i < mCurrentDirAssets.size(); i++) {
					DrawAsset(mCurrentDirAssets[i], lastItemPosition, size);
				}
			}

			ImGui::End();
		}
	}

	void Explorer::DrawAsset(Asset& asset, ImVec2& position, const ImVec2 buttonSize)
	{
		constexpr uint32_t displayMaxChars = 32;

		// check if item will be displaying on the same line
		if ((position.x + (buttonSize.x * 2)) <= ImGui::GetContentRegionAvail().x) {
			ImGui::SameLine();
		}

		// the group consists of the asset's image and name
		ImGui::BeginGroup();
		{
			if (ImGui::ImageButton(asset.path.c_str(), asset.view.descriptor, buttonSize))
			{
				std::filesystem::path path(asset.path);
				auto dir = std::filesystem::directory_entry(path);

				if (dir.is_directory()) {
					mRefreshExplorer = true;
					mCurrentDir = dir.path().string();
				}
			}

			position = ImGui::GetCursorPos();
			ImGui::SetCursorPosX(position.x + 5.0f); // 5.0 is the ofset

			// name
			if (asset.name.size() > displayMaxChars) {
				std::string shortName = asset.name.substr(0, displayMaxChars);
				ImGui::Text("%s", shortName.c_str());
			}

			else {
				ImGui::Text("%s", asset.name.c_str());
			}
		}
		ImGui::EndGroup();

		// drag behaviour
		if (asset.type != Asset::Type::Folder)
		{
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID | ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
				ImGui::BeginTooltip();
				ImGui::Text("%s", asset.path.c_str());
				ImGui::EndTooltip();
		
				ImGui::SetDragDropPayload("EXPLORER", asset.path.c_str(), asset.path.size() + 1, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}
		}

		DisplayAssetMenu(asset);
	}

	void Explorer::Refresh(std::string path)
	{
		mCurrentDirAssets.clear();
		mRefreshExplorer = false;

		// holds paths
		std::vector<std::filesystem::path> paths = {};

		// recusive search
		if (mRecursiveSearch && !mSearchboxText.empty()) {
			for (auto const& dirEntry : std::filesystem::recursive_directory_iterator(path)) {
				if (dirEntry.path().string().find(mSearchboxText) != std::string::npos) {
					paths.push_back(dirEntry.path());
				}
			}
		}

		// current path search
		else {
			for (auto const& dirEntry : std::filesystem::directory_iterator(path)) {

				// not on search mode, append all dir paths
				if (mSearchboxText.empty()) {
					paths.push_back(dirEntry.path());
					continue;
				}

				// on search mode, mSearchboxText must be a substring of dirEntry.filename
				if (dirEntry.path().filename().string().find(mSearchboxText) != std::string::npos) {
					paths.push_back(dirEntry.path());
				}
			}
		}

		// draw all found assets, according with previous specifications
		for (auto& path : paths) {

			// string manipulation
			std::string ext = path.extension().string();
			std::filesystem::path pathCorrected = path;

			// default asset configs
			Asset asset = {};
			asset.path = pathCorrected.string();
			asset.name = pathCorrected.filename().replace_extension().string();
			Cosmos::replace(asset.path.begin(), asset.path.end(), char('\\'), char('/'));

			// check if it's a folder
			if (std::filesystem::is_directory(path)) {
				asset.type = Asset::Type::Folder;
				asset.view = mAssets[1].view;

				mCurrentDirAssets.push_back(asset);
				continue;
			}

			// text files
			if (strcmp(".txt", ext.c_str()) == 0 || strcmp(".cfg", ext.c_str()) == 0 || strcmp(".cfg", ext.c_str()) == 0) {
				asset.type = Asset::Type::Text;
				asset.view = mAssets[Asset::Type::Text].view;

				mCurrentDirAssets.push_back(asset);
				continue;
			}

			// scenes
			if (strcmp(".scene", ext.c_str()) == 0) {
				asset.type = Asset::Type::Scene;
				asset.view = mAssets[Asset::Type::Scene].view;

				mCurrentDirAssets.push_back(asset);
				continue;
			}

			// vertex shaders
			if (strcmp(".vert", ext.c_str()) == 0) {
				asset.type = Asset::Type::Vert;
				asset.view = mAssets[Asset::Type::Vert].view;

				mCurrentDirAssets.push_back(asset);
				continue;
			}

			// fragment shader
			if (strcmp(".frag", ext.c_str()) == 0) {
				asset.type = Asset::Type::Frag;
				asset.view = mAssets[Asset::Type::Frag].view;

				mCurrentDirAssets.push_back(asset);
				continue;
			}

			// spir-v shader
			if (strcmp(".spv", ext.c_str()) == 0) {
				asset.type = Asset::Type::Spv;
				asset.view = mAssets[Asset::Type::Spv].view;

				mCurrentDirAssets.push_back(asset);
				continue;
			}

			// meshes
			if (strcmp(".gltf", ext.c_str()) == 0) {
				asset.type = Asset::Type::Mesh;
				asset.view = mAssets[Asset::Type::Mesh].view;

				mCurrentDirAssets.push_back(asset);
				continue;
			}

			// sound
			if (strcmp(".mp3", ext.c_str()) == 0  || strcmp(".wav", ext.c_str()) == 0  || strcmp(".ogg", ext.c_str()) == 0) {
				asset.type = Asset::Type::Sound;
				asset.view = mAssets[Asset::Type::Sound].view;

				mCurrentDirAssets.push_back(asset);
				continue;
			}

			// images
			if (strcmp(".png", ext.c_str()) == 0 || strcmp(".jpg", ext.c_str()) == 0)
			{
				asset.type = Asset::Type::Image;
				asset.view.texture = CreateShared<Renderer::Vulkan::Texture2D>(asset.path.c_str(), true);
				asset.view.descriptor = (VkDescriptorSet)Renderer::GUI::GetRef().AddTexture(asset.view.texture);

				mCurrentDirAssets.push_back(asset);
				continue;
			}
		}
	}
	
    void Explorer::DisplayAssetMenu(Asset& asset)
    {
		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
			return;
		}
		
		ImGui::PushID((&asset));
		switch (asset.type)
		{
			case Asset::Type::Scene:
			{
				if (ImGui::BeginPopupContextItem("##RightClickExplorerMesh", ImGuiPopupFlags_MouseButtonRight)) {
					if (ImGui::MenuItem(ICON_LC_FOLDER " Load")) {

						Datafile scene;
						Datafile::Read(scene, asset.path.c_str());
						mApplication->GetCurrentScene()->Deserialize(scene);

					}

					ImGui::EndPopup();
				}
				break;
			}

			default: { break; }
		}
		ImGui::PopID();
    }
}