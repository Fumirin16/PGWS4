// �|���S���\��
#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <vector>
#include <d3dcompiler.h>
#include <DirectXTex.h>
#ifdef _DEBUG
#include <iostream>
#endif

#pragma comment(lib,"DirectXTex.lib")
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace std;
using namespace DirectX;

// @bref �R���\�[����ʂɃt�H�[�}�b�g�t���������\��
// @param format �t�H�[�}�b�g�i%d �Ƃ� %f�@�Ƃ��́j
// @param �ϒ�����
// @remarks ���̊֐��̓f�o�b�O�p�B�f�o�b�O���������삵�Ȃ�
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif
}

// OS�̃C�x���g�ɉ������鏈��
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// �E�B���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); // OS�ɑ΂��āu�������̃A�v���͏I���v�Ɠ`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam); // ����̏������s��
}

const unsigned int window_width = 1280;
const unsigned int window_height = 720;

///�A���C�����g�ɑ������T�C�Y��Ԃ�
///@param size ���̃T�C�Y
///@param alignment �A���C�����g�T�C�Y
///@return �A���C�����g�����낦���T�C�Y
size_t AlignmentedSize(size_t size, size_t alignment) 
{
	return size + alignment - size % alignment;
}

ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
ID3D12CommandAllocator* _cmdAllocator = nullptr;
ID3D12GraphicsCommandList* _cmdList = nullptr;
ID3D12CommandQueue* _cmdQueue = nullptr;
IDXGISwapChain4* _swapchain = nullptr;


void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	if (!SUCCEEDED(result)) return;

	debugLayer->EnableDebugLayer(); // �f�o�b�O���C���[��L��������
	debugLayer->Release(); // �L����������C���^�[�t�F�C�X���������
}

#ifdef _DEBUG
int main()
{
#else
#include<Windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#endif

	// �E�B���h�E�N���X�̐������o�^
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure; // �R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T("DX12Sample"); // �A�v���P�[�V�����N���X���i�K���ł����j
	w.hInstance = GetModuleHandle(nullptr); // �n���h���̎擾

	RegisterClassEx(&w); // �E�B���h�E�T�C�Y�����߂�

	RECT wrc = { 0, 0, window_width, window_height };

	// �֐����g���ăE�B���h�E�̃T�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	// �E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName, // �N���X���w��
		_T("DX12 �e�X�g "), // �^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW, // �^�C�g���o�[�Ƌ��E��������E�B���h�E
		CW_USEDEFAULT, // �\�������W�͂n�r�ɂ��C��
		CW_USEDEFAULT, // �\�������W�͂n�r�ɂ��C��
		wrc.right - wrc.left, // �E�B���h�E��
		wrc.bottom - wrc.top, // �E�B���h�E��
		nullptr, // �e�E�B���h�E�n���h��
		nullptr, // ���j���[�n���h��
		w.hInstance, // �Ăяo���A�v���P�[�V�����n���h��
		nullptr); // �ǉ��p�����[�^�[

	// �����ŁuS_OK�v�A���s���Ă�����҃R�[�h���Ԃ��Ă���
	HRESULT D3D12CreteDevice(
		IUnknown * pAdapter, // �ЂƂ܂���nillptr��OK�B�����ŃO���t�B�b�N�X�h���C�o�[��I��
		D3D_FEATURE_LEVEL MinimumFeatureLevel, // �Œ���K�v�ȃt�B�[�`���[���x���B�I�������h���C�o�[���Ή����Ă��Ȃ���΁A�Ăяo���͎��s����B�f�o�C�X�Ăяo�������s������t�B�[�`���[���x���������邩�A�ʂ̃h���C�o�[���w�肷��B
		REFIID riid, // ��q�B�󂯎�肽���I�u�W�F�N�g�̌^�����ʂ��邽�߂�ID
		void** ppDevice // ��q
	);

#ifdef _DEBUG
	// �f�o�b�O���C���[���I����
	EnableDebugLayer();
#endif
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

#ifdef _DEBUG
	auto result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
#else
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif
	// �A�_�v�^�[�̗񋓗p
	vector <IDXGIAdapter*> adapters;
	// �����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0;
		_dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc); // �A�_�v�^�[�̐����I�u�W�F�N�g�擾
		wstring strDesc = adesc.Description;
		// �T�������A�_�v�^�[�̖��O���m�F
		if (strDesc.find(L"NVIDIA") != string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}
	// Direct3D �f�o�C�X�̏�����
	D3D_FEATURE_LEVEL featureLevel;
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featureLevel = lv;
			break; // �����\�ȃo�[�W���������������烋�[�v��ł��؂�
			// �S�Ẵ��x���Ŏ��s�����ꍇ�̏��u���Ȃ�
		}
	}

	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&_cmdAllocator));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator, nullptr,
		IID_PPV_ARGS(&_cmdList));

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	// �^�C���A�E�g�Ȃ�
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	// �A�_�v�^�[���P�����g��Ȃ��Ƃ��͂O�ł���
	cmdQueueDesc.NodeMask = 0;
	// �v���C�I���e�B�͓��Ɏw��Ȃ�
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	// �R�}���h���X�g�ƍ��킹��
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	// �L���[����
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	// �o�b�N�o�b�t�@�[�͐L�яk�݉\
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	// �t���b�v��͑��₩�ɔj��
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	// ���Ɏw��Ȃ�
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	// �E�B���h�E�ƃt���X�N���[���؂�ւ��\
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue, hwnd,
		&swapchainDesc, nullptr, nullptr,
		(IDXGISwapChain1**)&_swapchain); // �{����QueryInterface�Ȃǂ�p����
	// IDXGISwapChain4*�ւ̕ϊ��`�F�b�N�����邪�A�����ł͂킩��₷���d���̂��߃L���X�g�őΉ�
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // ��m�_�[�^�[�Q�b�g�r���[�Ȃ̂�RTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2; // ���\�̂Q��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // ���ɂȂ�

	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);

	// SRGB �����_�[�^�[�Q�b�g�r���[�̐ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // �K���}�␳����(sRGB)
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
		D3D12_CPU_DESCRIPTOR_HANDLE handle
			= rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_dev->CreateRenderTargetView(_backBuffers[idx], &rtvDesc, handle);
	}

	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	// �E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);

	// ���_�f�[�^�\����
	struct Vertex {
		XMFLOAT3 pos; // xyz���W
		XMFLOAT2 uv; // uv���W
	};

	// ���_�̏������f�[�^
	Vertex vertices[] = {
		{{-1.0f, -1.0f, 0.0f},{0.0f, 1.0f}}, // ����
		{{-1.0f, +1.0f, 0.0f},{0.0f, 0.0f}}, // ����
		{{+1.0f, -1.0f, 0.0f},{1.0f, 1.0f}}, // �E��
		{{+1.0f, +1.0f, 0.0f},{1.0f, 0.0f}}, // �E��
	};

	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices); // ���_��񂪓��邾���̃T�C�Y
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// ���_�C���f�b�N�X
	unsigned short indices[] = {
		0, 1, 2,
		2, 1, 3
	};

	// �C���e�b�N�X�o�b�t�@�̍쐬
	ID3D12Resource* idxBuff = nullptr;
	// �ݒ�́A�o�b�t�@�[�̃T�C�Y�ȊO�A���_�o�b�t�@�[�̐ݒ���g���܂킵�Ă���
	resdesc.Width = sizeof(indices);
	result = _dev->CreateCommittedResource(
		&heapprop, D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&idxBuff));

	// ������o�b�t�@�[�ɃC���f�b�N�X�f�[�^���R�s�[
	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	// �C���f�b�N�X�o�b�t�@�[�r���[���쐬
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);


	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	// ���_�V�F�[�_�[�̃R���p�C��
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl", // �V�F�[�_�[��
		nullptr, // define�͖���
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�̓f�t�H���g
		"BasicVS", "vs_5_0", // �֐���BasicVS�A�ΏۃV�F�[�_�[��vs_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�y�эœK���Ȃ�
		0,
		&_vsBlob, &errorBlob); // �G���[����errorBlob�Ƀ��b�Z�[�W������
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�t�@�C������������܂���");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

	//�s�N�Z���V�F�[�_�[�̃R���p�C��
	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl", // �V�F�[�_�[��
		nullptr, // define�͖���
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�̓f�t�H���g
		"BasicPS", "ps_5_0", // �֐���BasicVS�A�ΏۃV�F�[�_�[��vs_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�y�эœK���Ȃ�
		0,
		&_psBlob, &errorBlob); // �G���[����errorBlob�Ƀ��b�Z�[�W������
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�t�@�C������������܂���");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

	// ���C�A�E�g����uv����ǉ�����
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{// uv�i�ǉ��j
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	gpipeline.pRootSignature = nullptr; // ���ƂŐݒ肷��

	gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();
	gpipeline.InputLayout.pInputElementDescs = inputLayout; // ���C�A�E�g�擪�A�h���X
	gpipeline.InputLayout.NumElements = _countof(inputLayout); // ���C�A�E�g�z��

	// �f�t�H���g�̃T���v���}�X�N��\���萔�i0xffffffff�j
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// �܂��A���`�G�C���A�X�͎g��Ȃ�����false
	gpipeline.RasterizerState.MultisampleEnable = false;

	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // �J�����O���Ȃ�
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // ���g��h��Ԃ�
	gpipeline.RasterizerState.DepthClipEnable = true; // �[�x�����̃N���b�s���O�͗L����
	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	// �ЂƂ܂����Z���Z�⃿�u�����f�B���O�͎g�p���Ȃ�
	renderTargetBlendDesc.BlendEnable = false;
	//
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	gpipeline.InputLayout.pInputElementDescs = inputLayout; // ���C�A�E�g�擪�A�h���X
	gpipeline.InputLayout.NumElements = _countof(inputLayout); // ���C�A�E�g�z��̗v�f��

	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED; // �X�v���b�g���̃J�b�g�Ȃ�
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // �O�p�`�ō\��

	gpipeline.NumRenderTargets = 1; // ����͂P�̂�
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 0~1�ɐ��K�����ꂽRGBA

	gpipeline.SampleDesc.Count = 1; // �T���v�����O�͂P�s�N�Z���ɂ��P
	gpipeline.SampleDesc.Quality = 0; // �N�I���e�B�͍Œ�

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// �f�B�X�N�v���^�����W
	D3D12_DESCRIPTOR_RANGE descTblRange[2] = {}; // �e�N�X�`���ƒ萔�̂Q��

	// �e�N�X�`���p���W�X�^�[�O��
	descTblRange[0].NumDescriptors = 1; // �e�N�X�`���P��
	descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // ��ʂ̓e�N�X�`��
	descTblRange[0].BaseShaderRegister = 0; // 0�ԃX���b�g����
	descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// �萔�p���W�X�^�[�P��
	descTblRange[1].NumDescriptors = 1; // �萔�P��
	descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; // ��ʂ͒萔
	descTblRange[1].BaseShaderRegister = 0; // 0�ԃX���b�g����
	descTblRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ���[�g�p�����[�^�[�̍쐬
	D3D12_ROOT_PARAMETER rootparam = {};
	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	// �f�B�X�N�v���^�����W�̃A�h���X
	rootparam.DescriptorTable.pDescriptorRanges = descTblRange;
	//�f�X�N���v�^�����W��
	rootparam.DescriptorTable.NumDescriptorRanges = 2;
	// �s�N�Z���V�F�[�_�[���猩����
	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// ���[�g�V�O�l�`���ւ̒ǉ�
	rootSignatureDesc.pParameters = &rootparam; // ���[�g�p�����[�^�̐擪�A�h���X
	rootSignatureDesc.NumParameters = 1; // ���[�g�p�����[�^��

	// �T���v���[�̐ݒ�
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // ���J��Ԃ�
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // �c�J��Ԃ�
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // ���s�J��Ԃ�
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK; // �{�[�_�[�͍�
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // ��Ԃ��Ȃ�(�j�A���X�g�l�C�o�[)
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX; // �~�b�v�}�b�v�ő�l
	samplerDesc.MinLOD = 0.0f; // �~�b�v�}�b�v�ŏ��l
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // �s�N�Z���V�F�[�_����̂݉�
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // �I�[�o�[�T���v�����O�̍ۃ��T���v�����O���Ȃ��H

	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	ID3DBlob* rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(
		&rootSignatureDesc, //
		D3D_ROOT_SIGNATURE_VERSION_1_0, // 
		&rootSigBlob, // 
		&errorBlob); //

	ID3D12RootSignature* rootsignature = nullptr;
	result = _dev->CreateRootSignature(
		0, //
		rootSigBlob->GetBufferPointer(),//
		rootSigBlob->GetBufferSize(), //
		IID_PPV_ARGS(&rootsignature));//

	gpipeline.pRootSignature = rootsignature;

	ID3D12PipelineState* _pipelinestate = nullptr;
	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));

	D3D12_VIEWPORT viewport = {};
	viewport.Width = window_width;
	viewport.Height = window_height;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;

	D3D12_RECT scissorrect = {};
	scissorrect.top = 0;
	scissorrect.left = 0;
	scissorrect.right = scissorrect.left + window_width;
	scissorrect.bottom = scissorrect.top + window_height;

	//WIC�e�N�X�`���̃��[�h
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	result = LoadFromWICFile(L"img/textest.png", WIC_FLAGS_NONE, &metadata, scratchImg);
	auto img = scratchImg.GetImage(0, 0, 0);//���f�[�^���o

	//�v���V�[�W�����e�N�X�`��
	//struct TexRGBA 
	//{
	//	unsigned char R, G, B, A;
	//};
	//vector<TexRGBA> texturedata(256 * 256);
	//for (auto& rgba : texturedata) 
	//{
	//	rgba.R = rand() % 256;
	//	rgba.G = rand() % 256;
	//	rgba.B = rand() % 256;
	//	rgba.A = 255;//�A���t�@��1.0
	//}

	//�܂��͒��ԃo�b�t�@�Ƃ��Ă�Upload�q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES uploadHeapProp = {};
	// �}�b�v�\�ɂ��邽�߁AUPLOAD�ɂ���
	uploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	// �A�b�v���[�h�p�Ɏg�p���邱�ƑO��Ȃ̂�UNKNOWN�ł���
	uploadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	uploadHeapProp.CreationNodeMask = 0;//�P��A�_�v�^�̂���0
	uploadHeapProp.VisibleNodeMask = 0;//�P��A�_�v�^�̂���0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_UNKNOWN; // �P�Ȃ�f�[�^�̉�Ȃ̂�UNKNOWN
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // �P�Ȃ�o�b�t�@�Ƃ��Ďw��

	resDesc.Width = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * img->height; // �f�[�^�T�C�Y
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;

	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // �A�������f�[�^
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE; // �Ƃ��Ƀt���O�Ȃ�

	resDesc.SampleDesc.Count = 1; // �ʏ�e�N�X�`���Ȃ̂ŃA���`�G�C���A�V���O���Ȃ����Ȃ�
	resDesc.SampleDesc.Quality = 0;

	// ���ԃo�b�t�@�[�쐬
	ID3D12Resource* uploadbuff = nullptr;
	result = _dev->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,//���Ɏw��Ȃ�
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,//CPU���珑�����݉\
		nullptr,
		IID_PPV_ARGS(&uploadbuff)
	);

	//WriteToSubresource�œ]������p�̃q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	// ����Ȑݒ�Ȃ̂�DEFAULT�ł�UPLOAD�ł��Ȃ�
	texHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	//���C�g�o�b�N
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	// �]����LO�A�܂�CPU�����璼�ڍs��
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	//�P��A�_�v�^�̂���0
	texHeapProp.CreationNodeMask = 0;
	texHeapProp.VisibleNodeMask = 0;

	// ���\�[�X�̐ݒ�(���^�f�[�^�̗��p)
	D3D12_RESOURCE_DESC resDesc2 = {};
	resDesc2.Format = metadata.format; // RGBA�t�H�[�}�b�g
	resDesc2.Width = static_cast<UINT>(metadata.width); // ��
	resDesc2.Height = static_cast<UINT>(metadata.height); // ����
	resDesc2.DepthOrArraySize = static_cast<uint16_t>(metadata.arraySize); // 2D�Ŕz��ł��Ȃ��̂łP
	resDesc2.SampleDesc.Count = 1; // �ʏ�e�N�X�`���Ȃ̂ŃA���`�F�����Ȃ�
	resDesc2.SampleDesc.Quality = 0; // �N�I���e�B�͍Œ�
	resDesc2.MipLevels = static_cast<uint16_t>(metadata.mipLevels); // �~�b�v�}�b�v���Ȃ��̂Ń~�b�v���͂P��
	resDesc2.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension); // 2D�e�N�X�`���p
	resDesc2.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // ���C�A�E�g�ɂ��Ă͌��肵�Ȃ�
	resDesc2.Flags = D3D12_RESOURCE_FLAG_NONE; // ���Ƀt���O�Ȃ�

	// ���\�[�X�̐���
	ID3D12Resource* texbuff = nullptr;
	result = _dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE, // ���Ɏw��Ȃ�
		&resDesc2,
		D3D12_RESOURCE_STATE_COPY_DEST, // �e�N�X�`���p�w��
		nullptr,
		IID_PPV_ARGS(&texbuff));

	// image->pixels�Ɠ����^�ɂ���
	uint8_t* mapforImg = nullptr;
	// �}�b�v
	result = uploadbuff->Map(0, nullptr, (void**)&mapforImg); // �}�b�v

	auto srcAddress = img->pixels;

	auto rowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

	for (int y = 0; y < img->height; ++y) 
	{
		std::copy_n(srcAddress,
			rowPitch,
			mapforImg);//�R�s�[
		//1�s���Ƃ̒�������킹�Ă��
		srcAddress += img->rowPitch;
		mapforImg += rowPitch;
	}

	//std::copy_n(img->pixels, img->slicePitch, mapforImg);
	// �A���}�b�v
	uploadbuff->Unmap(0, nullptr);

	// WriteToSubresource���\�b�h�ɂ��f�[�^�]��
	result = texbuff->WriteToSubresource(
		0,
		nullptr, // �S�̈�փR�s�[
		img->pixels, // ���f�[�^�A�h���X
		static_cast<UINT>(img->rowPitch), // 1���C���T�C�Y
		static_cast<UINT>(img->slicePitch) // �S�T�C�Y
	);

	// �萔�o�b�t�@�[�쐬
	// Y�����S��45�x��]������
	XMMATRIX worldMat = XMMatrixRotationY(XM_PIDIV4);

	XMFLOAT3 eye(0, 0, -5);
	XMFLOAT3 target(0, 0, 0);
	XMFLOAT3 up(0, 1, 0);

	auto viewMat = XMMatrixLookAtLH(
		XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

	auto projMat = XMMatrixPerspectiveFovLH(
		XM_PIDIV2, // ��p��90��
		static_cast<float>(window_width) / static_cast<float>(window_height), // �A�X�y�N�g��
		1.0f, // �߂��ق�
		10.0f // �����ق�
	);

//	matrix.r[0].m128_f32[0] = +2.0f / window_width;
//	matrix.r[1].m128_f32[1] = -2.0f / window_height;

//	matrix.r[3].m128_f32[0] = -1.0f;
//	matrix.r[3].m128_f32[1] = +1.0f;

	ID3D12Resource* constBuff = nullptr;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(XMMATRIX) + 0xff) & ~0xff);

	result = _dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff)
	);

	// �}�b�v��������|�C���^�[
	XMMATRIX* mapMatrix;
	// �}�b�v
	result = constBuff->Map(0, nullptr, (void**)&mapMatrix);
	// �s����e���R�s�[
	//*mapMatrix = matrix;

	// �f�B�X�N���v�^�q�[�v�����
	ID3D12DescriptorHeap* basicDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	// �V�F�[�_�[���猩����悤��
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	// �}�X�N�͂O
	descHeapDesc.NodeMask = 0;
	// SRV1�gCBV1��
	descHeapDesc.NumDescriptors = 2;
	// �V�F�[�_�[���\�[�X�r���[�p
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	// ����
	result = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));

	// �V�F�[�_�[���\�[�X�r���[
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = metadata.format; // RGBA(0.0f �` 1.0f �ɐ��K��)
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // ��q
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1; // �~�b�v�}�b�v�͎g�p���Ȃ�

	// �f�X�N���v�^�̐擪�n���h�����擾���Ă���
	auto basicHeapHandle = basicDescHeap->GetCPUDescriptorHandleForHeapStart();

	_dev->CreateShaderResourceView(
		texbuff,  // �r���[�Ɗ֘A�t����o�b�t�@�[
		&srvDesc, // ��قǐݒ肵���e�N�X�`���ݒ���
		basicHeapHandle // �擪�̏ꏊ�������n���h��
	);

	// ���̏ꏊ�Ɉړ�
	basicHeapHandle.ptr +=
		_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(constBuff->GetDesc().Width);

	// �萔�o�b�t�@�[�r���[�̍쐬
	_dev->CreateConstantBufferView(&cbvDesc, basicHeapHandle);

	ID3D12Resource* vertBuff = nullptr;

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	Vertex* vertMap = nullptr; // �^��Vertex�ɕύX
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress(); // �o�b�t�@�[�̉��z�A�h���X
	vbView.SizeInBytes = sizeof(vertices); // �S�o�C�g��
	vbView.StrideInBytes = sizeof(vertices[0]); //�P���_������̃o�C�g��

	D3D12_TEXTURE_COPY_LOCATION src = {};

	// �R�s�[���i�A�b�v���[�h���j�ݒ�
	src.pResource = uploadbuff;//���ԃo�b�t�@
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;//�t�b�g�v�����g�w��
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = static_cast<UINT>(metadata.width);
	src.PlacedFootprint.Footprint.Height = static_cast<UINT>(metadata.height);
	src.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metadata.depth);
	src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
	src.PlacedFootprint.Footprint.Format = img->format;

	D3D12_TEXTURE_COPY_LOCATION dst = {};

	// �R�s�[��ݒ�
	dst.pResource = texbuff;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;
	{
	_cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = texbuff;
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	_cmdList->ResourceBarrier(1, &BarrierDesc);
	_cmdList->Close();

	//�R�}���h���X�g�̎��s
	ID3D12CommandList* cmdlists[] = { _cmdList };
	_cmdQueue->ExecuteCommandLists(1, cmdlists);

	_cmdQueue->Signal(_fence, ++_fenceVal);

	if (_fence->GetCompletedValue() != _fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		_fence->SetEventOnCompletion(_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
	_cmdAllocator->Reset();//�L���[���N���A
	_cmdList->Reset(_cmdAllocator, nullptr);
}

	float angle = 0.0f;

	// �Q�[�����[�v
	while (true)
	{
		MSG msg;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// �A�v���P�[�V�������I���Ƃ���message��WM_QUIT�ɂȂ�
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		angle += 0.1f;
		worldMat = XMMatrixRotationY(angle);
		*mapMatrix = worldMat * viewMat * projMat;

		// DirectX�̏���
		// �o�b�N�o�b�t�@�̃C���f�b�N�X���擾
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

		_cmdList->SetPipelineState(_pipelinestate);
		
		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // �J��
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE; // ���Ɏw��Ȃ�
		BarrierDesc.Transition.pResource = _backBuffers[bbIdx]; // �o�b�N�o�b�t�@�[���\�[�X
		BarrierDesc.Transition.Subresource = 0;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT; // ���O��PRESENT���
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET; // ������RT���
		_cmdList->ResourceBarrier(1, &BarrierDesc); // �o���A�w����s

		// �����_�[�^�[�Q�b�g���w��
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

		// ��ʃN���A
		float clearColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
		//_color = (_color == 0) ? 1 : 0;
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		// ���[�g�V�O�l�`���̎w��
		_cmdList->SetGraphicsRootSignature(rootsignature);
		// �f�B�X�N���v�^�q�[�v�̎w��
		_cmdList->SetDescriptorHeaps(1, &basicDescHeap);

		_cmdList->SetGraphicsRootDescriptorTable(0,
			basicDescHeap->GetGPUDescriptorHandleForHeapStart());

		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorrect);
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		_cmdList->IASetVertexBuffers(0, 1, &vbView);
		_cmdList->IASetIndexBuffer(&ibView);
		_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

		// �O�ゾ������ւ���
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		_cmdList->ResourceBarrier(1, &BarrierDesc);

		//���߂̃N���[�Y
		_cmdList->Close();

		// �R�}���h���X�g�̎��s
		ID3D12CommandList* cmdlists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);

		// �҂�
		_cmdQueue->Signal(_fence, ++_fenceVal);

		if (_fence->GetCompletedValue() != _fenceVal)
		{
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event); // �C�x���g�n���h���̎擾
			WaitForSingleObject(event, INFINITE); // �C�x���g����������܂Ŗ����ɑ҂�
			CloseHandle(event); // �C�x���g�n���h�������
		}
		_cmdAllocator->Reset(); // �L���[���N���A
		_cmdList->Reset(_cmdAllocator, nullptr); // �ĂуR�}���h���X�g�����߂鏀��

		// �t���b�v
		_swapchain->Present(1, 0);
	}
	// �g��Ȃ�����o�^��������
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}