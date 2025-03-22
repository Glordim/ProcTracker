#include "Pch.hpp"
#include "ProcessDrawer.hpp"

#include "DearImGui/imgui.h"
#include "DearImGui/ImPlot/implot.h"
#include "Font/IconsMaterialDesignIcons.h"
#include "Handle/Handle.hpp"
#include "Handle/HandleQuery.hpp"
#include "OS.hpp"
#include "Process.hpp"

#include <algorithm>
#include <format>

#undef min
#undef max

template <typename _RingBufferType_>
struct RingBufferView
{
	RingBufferView(const _RingBufferType_& ringBuffer, uint32_t offset)
	: ringBuffer(ringBuffer)
	, offset(offset)
	{}

	const _RingBufferType_& ringBuffer;
	uint32_t                offset;
};

ImPlotPoint ImPlotRingBufferGetterFloat(int index, void* user_data)
{
	RingBufferView<RingBuffer<float>>* ringBufferView = static_cast<RingBufferView<RingBuffer<float>>*>(user_data);
	uint32_t                           offsetIndex = index + ringBufferView->offset;
	return ImPlotPoint(index, ringBufferView->ringBuffer.GetRawData()[ringBufferView->ringBuffer.GetRawIndex(offsetIndex)]);
}

ImPlotPoint ImPlotRingBufferGetterFloatZeroY(int index, void* user_data)
{
	return ImPlotPoint(index, 0.0f);
}

ImPlotPoint ImPlotRingBufferGetterUInt64(int index, void* user_data)
{
	RingBufferView<RingBuffer<uint64_t>>* ringBufferView = static_cast<RingBufferView<RingBuffer<uint64_t>>*>(user_data);
	uint32_t                              offsetIndex = index + ringBufferView->offset;
	return ImPlotPoint(index, ringBufferView->ringBuffer.GetRawData()[ringBufferView->ringBuffer.GetRawIndex(offsetIndex)]);
}

ImPlotPoint ImPlotRingBufferGetterUInt64ZeroY(int index, void* user_data)
{
	return ImPlotPoint(index, 0.0f);
}

int SizeFormatter(double value, char* buff, int size, void* user_data)
{
	std::pair<double, const char*> adjustedSize = AdjustSizeValue(value);
	return snprintf(buff, size, "%.2f %s", adjustedSize.first, adjustedSize.second);
}

SystemSpecs ProcessDrawer::_specs;

ProcessDrawer::ProcessDrawer(Process& process)
: _process(process)
, _cpuUsageRingBuffer(256, 0)
, _memoryUsageRingBuffer(256, 0)
, _ioReadUsageRingBuffer(256, 0)
, _ioWriteUsageRingBuffer(256, 0)
, _netDownUsageRingBuffer(256, 0)
, _netUpUsageRingBuffer(256, 0)
, _title(std::format("{}", process.pid))
, _performanceQuery(process)
{}

void ProcessDrawer::Update()
{
	_data = _performanceQuery.Retrieve(_specs);
	_cpuUsageRingBuffer.PushBack(_data.cpuUsage);
	_memoryUsageRingBuffer.PushBack(_data.privateBytes);
	if (_data.privateBytes > _memoryMax)
	{
		_memoryMax = _data.privateBytes;
	}

	_ioReadUsageRingBuffer.PushBack(_data.read.bytes);
	if (_data.read.bytes > _ioMax)
	{
		_ioMax = _data.read.bytes;
	}
	_ioWriteUsageRingBuffer.PushBack(_data.write.bytes);
	if (_data.write.bytes > _ioMax)
	{
		_ioMax = _data.write.bytes;
	}

	_netDownUsageRingBuffer.PushBack(_data.netDown);
	if (_data.read.bytes > _netMax)
	{
		_netMax = _data.read.bytes;
	}
	_netUpUsageRingBuffer.PushBack(_data.netUp);
	if (_data.write.bytes > _netMax)
	{
		_netMax = _data.write.bytes;
	}
}

bool ProcessDrawer::Draw()
{
	if (ImGui::BeginTabItem(_title.data()))
	{
		if (ImGui::Button(ICON_MDI_SKULL_CROSSBONES " Kill me !"))
		{
			OS::KillProcess(_process.pid);
		}
		ImGui::SameLine(0.0, 15.0f);
		ImGui::Text("CPU:  %.2f%%", _data.cpuUsage);
		ImGui::SameLine(0.0, 15.0f);
		std::pair<double, const char*> adjustedSize = AdjustSizeValue(_data.privateBytes);
		ImGui::Text("RAM:  %.2f %s", adjustedSize.first, adjustedSize.second);

		uint32_t nbSamplesToDisplay = std::min(_cpuUsageRingBuffer.GetSize(), (uint32_t)ImGui::GetWindowWidth() / 5);

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
					ImPlot::SetNextAxisLimits(ImAxis_X1, 0, nbSamplesToDisplay, ImGuiCond_Always);
					ImPlot::SetNextAxisLimits(ImAxis_Y1, 0.0f, 100.0f, ImGuiCond_Always);
					if (ImPlot::BeginPlot("##CPU_Plot", ImVec2(-1, 200), ImPlotFlags_NoFrame))
					{
						ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, 0);
						RingBufferView ringBufferView(_cpuUsageRingBuffer, _cpuUsageRingBuffer.GetSize() - nbSamplesToDisplay);
						ImPlot::PlotShadedG("CPU (%)", &ImPlotRingBufferGetterFloat, &ringBufferView, &ImPlotRingBufferGetterFloatZeroY, nullptr, _cpuUsageRingBuffer.GetSize());
						ImPlot::EndPlot();
					}
				}
			}
			ImGui::EndChild();

			if (ImGui::BeginChild("CollapsablePlotWindowMemory", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_None))
			{
				ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered]);
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive]);
				bool opened = ImGui::CollapsingHeader("Memory", ImGuiTreeNodeFlags_DefaultOpen);
				ImGui::PopStyleColor(3);
				if (opened)
				{
					ImPlot::SetNextAxisLimits(ImAxis_X1, 0, nbSamplesToDisplay, ImGuiCond_Always);
					ImPlot::SetNextAxisLimits(ImAxis_Y1, 0.0f, (double)_memoryMax * 1.3f, ImGuiCond_Always);
					if (ImPlot::BeginPlot("##Memory_Plot", ImVec2(-1, 200), ImPlotFlags_NoFrame))
					{
						ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, 0);
						ImPlot::SetupMouseText(ImPlotLocation_SouthEast, ImPlotMouseTextFlags_NoAuxAxes);
						ImPlot::SetupAxisFormat(ImAxis_Y1, &SizeFormatter);
						RingBufferView ringBufferView(_memoryUsageRingBuffer, _memoryUsageRingBuffer.GetSize() - nbSamplesToDisplay);
						ImPlot::PlotShadedG("Memory", &ImPlotRingBufferGetterUInt64, &ringBufferView, &ImPlotRingBufferGetterUInt64ZeroY, nullptr,
						                    _memoryUsageRingBuffer.GetSize());
						ImPlot::EndPlot();
					}
				}
			}
			ImGui::EndChild();

			if (ImGui::BeginChild("CollapsablePlotWindowIo", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_None))
			{
				ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered]);
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive]);
				bool opened = ImGui::CollapsingHeader("IO", ImGuiTreeNodeFlags_DefaultOpen);
				ImGui::PopStyleColor(3);
				if (opened)
				{
					ImPlot::SetNextAxisLimits(ImAxis_X1, 0, nbSamplesToDisplay, ImGuiCond_Always);
					ImPlot::SetNextAxisLimits(ImAxis_Y1, 0.0f, (double)_ioMax * 1.3f, ImGuiCond_Always);
					if (ImPlot::BeginPlot("##IO_Plot", ImVec2(-1, 200), ImPlotFlags_NoFrame))
					{
						ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, 0);
						ImPlot::SetupMouseText(ImPlotLocation_SouthEast, ImPlotMouseTextFlags_NoAuxAxes);
						ImPlot::SetupAxisFormat(ImAxis_Y1, &SizeFormatter);
						RingBufferView readRingBufferView(_ioReadUsageRingBuffer, _ioReadUsageRingBuffer.GetSize() - nbSamplesToDisplay);
						ImPlot::PlotShadedG("Read", &ImPlotRingBufferGetterUInt64, &readRingBufferView, &ImPlotRingBufferGetterUInt64ZeroY, nullptr,
						                    _ioReadUsageRingBuffer.GetSize());
						RingBufferView writeRingBufferView(_ioWriteUsageRingBuffer, _ioWriteUsageRingBuffer.GetSize() - nbSamplesToDisplay);
						ImPlot::PlotShadedG("Write", &ImPlotRingBufferGetterUInt64, &writeRingBufferView, &ImPlotRingBufferGetterUInt64ZeroY, nullptr,
						                    _ioWriteUsageRingBuffer.GetSize());
						ImPlot::EndPlot();
					}
				}
			}
			ImGui::EndChild();

			if (ImGui::BeginChild("CollapsablePlotWindowNet", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_None))
			{
				ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered]);
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive]);
				bool opened = ImGui::CollapsingHeader("Network", ImGuiTreeNodeFlags_DefaultOpen);
				ImGui::PopStyleColor(3);
				if (opened)
				{
					ImPlot::SetNextAxisLimits(ImAxis_X1, 0, nbSamplesToDisplay, ImGuiCond_Always);
					ImPlot::SetNextAxisLimits(ImAxis_Y1, 0.0f, (double)_netMax * 1.3f, ImGuiCond_Always);
					if (ImPlot::BeginPlot("##Net_Plot", ImVec2(-1, 200), ImPlotFlags_NoFrame))
					{
						ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, 0);
						ImPlot::SetupMouseText(ImPlotLocation_SouthEast, ImPlotMouseTextFlags_NoAuxAxes);
						ImPlot::SetupAxisFormat(ImAxis_Y1, &SizeFormatter);
						RingBufferView readRingBufferView(_netDownUsageRingBuffer, _netDownUsageRingBuffer.GetSize() - nbSamplesToDisplay);
						ImPlot::PlotShadedG("Down", &ImPlotRingBufferGetterUInt64, &readRingBufferView, &ImPlotRingBufferGetterUInt64ZeroY, nullptr,
						                    _netDownUsageRingBuffer.GetSize());
						RingBufferView writeRingBufferView(_netUpUsageRingBuffer, _netUpUsageRingBuffer.GetSize() - nbSamplesToDisplay);
						ImPlot::PlotShadedG("Up", &ImPlotRingBufferGetterUInt64, &writeRingBufferView, &ImPlotRingBufferGetterUInt64ZeroY, nullptr,
						                    _netUpUsageRingBuffer.GetSize());
						ImPlot::EndPlot();
					}
				}
			}
			ImGui::EndChild();

			ImGui::Text("CPU: %.2f%%", _data.cpuUsage);
			Time time = AdjustTimeValue(_data.time);
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
			ImGui::Text("Thread Count: %u", _data.threadCount);
			ImGui::NewLine();
			std::pair adjustedSize = AdjustSizeValue(_data.read.bytes);
			ImGui::Text("Read Data: %.2f %s/s", adjustedSize.first, adjustedSize.second);
			ImGui::Text("Read Operations: %ju/s", _data.read.op);
			adjustedSize = AdjustSizeValue(_data.write.bytes);
			ImGui::Text("Write Data: %.2f %s/s", adjustedSize.first, adjustedSize.second);
			ImGui::Text("Write Operations: %juf/s", _data.write.op);
			adjustedSize = AdjustSizeValue(_data.other.bytes);
			ImGui::Text("Other Data: %.2f %s/s", adjustedSize.first, adjustedSize.second);
			ImGui::Text("Other Operations: %juf/s", _data.other.op);
			ImGui::NewLine();
			// ImGui::Text("Page Faults: %.2f/s", _data.pageFaultsPerSec);
			adjustedSize = AdjustSizeValue(_data.privateBytes);
			ImGui::Text("Private Memory: %.2f %s", adjustedSize.first, adjustedSize.second);
			adjustedSize = AdjustSizeValue(_data.workingSet);
			ImGui::Text("Working Set: %.2f %s", adjustedSize.first, adjustedSize.second);
			adjustedSize = AdjustSizeValue(_data.virtualBytes);
			ImGui::Text("Virtual Memory: %.2f %s", adjustedSize.first, adjustedSize.second);
			ImGui::NewLine();
			adjustedSize = AdjustSizeValue(_data.netDown);
			ImGui::Text("Network Down: %.2f %s", adjustedSize.first, adjustedSize.second);
			adjustedSize = AdjustSizeValue(_data.netUp);
			ImGui::Text("Network Up: %.2f %s", adjustedSize.first, adjustedSize.second);

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
					for (Handle* handle : _process.handles)
					{
						delete handle;
					}
					_process.handles.clear();

					HandleQuery::GenerateHandles(_process.pid, _process.handles);
					forceHandleSort = true;
				}

				if (ImGui::BeginTable("Handles", 3,
				                      ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg |
				                          ImGuiTableFlags_ScrollY))
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
							std::sort(_process.handles.begin(), _process.handles.end(), [ascending](Handle* a, Handle* b) { return (a->id < b->id) == ascending; });
						}
						else if (ImGui::TableGetSortSpecs()->Specs[0].ColumnIndex == 1)
						{
							std::sort(_process.handles.begin(), _process.handles.end(), [ascending](Handle* a, Handle* b) { return (a->type < b->type) == ascending; });
						}
						else if (ImGui::TableGetSortSpecs()->Specs[0].ColumnIndex == 2)
						{
							std::sort(_process.handles.begin(), _process.handles.end(), [ascending](Handle* a, Handle* b) { return (a->info < b->info) == ascending; });
						}
					}

					for (Handle* handle : _process.handles)
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

	return true;
}
