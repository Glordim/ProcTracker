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
#include "SystemWatcher.hpp"

#include <algorithm>
#include <format>
#include <functional>
#include <mutex>

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
	SDL_SetGPUSwapchainParameters(gpu_device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_MAILBOX);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

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
			ImGui::SetNextItemWidth(100.f);
			ImGui::SliderFloat("Refresh Rate", &maxTimer, 0.01f, 2.f, "%.2fs");

			ImGui::Checkbox("Show Demo", &show_demo_window);

			ImGui::EndMainMenuBar();
		}

		ImVec2 pos(0, ImGui::GetFrameHeight());
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size - pos);
		if (ImGui::Begin("Tmp", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize))
		{
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Process Name");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-1);
			if (ImGui::InputText("##ProcessName", processName, sizeof(processName), ImGuiInputTextFlags_EnterReturnsTrue))
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

			if (ImGui::BeginTabBar("##processTabBar"))
			{
				processesLock.lock();
				for (uint32_t i {0}; i < processes.size(); ++i)
				{
					if (ImGui::BeginTabItem(std::format("{}", processes[i]->pid).data()))
					{
						static PerformanceSnapshot data;
						if (update)
						{
							data = queries[i].Retrieve();
						}

						ImGui::Text("CPU: %.2f%%", data.cpuUsage);
						ImGui::Text("Elapsed Time: %.2f s", data.time);
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
