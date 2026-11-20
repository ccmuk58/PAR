#include "GpuProfiler.h"
#include "D3DUtil.h"

#include <algorithm>

namespace
{
    template <class T>
    bool ReadQuery(ID3D11DeviceContext* ctx, ID3D11Query* query, T& out)
    {
        HRESULT hr;
        while ((hr = ctx->GetData(query, &out, sizeof(T), 0)) == S_FALSE) {}
        return hr == S_OK;
    }
}

bool GpuProfiler::Init(ID3D11Device* device)
{
    const D3D11_QUERY_DESC disjointDesc{ D3D11_QUERY_TIMESTAMP_DISJOINT, 0 };
    const D3D11_QUERY_DESC timestampDesc{ D3D11_QUERY_TIMESTAMP, 0 };

    for (FrameSlot& slot : m_slots)
    {
        if (!Check(device->CreateQuery(&disjointDesc, &slot.disjoint), "CreateQuery(disjoint)"))
            return false;
        slot.scopes.resize(kMaxScopes);
        for (Scope& scope : slot.scopes)
        {
            if (!Check(device->CreateQuery(&timestampDesc, &scope.begin), "CreateQuery(timestamp)"))
                return false;
            if (!Check(device->CreateQuery(&timestampDesc, &scope.end), "CreateQuery(timestamp)"))
                return false;
        }
    }
    return true;
}

void GpuProfiler::BeginFrame(ID3D11DeviceContext* ctx)
{
    FrameSlot& slot = m_slots[m_frame % kFrameLatency];
    if (slot.pending)
        Resolve(ctx, slot);

    slot.used = 0;
    slot.frame = m_frame;
    slot.pending = true;
    m_depth = 0;
    m_inFrame = true;
    ctx->Begin(slot.disjoint.Get());
}

void GpuProfiler::EndFrame(ID3D11DeviceContext* ctx)
{
    if (!m_inFrame)
        return;
    ctx->End(m_slots[m_frame % kFrameLatency].disjoint.Get());
    m_inFrame = false;
    ++m_frame;
}

int GpuProfiler::Begin(ID3D11DeviceContext* ctx, std::string label)
{
    FrameSlot& slot = m_slots[m_frame % kFrameLatency];
    if (!m_inFrame || slot.used >= kMaxScopes)
        return -1;

    Scope& scope = slot.scopes[slot.used];
    scope.label = std::move(label);
    scope.depth = m_depth++;
    scope.ended = false;
    ctx->End(scope.begin.Get());
    return slot.used++;
}

void GpuProfiler::End(ID3D11DeviceContext* ctx, int index)
{
    if (index < 0 || !m_inFrame)
        return;
    Scope& scope = m_slots[m_frame % kFrameLatency].scopes[index];
    ctx->End(scope.end.Get());
    scope.ended = true;
    --m_depth;
}

void GpuProfiler::Resolve(ID3D11DeviceContext* ctx, FrameSlot& slot)
{
    slot.pending = false;

    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint{};
    if (!ReadQuery(ctx, slot.disjoint.Get(), disjoint) || disjoint.Disjoint || disjoint.Frequency == 0)
        return;

    for (int i = 0; i < slot.used; ++i)
    {
        const Scope& scope = slot.scopes[i];
        if (!scope.ended)
            continue;
        UINT64 t0 = 0, t1 = 0;
        if (!ReadQuery(ctx, scope.begin.Get(), t0) || !ReadQuery(ctx, scope.end.Get(), t1) || t1 < t0)
            continue;
        const float ms = static_cast<float>(double(t1 - t0) / double(disjoint.Frequency) * 1000.0);
        Record(scope.label, scope.depth, ms, slot.frame);
    }
    m_resolvedFrame = slot.frame;
}

void GpuProfiler::Record(const std::string& label, int depth, float ms, uint64_t frame)
{
    auto it = std::find_if(m_stats.begin(), m_stats.end(), [&](const Stat& s) { return s.label == label; });
    if (it == m_stats.end())
    {
        m_stats.push_back({});
        it = m_stats.end() - 1;
        it->label = label;
    }

    Stat& stat = *it;
    stat.depth = depth;
    stat.lastMs = ms;
    stat.lastFrame = frame;
    stat.history[stat.head] = ms;
    stat.head = (stat.head + 1) % kHistory;
    stat.count = std::min(stat.count + 1, kHistory);

    float sum = 0.0f;
    stat.minMs = stat.history[0];
    stat.maxMs = stat.history[0];
    for (int i = 0; i < stat.count; ++i)
    {
        sum += stat.history[i];
        stat.minMs = std::min(stat.minMs, stat.history[i]);
        stat.maxMs = std::max(stat.maxMs, stat.history[i]);
    }
    stat.avgMs = sum / static_cast<float>(stat.count);
}
