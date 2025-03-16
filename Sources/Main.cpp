#include "Pch.hpp"

#include "DearImGui/backends/imgui_impl_sdl3.h"
#include "DearImGui/backends/imgui_impl_sdlgpu3.h"
#include "DearImGui/imgui.h"
#include "DearImGui/ImPlot/implot.h"
#include <SDL3/SDL.h>

#include "Font/IconsMaterialDesignIcons.h"
#include "Font/MaterialDesignIcons.ttf.hpp"
#include "Font/Roboto-Regular.ttf.hpp"

#include "OS.hpp"
#include "Process.hpp"
#include "ProcessDrawer.hpp"
#include "SystemWatcher.hpp"

#include <functional>
#include <mutex>

#include "RingBuffer.hpp"

void SetupDarkTheme()
{
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.0f);
	colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.0f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.22f, 0.23f, 0.25f, 1.0f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.05f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
	colors[ImGuiCol_Separator] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.17f, 0.18f, 0.2f, 1.0f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.23f, 0.25f, 1.0f);
	colors[ImGuiCol_TabSelected] = ImVec4(0.22f, 0.23f, 0.25f, 1.0f);
	colors[ImGuiCol_TabDimmed] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	// colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	// colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.25f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	// colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding = ImVec2(8.00f, 8.00f);
	style.FramePadding = ImVec2(5.00f, 2.00f);
	style.CellPadding = ImVec2(6.00f, 6.00f);
	style.ItemSpacing = ImVec2(6.00f, 6.00f);
	style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
	style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
	style.IndentSpacing = 25;
	style.ScrollbarSize = 15;
	style.GrabMinSize = 10;
	style.WindowBorderSize = 1;
	style.ChildBorderSize = 1;
	style.PopupBorderSize = 1;
	style.FrameBorderSize = 1;
	style.TabBorderSize = 1;
	style.WindowRounding = 7;
	style.ChildRounding = 4;
	style.FrameRounding = 3;
	style.PopupRounding = 4;
	style.ScrollbarRounding = 9;
	style.GrabRounding = 3;
	style.LogSliderDeadzone = 4;
	style.TabRounding = 4;
	style.DisabledAlpha = 0.5f;
}

bool Init(SystemWatcher& systemWatcher, SDL_Window*& window, SDL_GPUDevice*& gpu_device)
{
	if (OS::Init() == false)
	{
		return false;
	}

	if (systemWatcher.Init() == false)
	{
		return false;
	}

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) != 0)
	{
		printf("Error: SDL_Init(): %s\n", SDL_GetError());
		return false;
	}

	// Create SDL window graphics context
	window = SDL_CreateWindow("ProcTracker", 768, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
	if (window == nullptr)
	{
		printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		return false;
	}

	// Create GPU Device
	gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB, false, nullptr);
	if (gpu_device == nullptr)
	{
		printf("Error: SDL_CreateGPUDevice(): %s\n", SDL_GetError());
		return false;
	}

	// Claim window for GPU Device
	if (!SDL_ClaimWindowForGPUDevice(gpu_device, window))
	{
		printf("Error: SDL_ClaimWindowForGPUDevice(): %s\n", SDL_GetError());
		return false;
	}
	SDL_SetGPUSwapchainParameters(gpu_device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	SetupDarkTheme();

	ImFontConfig robotoFontConfig;
	robotoFontConfig.FontDataOwnedByAtlas = false;
	robotoFontConfig.SizePixels = 13.0f;
	robotoFontConfig.OversampleH = 1;
	robotoFontConfig.OversampleV = 1;
	robotoFontConfig.PixelSnapH = true;
	io.Fonts->AddFontFromMemoryTTF(Roboto_Regular_ttf, Roboto_Regular_ttf_size, 16.0f, &robotoFontConfig);
	io.Fonts->Build();

	const ImWchar materialDesignIconFontRanges[] = {ICON_MIN_MDI, ICON_MAX_MDI, 0};
	ImFontConfig  materialDesignIconFontConfig;
	materialDesignIconFontConfig.FontDataOwnedByAtlas = false;
	materialDesignIconFontConfig.MergeMode = true;
	materialDesignIconFontConfig.PixelSnapH = true;
	materialDesignIconFontConfig.GlyphOffset.y = 2;
	materialDesignIconFontConfig.GlyphOffset.x = -0.5;
	materialDesignIconFontConfig.GlyphMaxAdvanceX = 13;

	io.Fonts->AddFontFromMemoryTTF(MaterialDesignIcons_ttf, MaterialDesignIcons_ttf_size, 16.0f, &materialDesignIconFontConfig, materialDesignIconFontRanges);
	io.Fonts->Build();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForSDLGPU(window);
	ImGui_ImplSDLGPU3_InitInfo init_info = {};
	init_info.Device = gpu_device;
	init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu_device, window);
	init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
	ImGui_ImplSDLGPU3_Init(&init_info);

	return true;
}

void Terminate(SystemWatcher& systemWatcher, SDL_Window* window, SDL_GPUDevice* gpu_device)
{
	SDL_WaitForGPUIdle(gpu_device);
	ImGui_ImplSDL3_Shutdown();
	ImGui_ImplSDLGPU3_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();

	SDL_ReleaseWindowFromGPUDevice(gpu_device, window);
	SDL_DestroyGPUDevice(gpu_device);
	SDL_DestroyWindow(window);
	SDL_Quit();

	systemWatcher.Terminate();
	OS::Terminate();
}

int main(int argc, char** argv)
{
	SystemWatcher  systemWatcher;
	SDL_Window*    window = nullptr;
	SDL_GPUDevice* gpu_device = nullptr;

	if (Init(systemWatcher, window, gpu_device) == false)
	{
		return -1;
	}

	static char processName[4096] = {'\0'};
	std::strcpy(processName, "ProcTracker");

	std::mutex                                        processesLock;
	std::vector<Process*>                             processes;
	std::vector<ProcessDrawer*>                       processDrawers;
	std::function<void(const std::string&, uint64_t)> onProcessCreated(
		[&processesLock, &processes, &processDrawers](const std::string& name, uint64_t pid)
		{
			processesLock.lock();
			processes.push_back(new Process(pid, name));
			processDrawers.push_back(new ProcessDrawer(*processes.back()));
			processesLock.unlock();
		});
	std::function<void(uint64_t)> onProcessTerminated(
		[&processesLock, &processes, &processDrawers](uint64_t pid)
		{
			processesLock.lock();
			for (size_t i = 0; i < processes.size(); ++i)
			{
				Process* process = processes[i];
				if (process->pid == pid)
				{
					ProcessDrawer* processDrawer = processDrawers[i];
					processes.erase(processes.begin() + i);
					processDrawers.erase(processDrawers.begin() + i);
					delete process;
					delete processDrawer;
					break;
				}
			}
			processesLock.unlock();
		});
	systemWatcher.StartWatch(processName, onProcessCreated, onProcessTerminated);

	float maxTimer = 0.025f;
	float timer = maxTimer;
	bool  exit = false;
	while (exit == false)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT)
			{
				exit = true;
			}
			if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
			{
				exit = true;
			}
		}
		if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
		{
			SDL_Delay(10);
			continue;
		}

		ImGui_ImplSDLGPU3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		bool update = false;
		timer += ImGui::GetIO().DeltaTime;
		if (timer >= maxTimer)
		{
			timer -= maxTimer;
			update = true;
		}

		static bool show_demo_window = true;
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Quit"))
				{
					exit = true;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("About"))
				{
					// todo open about modal
				}
				ImGui::EndMenu();
			}

			/*
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Process Name");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-1);
			*/

			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.0f, 0));
			if (ImGui::BeginTable("MainBarTable", 3))
			{
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				float cellWidth = ImGui::GetContentRegionAvail().x;
				float contentWidth = ImGui::CalcTextSize(ICON_MDI_TARGET).x * 2 + 350.0f + ImGui::GetStyle().ItemSpacing.x * 2;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cellWidth - contentWidth) * 0.5f);

				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.0f, 0.0f, 1.0f));
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(ICON_MDI_TARGET);
				ImGui::PopStyleColor();
				ImGui::SameLine();
				ImGui::SetNextItemWidth(350.f);
				const char* placeholder = "Enter the process name to track...";
				const char* hint = placeholder;
				if (processName[0] != '\0')
				{
					hint = processName;
				}

				if (ImGui::InputTextWithHint("##ProcessName", hint, processName, sizeof(processName), ImGuiInputTextFlags_EnterReturnsTrue))
				{
					systemWatcher.StopWatch();
					for (Process* process : processes)
					{
						delete process;
					}
					for (ProcessDrawer* processDrawer : processDrawers)
					{
						delete processDrawer;
					}
					processes.clear();
					processDrawers.clear();
					systemWatcher.StartWatch(processName, onProcessCreated, onProcessTerminated);
				}

				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.0f, 0.0f, 1.0f));
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(ICON_MDI_TARGET);
				ImGui::PopStyleColor();

				ImGui::TableNextColumn();
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Refresh Rate");
				ImGui::SetNextItemWidth(100.f);
				ImGui::SliderFloat("##RefreshRate", &maxTimer, 0.01f, 1.f, "%.2fs");

				ImGui::TableNextColumn();
				ImVec2 buttonSize = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
				ImGui::SetWindowFontScale(0.9f);
				if (ImGui::Button(ICON_MDI_COG, buttonSize))
				{
				}
				ImGui::SetWindowFontScale(1.0f);

				ImGui::EndTable();
			}
			ImGui::PopStyleVar();

			// ImGui::Checkbox("Show Demo", &show_demo_window);

			ImGui::EndMainMenuBar();
		}

		ImVec2 pos(0, ImGui::GetFrameHeight());
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size - pos);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		bool opened = ImGui::Begin("Tmp", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
		ImGui::PopStyleVar();
		if (opened)
		{
			if (ImGui::BeginTabBar("##processTabBar"))
			{
				processesLock.lock();
				for (uint32_t i {0}; i < processDrawers.size(); ++i)
				{
					if (update)
					{
						processDrawers[i]->Update();
					}
					processDrawers[i]->Draw();
				}
				processesLock.unlock();
				ImGui::EndTabBar();
			}
		}
		ImGui::End();

		if (show_demo_window)
		{
			ImGui::ShowDemoWindow(&show_demo_window);
		}

		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
		const bool  is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);

		SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(gpu_device); // Acquire a GPU command buffer

		SDL_GPUTexture* swapchain_texture;
		SDL_AcquireGPUSwapchainTexture(command_buffer, window, &swapchain_texture, nullptr, nullptr); // Acquire a swapchain texture

		if (swapchain_texture != nullptr && !is_minimized)
		{
			// This is mandatory: call Imgui_ImplSDLGPU3_PrepareDrawData() to upload the vertex/index buffer!
			Imgui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);

			// Setup and start a render pass
			SDL_GPUColorTargetInfo target_info = {};
			target_info.texture = swapchain_texture;
			target_info.clear_color = SDL_FColor {0.0f, 0.0f, 0.0f, 1.0f};
			target_info.load_op = SDL_GPU_LOADOP_CLEAR;
			target_info.store_op = SDL_GPU_STOREOP_STORE;
			target_info.mip_level = 0;
			target_info.layer_or_depth_plane = 0;
			target_info.cycle = false;
			SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &target_info, 1, nullptr);

			// Render ImGui
			ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, render_pass);

			SDL_EndGPURenderPass(render_pass);
		}

		// Submit the command buffer
		SDL_SubmitGPUCommandBuffer(command_buffer);
	}

	Terminate(systemWatcher, window, gpu_device);

	return 0;
}
