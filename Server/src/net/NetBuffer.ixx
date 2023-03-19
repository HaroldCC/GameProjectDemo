export module net:NetBuffer;

import net;

export namespace net
{

    class Packet : public Buffer
    {
    public:
        struct PacketHead
        {
            uint16_t msgID;
        };

    public:
        bool HasData() const
        {
            return ReadableBytes() > 0;
        }

    private:
        uint16_t _msgID;
    };

}