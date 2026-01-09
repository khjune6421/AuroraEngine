#pragma once

enum class ActionKind : uint16_t
{
    Move = 1,
    EndTurn = 2,
};

enum class MoveDir : uint8_t { Up, Down, Left, Right };

#pragma pack(push, 1)
struct ActionHeader
{
    uint32_t turn;
    uint32_t actorNetId;
    uint32_t targetNetId; // 없으면 0
    uint16_t kind;        // ActionKind
    uint16_t payloadSize; // 뒤 payload byte
    uint32_t seq;         // client local seq
};
#pragma pack(pop)
static_assert(sizeof(ActionHeader) == 20);

#pragma pack(push, 1)
struct ActionMove
{
    uint8_t dir; // MoveDir
    float dist;
};
#pragma pack(pop)
static_assert(sizeof(ActionMove) == 5);

// Serialize helpers
struct ByteBuffer
{
    std::vector<uint8_t> data;

    template<typename T>
    void WritePod(const T& v)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        auto p = reinterpret_cast<const uint8_t*>(&v);
        data.insert(data.end(), p, p + sizeof(T));
    }
};

struct ByteReader
{
    const uint8_t* p = nullptr;
    size_t n = 0;
    size_t off = 0;

    ByteReader(const void* data, size_t size)
        : p(reinterpret_cast<const uint8_t*>(data)), n(size) {
    }

    template<typename T>
    bool ReadPod(T& out)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        if (off + sizeof(T) > n) return false;
        std::memcpy(&out, p + off, sizeof(T));
        off += sizeof(T);
        return true;
    }

    size_t Remaining() const { return n - off; }
};