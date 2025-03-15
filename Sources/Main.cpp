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
#include "PerformanceQuery.hpp"
#include "Process.hpp"
#include "SystemSpecs.hpp"
#include "SystemWatcher.hpp"

#include "Handle/Handle.hpp"
#include "Handle/HandleQuery.hpp"

#include <algorithm>
#include <format>
#include <functional>
#include <mutex>

#include "RingBuffer.hpp"

#undef max

ImPlotPoint ImPlotRingBufferGetterFloat(int index, void* user_data)
{
	RingBuffer<float>* ringBuffer = static_cast<RingBuffer<float>*>(user_data);
	return ImPlotPoint(index, ringBuffer->GetRawData()[ringBuffer->GetRawIndex(index)]);
}

ImPlotPoint ImPlotRingBufferGetterFloatZeroY(int index, void* user_data)
{
	RingBuffer<float>* ringBuffer = static_cast<RingBuffer<float>*>(user_data);
	return ImPlotPoint(index, 0.0f);
}

ImPlotPoint ImPlotRingBufferGetterUInt64(int index, void* user_data)
{
	RingBuffer<uint64_t>* ringBuffer = static_cast<RingBuffer<uint64_t>*>(user_data);
	return ImPlotPoint(index, ringBuffer->GetRawData()[ringBuffer->GetRawIndex(index)]);
}

ImPlotPoint ImPlotRingBufferGetterUInt64ZeroY(int index, void* user_data)
{
	RingBuffer<uint64_t>* ringBuffer = static_cast<RingBuffer<uint64_t>*>(user_data);
	return ImPlotPoint(index, 0.0f);
}

int SizeFormatter(double value, char* buff, int size, void* user_data)
{
	std::pair<double, const char*> adjustedSize = AdjustSizeValue(value);
	return snprintf(buff, size, "%.2f %s", adjustedSize.first, adjustedSize.second);
}

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

int main(int argc, char** argv)
{
	if (OS::Init() == false)
	{
		return -1;
	}

	SystemWatcher systemWatcher;
	if (systemWatcher.Init() == false)
	{
		return -1;
	}

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) != 0)
	{
		printf("Error: SDL_Init(): %s\n", SDL_GetError());
		return -1;
	}

	// Create SDL window graphics context
	SDL_Window* window = SDL_CreateWindow("ProcTracker", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
	if (window == nullptr)
	{
		printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		return -1;
	}

	// Create GPU Device
	SDL_GPUDevice* gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB, false, nullptr);
	if (gpu_device == nullptr)
	{
		printf("Error: SDL_CreateGPUDevice(): %s\n", SDL_GetError());
		return -1;
	}

	// Claim window for GPU Device
	if (!SDL_ClaimWindowForGPUDevice(gpu_device, window))
	{
		printf("Error: SDL_ClaimWindowForGPUDevice(): %s\n", SDL_GetError());
		return -1;
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

	static char processName[4096] = {'\0'};
	std::strcpy(processName, "ProcTracker");
	SystemSpecs specs;

	std::mutex                                        processesLock;
	std::vector<Process*>                             processes;
	std::vector<Query>                                queries;
	std::function<void(const std::string&, uint64_t)> onProcessCreated(
		[&processesLock, &processes, &queries](const std::string& name, uint64_t pid)
		{
			processesLock.lock();
			processes.push_back(new Process(pid, name));
			queries.emplace_back(processes.back());
			processesLock.unlock();
		});
	std::function<void(uint64_t)> onProcessTerminated(
		[&processesLock, &processes, &queries](uint64_t pid)
		{
			processesLock.lock();
			for (size_t i = 0; i < processes.size(); ++i)
			{
				Process* process = processes[i];
				if (process->pid == pid)
				{
					processes.erase(processes.begin() + i);
					queries.erase(queries.begin() + i);
					delete process;
					break;
				}
			}
			processesLock.unlock();
		});
	systemWatcher.StartWatch(processName, onProcessCreated, onProcessTerminated);

	float maxTimer = 1.f;
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
					processes.clear();
					queries.clear();
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
				for (uint32_t i {0}; i < processes.size(); ++i)
				{
					Process*        process = processes[i];
					static uint32_t lastTab {0};
					if (ImGui::BeginTabItem(std::format("{}", process->pid).data()))
					{
						static PerformanceSnapshot  data;
						static RingBuffer<float>    cpuUsageBuffer(256, 0.0f);
						static uint64_t             maxRam = 0;
						static RingBuffer<uint64_t> ramBuffer(256, 0.0f);

						if (lastTab != i)
						{
							update = true;
							lastTab = i;
							cpuUsageBuffer.Fill(0.0f);
							ramBuffer.Fill(0.0f);
							maxRam = 0;
						}

						if (update)
						{
							data = queries[i].Retrieve(specs);
							cpuUsageBuffer.PushBack(data.cpuUsage);
							ramBuffer.PushBack(data.privateBytes);
							if (data.privateBytes > maxRam)
							{
								maxRam = data.privateBytes;
							}
						}

						if (ImGui::Button(ICON_MDI_SKULL_CROSSBONES " Kill me !"))
						{
						}
						ImGui::SameLine(0.0, 15.0f);
						ImGui::Text("CPU:  %.2f%%", data.cpuUsage);
						ImGui::SameLine(0.0, 15.0f);
						std::pair<double, const char*> adjustedSize = AdjustSizeValue(data.privateBytes);
						ImGui::Text("RAM:  %.2f %s", adjustedSize.first, adjustedSize.second);

						if (ImGui::BeginChild("SubWindowForScroll", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground))
						{
							if (ImGui::BeginChild("CollapsablePlotWindowCPU", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_None))
							{
								ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
								ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered]);
								ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive]);
								bool opened = ImGui::CollapsingHeader("CPU", ImGuiTreeNodeFlags_DefaultOpen);
								ImGui::PopStyleColor(3);
								if (opened)
								{
									ImPlot::SetNextAxisLimits(ImAxis_X1, 0, cpuUsageBuffer.GetSize(), ImGuiCond_Always);
									ImPlot::SetNextAxisLimits(ImAxis_Y1, 0.0f, 100.0f, ImGuiCond_Always);
									if (ImPlot::BeginPlot("##CPU_Plot", ImVec2(-1, 200), ImPlotFlags_NoFrame))
									{
										ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, 0);
										ImPlot::PlotShadedG("CPU (%)", &ImPlotRingBufferGetterFloat, &cpuUsageBuffer, &ImPlotRingBufferGetterFloatZeroY, &cpuUsageBuffer,
										                    cpuUsageBuffer.GetSize());
										ImPlot::EndPlot();
									}
								}
							}
							ImGui::EndChild();

							if (ImGui::BeginChild("CollapsablePlotWindowRAM", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_None))
							{
								ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
								ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered]);
								ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive]);
								bool opened = ImGui::CollapsingHeader("RAM", ImGuiTreeNodeFlags_DefaultOpen);
								ImGui::PopStyleColor(3);
								if (opened)
								{
									ImPlot::SetNextAxisLimits(ImAxis_X1, 0, cpuUsageBuffer.GetSize(), ImGuiCond_Always);
									ImPlot::SetNextAxisLimits(ImAxis_Y1, 0.0f, (double)maxRam * 1.3f, ImGuiCond_Always);
									if (ImPlot::BeginPlot("##RAM_Plot", ImVec2(-1, 200), ImPlotFlags_NoFrame))
									{
										// ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
										ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, 0);
										/*
										static const double customTicks[] = {1, 1000, 1000000, 1000000000, 1000000000000};
										static const char*  customLabels[] = {"B", "KB", "MB", "GB", "TB"};
										ImPlot::SetupAxisTicks(ImAxis_Y1, customTicks, IM_ARRAYSIZE(customTicks), customLabels);
										*/
										ImPlot::SetupMouseText(ImPlotLocation_SouthEast, ImPlotMouseTextFlags_NoAuxAxes);
										ImPlot::SetupAxisFormat(ImAxis_Y1, &SizeFormatter);
										ImPlot::PlotShadedG("RAM", &ImPlotRingBufferGetterUInt64, &ramBuffer, &ImPlotRingBufferGetterUInt64ZeroY, &ramBuffer,
										                    cpuUsageBuffer.GetSize());
										ImPlot::EndPlot();
									}
								}
							}
							ImGui::EndChild();

							ImGui::Text("CPU: %.2f%%", data.cpuUsage);
							Time time = AdjustTimeValue(data.time);
							if (time.d)
							{
								ImGui::Text("Elapsed Time: %ud%uh%um%us", time.d, time.h, time.m, time.s);
							}
							else if (time.h)
							{
								ImGui::Text("Elapsed Time: %uh%um%us", time.h, time.m, time.s);
							}
							else if (time.m)
							{
								ImGui::Text("Elapsed Time: %um%us", time.m, time.s);
							}
							else
							{
								ImGui::Text("Elapsed Time: %us", time.s);
							}
							ImGui::NewLine();
							ImGui::Text("Handle Count: %u", data.handleCount);
							ImGui::Text("Thread Count: %u", data.threadCount);
							ImGui::NewLine();
							std::pair adjustedSize = AdjustSizeValue(data.read.bytesPerSec);
							ImGui::Text("Read Data: %.2f %s/s", adjustedSize.first, adjustedSize.second);
							ImGui::Text("Read Operations: %.2f/s", data.read.opPerSec);
							adjustedSize = AdjustSizeValue(data.write.bytesPerSec);
							ImGui::Text("Write Data: %.2f %s/s", adjustedSize.first, adjustedSize.second);
							ImGui::Text("Write Operations: %.2f/s", data.write.opPerSec);
							adjustedSize = AdjustSizeValue(data.other.bytesPerSec);
							ImGui::Text("Other Data: %.2f %s/s", adjustedSize.first, adjustedSize.second);
							ImGui::Text("Other Operations: %.2f/s", data.other.opPerSec);
							ImGui::NewLine();
							ImGui::Text("Page Faults: %.2f/s", data.pageFaultsPerSec);
							adjustedSize = AdjustSizeValue(data.privateBytes);
							ImGui::Text("Private Memory: %.2f %s", adjustedSize.first, adjustedSize.second);
							adjustedSize = AdjustSizeValue(data.workingSet);
							ImGui::Text("Working Set: %.2f %s", adjustedSize.first, adjustedSize.second);
							adjustedSize = AdjustSizeValue(data.virtualBytes);
							ImGui::Text("Virtual Memory: %.2f %s", adjustedSize.first, adjustedSize.second);

							if (ImGui::CollapsingHeader("Handles", ImGuiTreeNodeFlags_DefaultOpen))
							{
								ImGui::AlignTextToFramePadding();
								ImGui::TextUnformatted("Type");
								ImGui::SameLine();
								ImGui::SetNextItemWidth(320);
								static std::string typeFilterPreview = "All";
								static uint32_t    typeMask = std::numeric_limits<uint32_t>::max();
								if (ImGui::BeginCombo("##Type", typeFilterPreview.c_str()))
								{
									if (ImGui::Button("All"))
									{
										typeMask = std::numeric_limits<uint32_t>::max();
										typeFilterPreview = "All";
									}
									ImGui::SameLine();
									if (ImGui::Button("None"))
									{
										typeMask = 0;
										typeFilterPreview = "None";
									}
									ImGui::Separator();

									for (uint8_t index = 0; index < (uint8_t)Handle::Type::Count; ++index)
									{
										bool checked = typeMask & (1 << index);
										if (ImGui::Checkbox(Handle::typeLabels[index], &checked))
										{
											if (checked)
											{
												typeMask |= (1 << index);
											}
											else
											{
												typeMask &= ~(1 << index);
											}
											typeFilterPreview = "Mix";
										}
									}

									ImGui::EndCombo();
								}

								ImGui::SameLine();

								ImGui::AlignTextToFramePadding();
								ImGui::TextUnformatted("Info");
								ImGui::SameLine();
								ImGui::SetNextItemWidth(320);
								static char infoBuffer[2048] = {'\0'};
								ImGui::InputText("##Info", infoBuffer, sizeof(infoBuffer));

								ImGui::SameLine();

								bool forceHandleSort = false;
								if (ImGui::Button("Refresh"))
								{
									for (Handle* handle : process->handles)
									{
										delete handle;
									}
									process->handles.clear();

									HandleQuery::GenerateHandles(process->pid, process->handles);
									forceHandleSort = true;
								}

								if (ImGui::BeginTable("Handles", 3,
								                      ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV |
								                          ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
								{
									ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_WidthFixed);
									ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort);
									ImGui::TableSetupColumn("Info", ImGuiTableColumnFlags_WidthStretch);
									ImGui::TableHeadersRow();

									if (ImGui::TableGetSortSpecs()->SpecsDirty || forceHandleSort)
									{
										ImGui::TableGetSortSpecs()->SpecsDirty = false;

										bool ascending = ImGui::TableGetSortSpecs()->Specs[0].SortDirection == ImGuiSortDirection_Ascending;
										if (ImGui::TableGetSortSpecs()->Specs[0].ColumnIndex == 0)
										{
											std::sort(process->handles.begin(), process->handles.end(), [ascending](Handle* a, Handle* b) { return (a->id < b->id) == ascending; });
										}
										else if (ImGui::TableGetSortSpecs()->Specs[0].ColumnIndex == 1)
										{
											std::sort(process->handles.begin(), process->handles.end(),
											          [ascending](Handle* a, Handle* b) { return (a->type < b->type) == ascending; });
										}
										else if (ImGui::TableGetSortSpecs()->Specs[0].ColumnIndex == 2)
										{
											std::sort(process->handles.begin(), process->handles.end(),
											          [ascending](Handle* a, Handle* b) { return (a->info < b->info) == ascending; });
										}
									}

									for (Handle* handle : process->handles)
									{
										if ((typeMask & (1 << (uint32_t)handle->type)) == 0)
										{
											continue;
										}

										if (infoBuffer[0] != '\0' && handle->info.find(infoBuffer) == std::string::npos)
										{
											continue;
										}

										ImGui::TableNextRow();

										ImGui::TableNextColumn();
										ImGui::Text("%jX", (uint64_t)handle->id);

										ImGui::TableNextColumn();
										ImGui::TextUnformatted(Handle::typeLabels[(uint8_t)handle->type]);

										ImGui::TableNextColumn();
										ImGui::TextUnformatted(handle->info.c_str());
									}

									ImGui::EndTable();
								}
							}
						}
						ImGui::EndChild();
						ImGui::EndTabItem();
					}
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

	return 0;
}
