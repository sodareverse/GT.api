#include <ranges> /* std::ranges:: */
#include <algorithm> /* string manipulation */
#include <vector>

#include "include/database/items.hpp"
#include "include/network/enet.hpp"
#include <memory> /* std::unique_ptr<>  */
#include "include/database/peer.hpp"
#include "include/database/sqlite3.hpp"
#include "include/network/packet.hpp"
#include "include/database/world.hpp"

#include <random> /* std::minstd_rand */

/* a completely random randomizer */ // -> TODO: compress inside a .o file
class seed {
    public:
    seed() : lcr(device()) {}
    /* @brief get a random number effectively. */
    uint_fast32_t fast(const uint_fast32_t min, const uint_fast32_t max) 
    { 
        return std::uniform_int_distribution<uint_fast32_t>(min, max)(lcr); 
    }
    private:
    std::random_device device; /* a device to be wrapped in the (LCR) engine */
    std::minstd_rand lcr; /* reusable (LCR) engine */
};

namespace github {
    void sync(const char* commit); // -> github.o
}

#include <fstream> /* std::ifstream */
#include <thread> /* std::jthread */

int main() {
    github::sync("8f0b1cb19bdd8d70eadc432ef25a5932141fc40f");
    enet_initialize();

    ENetAddress address{.host = ENET_HOST_ANY, .port = 17091};

    int enet_host_compress_with_range_coder(ENetHost* host); // -> compress.o
    server = enet_host_create(&address, ENET_PROTOCOL_MAXIMUM_PEER_ID, 0, 0, 0);
        server->checksum = enet_crc32;
        enet_host_compress_with_range_coder(server);
    {
        std::ifstream file("items.dat", std::ios::binary | std::ios::ate);
        std::streamsize im_size = file.tellg();
        im_data.resize(im_size + 60);
        for (int i = 0; i < 5; ++i)
            *reinterpret_cast<int*>(im_data.data() + i * sizeof(int)) =  std::array<int, 5>{0x4, 0x10, -1, 0x0, 0x8}[i];
        *reinterpret_cast<int*>(im_data.data() + 56) = im_size;
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(im_data.data() + 60), im_size);
        auto span = std::span<const unsigned char>(reinterpret_cast<const unsigned char*>(im_data.data()), im_data.size());
            hash = std::accumulate(span.begin(), span.end(), 0x55555555u, 
                [](auto start, auto end) { return (start >> 27) + (start << 5) + end; });
    } /* deletes span, calls file.close(), deletes im_size */
    cache_items();

    ENetEvent event{};
    while (true) 
    {
        while (enet_host_service(server, &event, 10) > 0)
	    {
            std::jthread([&](std::stop_token stop)
	        {
                switch (event.type) 
                {
                    case ENET_EVENT_TYPE_NONE: break;
                    case ENET_EVENT_TYPE_CONNECT:
                        if (enet_peer_send(event.peer, 0, enet_packet_create((const enet_uint8[4]){0x1}, 4, ENET_PACKET_FLAG_RELIABLE)) < 0) break;
                        event.peer->data = new peer{};
                        break;
                    case ENET_EVENT_TYPE_DISCONNECT:
                        delete getpeer;
                        break;
                    case ENET_EVENT_TYPE_RECEIVE: 
                    {
                        /** @return list of the pipes '|' */
                        std::vector<std::string> readpipe(const std::string& str); // -> utility.o

                        std::span packet{reinterpret_cast<char*>(event.packet->data), event.packet->dataLength};
                            std::string header{packet.begin() + 4, packet.end() - 1};
                        switch (std::span{event.packet->data, event.packet->dataLength}[0]) 
                        {
                            case 2: 
                            {
                                std::call_once(getpeer->logging_in, [&]() 
                                {
                                    std::ranges::replace(header, '\n', '|'); /* e.g. requestedName|test\n = requestedName|test| */
                                    std::vector<std::string> read_once = readpipe(header); 
                                    seed random{};
                                    if (read_once[0] == "requestedName" or read_once[0] == "tankIDName") {
                                        read_once[0] == "requestedName" ? 
                                            getpeer->requestedName = read_once[1] + "_" + std::to_string(random.fast(100, 999)) :
                                            getpeer->tankIDName = read_once[1];
                                        if (not getpeer->tankIDName.empty() and getpeer->tankIDPass not_eq read_once[3])
                                        {
                                            gt_packet(*event.peer, 0, "OnConsoleMessage", "`4Unable to log on:`` That `wGrowID`` doesn't seem valid, or the password is wrong.  If you don't have one, press `wCancel``, un-check `w'I have a GrowID'``, then click `wConnect``.");
                                            enet_peer_disconnect_later(event.peer, 0);
                                            return;
                                        }
                                        short offset{};
                                        if (not getpeer->tankIDName.empty()) {
                                            getpeer->tankIDName = read_once[1];
                                            getpeer->tankIDPass = read_once[3];
                                            offset = 4;
                                        }
                                        getpeer->country = read_once[37 + offset];
                                    }
                                    gt_packet(*event.peer, 0,
                                        "OnSuperMainStartAcceptLogonHrdxs47254722215a", 
                                        hash, 
                                        "ubistatic-a.akamaihd.net", 
                                        "0098/2521452/cache/", 
                                        "cc.cz.madkite.freedom org.aqua.gg idv.aqua.bulldog com.cih.gamecih2 com.cih.gamecih com.cih.game_cih cn.maocai.gamekiller com.gmd.speedtime org.dax.attack com.x0.strai.frep com.x0.strai.free org.cheatengine.cegui org.sbtools.gamehack com.skgames.traffikrider org.sbtoods.gamehaca com.skype.ralder org.cheatengine.cegui.xx.multi1458919170111 com.prohiro.macro me.autotouch.autotouch com.cygery.repetitouch.free com.cygery.repetitouch.pro com.proziro.zacro com.slash.gamebuster", 
                                        "proto=208|choosemusic=audio/mp3/about_theme.mp3|active_holiday=12|wing_week_day=0|ubi_week_day=0|server_tick=59197218|clash_active=0|drop_lavacheck_faster=1|isPayingUser=0|usingStoreNavigation=1|enableInventoryTab=1|bigBackpack=1|"
                                    );
                                });
                                if (header.starts_with("action|refresh_item_data")) {
                                    enet_peer_send(event.peer, 0, enet_packet_create(im_data.data(), im_data.size(), ENET_PACKET_FLAG_NO_ALLOCATE));
                                    break;
                                }
                                else if (header.starts_with("action|enter_game"))
                                {
                                    std::call_once(getpeer->entered_game, [&]() 
                                    {
                                        getpeer->user_id = peers().size();
                                        gt_packet(*event.peer, 0, "OnConsoleMessage", std::format("Welcome back, `w`w{}````.", getpeer->requestedName).c_str());
                                        gt_packet(*event.peer, 0, "SetHasGrowID", getpeer->tankIDName.empty() ? 0 : 1, getpeer->tankIDName.c_str(), getpeer->tankIDName.c_str());
                                        getpeer->slots.emplace_back(slot{18, 1});
                                        getpeer->slots.emplace_back(slot{32, 1});
                                        getpeer->slots.emplace_back(slot{2, 200});
                                        OnRequestWorldSelectMenu(event);
                                    });
                                }
                                break;

                            }
                            case 3: 
                            {
                                if (header.contains("action|quit_to_exit")) {
                                    peers([&](ENetPeer& p) {
                                        if (not getp->recent_worlds.empty() and not getpeer->recent_worlds.empty() and getp->recent_worlds.back() == getpeer->recent_worlds.back())
                                            gt_packet(p, 0, "OnRemove", std::format("netID|{}\n", getpeer->netid).c_str());
                                    });
                                    OnRequestWorldSelectMenu(event);
                                }
                                else if (header.contains("action|quit")) 
                                    enet_peer_disconnect(event.peer, ENET_NORMAL_DISCONNECTION);
                                else if (header.starts_with("action|join_request")) {
                                    std::ranges::replace(header, '\n', '|');
                                    std::string big_name{readpipe(std::string{header})[3]};
                                    std::ranges::transform(big_name, big_name.begin(), [](char c) { return std::toupper(c); });
                                    std::unique_ptr<world> w = read_world(big_name);
                                    if (w == nullptr) /* create a new world */
                                    {
                                        w = std::make_unique<world>(world{.name = big_name}); /* replace nullptr with world constructor */
                                        seed random{};
                                        auto main_door = random.fast(2, ((100 * 60) / ((100 * 60) / 100) - 4));
                                        std::vector<block> blocks(100 * 60, block{0, 0});
                                        for (auto& b : blocks) {
                                            auto i = &b - &blocks[0];
                                            if (i >= 3700) {
                                                b.bg = 14; // cave background
                                                b.fg = (i >= 3800 and i < 5000 /* lava level */ and not random.fast(0, 38)) ? 10 : 
                                                    (i > 5000 and i < 5400 /* bedrock level */ and random.fast(0, 7) < 3) ? 4 : 
                                                    (i >= 5400) ? 8 : 2;
                                            }
                                            if (i == 3600 + main_door) b.fg = 6; // main door
                                            if (i == 3700 + main_door) b.fg = 8; // Bedrock below the door
                                        }
                                        w->blocks = std::move(blocks);
                                        write_world(w);
                                    }
                                    short y = w->blocks.size() / 100, x = w->blocks.size() / y;
                                    std::vector<std::byte> data(78 + w->name.length() + w->blocks.size() + 24 + (8 * w->blocks.size()), std::byte{0x00});
                                    data[0] = std::byte{0x4};
                                    data[4] = std::byte{0x4};
                                    data[16] = std::byte{0x8};
                                    unsigned char name_size = w->name.length(); /* Growtopia limits world name length hence 255 is plenty of space */
                                    data[66] = std::byte{name_size};
                                    for (size_t i = 0; i < name_size; ++i)
                                        data[68 + i] = static_cast<std::byte>(w->name[i]);
                                    data[68 + name_size] = static_cast<std::byte>(x);
                                    data[72 + name_size] = static_cast<std::byte>(y);
                                    *reinterpret_cast<unsigned short*>(data.data() + 76 + name_size) = static_cast<unsigned short>(w->blocks.size());
                                    int pos = 85 + name_size;
                                    for (size_t i = 0; i < w->blocks.size(); ++i) {
                                        *reinterpret_cast<short*>(data.data() + pos) = w->blocks[i].fg;
                                        *reinterpret_cast<short*>(data.data() + (pos + 2)) = w->blocks[i].bg;
                                        *reinterpret_cast<unsigned*>(data.data() + (pos + 4)) = w->blocks[i].flags;
                                        if (w->blocks[i].fg == 6) {
                                            getpeer->pos[0] = (i % x) * 32;
                                            getpeer->pos[1] = (i / x) * 32;
                                            data[pos + 8] = std::byte{0x1};
                                            *reinterpret_cast<short*>(data.data() + pos + 9) = 4;
                                            for (size_t i = 0; i < 4; ++i)
                                                data[pos + 11 + i] = static_cast<std::byte>(std::string_view{"EXIT"}[i]);
                                            pos += 8;
                                        }
                                        pos += 8;
                                    }
                                    enet_peer_send(event.peer, 0, enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE));
                                    for (std::size_t i = 0; i < getpeer->recent_worlds.size() - 1; ++i)
                                        getpeer->recent_worlds[i] = getpeer->recent_worlds[i + 1];
                                    getpeer->recent_worlds.back() = w->name;
                                    getpeer->netid = ++w->visitors;
                                    gt_packet(*event.peer, 0, "OnSpawn", std::format(
                                        "spawn|avatar\nnetID|{0}\nuserID|{1}\ncolrect|0|0|20|30\nposXY|{2}|{3}\nname|{4}\ncountry|{5}\ninvis|0\nmstate|0\nsmstate|0\ntype|local\n",
                                        getpeer->netid, getpeer->user_id, static_cast<int>(getpeer->pos[0]), static_cast<int>(getpeer->pos[1]), getpeer->requestedName, getpeer->country).c_str());
                                    gt_packet(*event.peer, 0, "OnSetPos", std::vector<float>{getpeer->pos[0], getpeer->pos[1]});
                                    gt_packet(*event.peer, 0, "OnConsoleMessage", std::format("World `w{0}`` entered.  There are `w{1}`` other people here, `w{2}`` online.",
                                        w->name, w->visitors - 1, peers().size()).c_str());
                                    inventory_visuals(*event.peer);
                                }
                                break;
                            }
                            case 4: 
                            {
                                std::unique_ptr<state> state{};
                                {
                                    std::vector<std::byte> packet(event.packet->dataLength - 4, std::byte{0x00});
                                    if ((packet.size() + 4) >= 60) {
                                        for (size_t i = 0; i < packet.size(); ++i)
                                            packet[i] = (reinterpret_cast<std::byte*>(event.packet->data) + 4)[i];
                                        if (std::to_integer<unsigned char>(packet[12]) bitand 0x8 and packet.size() < static_cast<size_t>(*reinterpret_cast<int*>(&packet[52])) + 56) 
                                            packet.clear();
                                    }
                                    state = get_state(packet);
                                } /* deletes packet ahead of time */
                                switch (state->type) 
                                {
                                    case 0: {
                                        state_visuals(event, *state); /* show peer moving (everyone) */
                                        break;
                                    }
                                    case 3: case 18: {
                                        state_visuals(event, *state); /* show tile being broken or placed (everyone) */
                                        break;
                                    }
                                    case 24: break; /* seems to happen before action|enter_game */
                                    default: break;
                                }
                                break;
                            }
                        }
                        enet_packet_destroy(event.packet); /* cleanup */
                        break;
                    }
                }
	        });
        }
    }
    return 0;
}
