// ImGui Win32 + DirectX12 binding
// https://github.com/ocornut/imgui

struct ID3D12Device;
struct ID3D12DeviceContext;
struct ID3D12DescriptorHeap;
struct ID3D12GraphicsCommandList;
struct ID3D12CommandQueue;

bool        ImGui_ImplDX12_Init(void* hwnd, ID3D12Device* device, ID3D12CommandQueue* pCmdQueue, ID3D12GraphicsCommandList* pCmdList, ID3D12DescriptorHeap* descHeap, unsigned int descHeapBaseOffset);
void        ImGui_ImplDX12_Shutdown();
void        ImGui_ImplDX12_NewFrame();

// Use if you want to reset your rendering device without losing ImGui state.
void        ImGui_ImplDX12_InvalidateDeviceObjects();
bool        ImGui_ImplDX12_CreateDeviceObjects();

// Handler for Win32 messages, update mouse/keyboard data.
// You may or not need this for your implementation, but it can serve as reference for handling inputs.
// Commented out to avoid dragging dependencies on <windows.h> types. You can copy the extern declaration in your code.
/*
extern LRESULT   ImGui_ImplDX12_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
*/
