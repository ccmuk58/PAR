#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

class GpuProfiler
{
public:
    static constexpr int kHistory = 120;

    struct Stat
    {
        std::string label;
        int         depth = 0;
        float       lastMs = 0.0f;
        float       avgMs = 0.0f;
        float       minMs = 0.0f;
        float       maxMs = 0.0f;
        uint64_t    lastFrame = 0;
        std::array<float, kHistory> history{};
        int         count = 0;
        int         head = 0;
    };

    bool Init(ID3D11Device* device);
    void BeginFrame(ID3D11DeviceContext* ctx);
    void EndFrame(ID3D11DeviceContext* ctx);
    int  Begin(ID3D11DeviceContext* ctx, std::string label);
    void End(ID3D11DeviceContext* ctx, int scope);
    void Reset() { m_stats.clear(); }

    const std::vector<Stat>& Stats() const { return m_stats; }
    bool IsCurrent(const Stat& stat) const { return stat.lastFrame == m_resolvedFrame; }

private:
    static constexpr int kFrameLatency = 4;
    static constexpr int kMaxScopes = 64;

    template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    struct Scope
    {
        ComPtr<ID3D11Query> begin;
        ComPtr<ID3D11Query> end;
        std::string         label;
        int                 depth = 0;
        bool                ended = false;
    };

    struct FrameSlot
    {
        ComPtr<ID3D11Query> disjoint;
        std::vector<Scope>  scopes;
        int                 used = 0;
        bool                pending = false;
        uint64_t            frame = 0;
    };

    void Resolve(ID3D11DeviceContext* ctx, FrameSlot& slot);
    void Record(const std::string& label, int depth, float ms, uint64_t frame);

    std::array<FrameSlot, kFrameLatency> m_slots;
    std::vector<Stat> m_stats;
    uint64_t m_frame = 0;
    uint64_t m_resolvedFrame = UINT64_MAX;
    int      m_depth = 0;
    bool     m_inFrame = false;
};

class GpuScope
{
public:
    GpuScope(GpuProfiler& profiler, ID3D11DeviceContext* ctx, std::string label)
        : m_profiler(profiler), m_ctx(ctx), m_scope(profiler.Begin(ctx, std::move(label))) {}
    ~GpuScope() { m_profiler.End(m_ctx, m_scope); }

    GpuScope(const GpuScope&) = delete;
    GpuScope& operator=(const GpuScope&) = delete;

private:
    GpuProfiler&         m_profiler;
    ID3D11DeviceContext* m_ctx;
    int                  m_scope;
};
