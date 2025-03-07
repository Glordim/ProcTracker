#include "Pch.hpp"

#include "DearImGui/imgui.h"
#include "DearImGui/ImPlot/implot.h"
#include "DearImGui/backends/imgui_impl_sdl3.h"
#include "DearImGui/backends/imgui_impl_sdlgpu3.h"
#include <SDL3/SDL.h>

#include "Font/Roboto-Regular.ttf.hpp"
#include "Font/MaterialDesignIcons.ttf.hpp"
#include "Font/IconsMaterialDesignIcons.h"

#include "OS.hpp"
#include "SystemWatcher.hpp"
#include "Process.hpp"

#include <functional>

int	main(int argc, char** argv)
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
	robotoFontConfig.SizePixels = 13.0f;
	robotoFontConfig.OversampleH = 1;
	robotoFontConfig.OversampleV = 1;
	robotoFontConfig.PixelSnapH = true;
	io.Fonts->AddFontFromMemoryTTF(Roboto_Regular_ttf, Roboto_Regular_ttf_size, 16.0f, &robotoFontConfig);
	io.Fonts->Build();

	const ImWchar materialDesignIconFontRanges[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };
	ImFontConfig materialDesignIconFontConfig;
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

	static char processName[4096] = { '\0' };
	std::strcpy(processName, "ProcTracker");

	std::vector<Process*> processes;
	std::function<void(uint64_t)> onProcessCreated([&processes](uint64_t pid)
	{
		processes.push_back(new Process(pid));
	});
	std::function<void(uint64_t)> onProcessTerminated([&processes](uint64_t pid)
	{
		for (size_t i = 0; i < processes.size(); ++i)
		{
			Process* process = processes[i];
			if (process->GetPid() == pid)
			{
				processes.erase(processes.begin() + i);
				delete process;
				break;
			}
		}
	});
	systemWatcher.StartWatch(processName, onProcessCreated, onProcessTerminated);

	bool exit = false;
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

		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
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
				systemWatcher.StartWatch(processName, onProcessCreated, onProcessTerminated);
			}

			for (Process* process : processes)
			{
				ImGui::Text("%ji", process->GetPid());
			}
		}
		ImGui::End();

		static bool show_demo_window = true;
		ImGui::ShowDemoWindow(&show_demo_window);

		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
		const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);

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
			target_info.clear_color = SDL_FColor { 0.0f, 0.0f, 0.0f, 1.0f };
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
