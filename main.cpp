#include "include\database\items.hpp"
#include "include\network\enet.hpp"
#include "include\database\sqlite3.hpp"
#include "include\database\peer.hpp"
#include "include\network\packet.hpp"
#include "include\database\world.hpp"
#include "include\tools\string_view.hpp"
#include "include\tools\random_engine.hpp"

#include <future>
#include "include\on\on"
#include "include\action\actions"

void git_check(const std::string& commit); // -> import git.o
void basic_https(const std::string& s_ip, u_short s_port, u_short https_port); // -> import https.o
bool ip_is_real(const std::string& ip, const std::string& token); // -> import anti-vpn.o
int enet_host_compress_with_range_coder(ENetHost* host); // -> import compress.o

int main() 
{
    git_check("0ff7da61e1ee40afda59b987605ef449cdbe536f");
    enet_initialize();
    {
        ENetAddress address{.host = ENET_HOST_ANY, .port = 17091};
        std::thread(&basic_https, "127.0.0.1", address.port, 443).detach();
        server = enet_host_create(&address, ENET_PROTOCOL_MAXIMUM_PEER_ID, 1, 0, 0);
            server->checksum = enet_crc32;
            enet_host_compress_with_range_coder(server);
    } /* deletes address */
    {
        struct _iobuf* file;
        if (fopen_s(&file, "items.dat", "rb") == 0) 
        {
            fseek(file, 0, SEEK_END);
            im_data.resize(ftell(file) + 60);
            for (int i = 0; i < 5; ++i)
                *reinterpret_cast<int*>(im_data.data() + i * sizeof(int)) = std::array<int, 5>{0x4, 0x10, -1, 0x0, 0x8}[i];
            *reinterpret_cast<int*>(im_data.data() + 56) = ftell(file);
            long end_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            fread(im_data.data() + 60, 1, end_size, file);
            std::span span{reinterpret_cast<const unsigned char*>(im_data.data()), im_data.size()};
                hash = std::accumulate(span.begin(), span.end(), 0x55555555u, 
                    [](auto start, auto end) { return (start >> 27) + (start << 5) + end; });
        } /* deletes span, deletes end_size */
        fclose(file);
    }
    cache_items();

    ENetEvent event{};
    while(true)
        while (enet_host_service(server, &event, 1) > 0)
            switch (event.type) 
            {
                case ENET_EVENT_TYPE_CONNECT:
                {
                    event.peer->data = new peer{}; // TODO
                    inet_ntop(AF_INET6, &(event.peer->address.host), getpeer->ipv6, INET6_ADDRSTRLEN); // I think this will also map-out peer's IPv4. I am too lazy to add IPv4 on it's own!

                    std::string problem{}; // waste of memory but it looks better this way...
                    if (not ip_is_real(getpeer->ipv6, "7fff5d956e4445e6943055fc17fcd0eb")) // only when hosting. this will not include localhost (::ffff:127.0.0.1)
                        problem = std::format("`4Can not make a new account!`` Sorry, but IP {} is not permitted to create NEW Growtopia account at this time. (this can be because there is an open proxy/VPN here or abuse has come from this IP) Please try again from another IP address.", getpeer->ipv6).c_str();
                    else if (peers(ENET_PEER_STATE_CONNECTING).size() > 2) 
                        problem = "`4OOPS:`` Too many people logging in at once. Please press `5CANCEL`` and try again in a few seconds.";
                    else if (enet_peer_send(event.peer, 0, enet_packet_create(
                        []{ std::array<enet_uint8, 4> data = {0x1}; return data.data(); }(), 4, ENET_PACKET_FLAG_RELIABLE)) == 0) break;
                    else [[unlikely]] problem = "`4ERROR:`` try reconnecting to the `wserver``."; // a static byte array. this outcome is unlikely. (tell me otherwise...)
                    packet(*event.peer, std::format("action|log\nmsg|{}", problem).c_str());
                    enet_peer_disconnect_later(event.peer, ENET_NORMAL_DISCONNECTION); // calls ENET_EVENT_TYPE_DISCONNECT. this is nice for deleting that pre-allocated peer class!
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: /* if peer closes growtopia.exe */
                    quit(event, "");
                    break;
                case ENET_EVENT_TYPE_RECEIVE: 
                {
                    std::cout << int{std::span{event.packet->data, event.packet->dataLength}[0]} << std::endl;
                    switch (std::span{event.packet->data, event.packet->dataLength}[0]) 
                    {
                        case 2: case 3: 
                        {
                            std::string header{std::span{event.packet->data, event.packet->dataLength}.begin() + 4, std::span{event.packet->data, event.packet->dataLength}.end() - 1};
                            std::cout << header << std::endl;
                            std::ranges::replace(header, '\n', '|');
                            std::vector<std::string> pipes = readpipe(header);
                            const std::string action{(pipes[0] == "protocol") ? pipes[0] : pipes[0] + "|" + pipes[1]};
                            if (auto i = action_pool.find(action); i not_eq action_pool.end())
                                (static_cast<void>(std::async(std::launch::async, i->second, std::ref(event), std::ref(header))));
                            break;
                        }
                        case 4: 
                        {
                            std::unique_ptr<state> state{};
                            {
                                std::vector<std::byte> packet(event.packet->dataLength - 4, std::byte{0x00});
                                if ((packet.size() + 4) >= 60)
                                    for (size_t i = 0; i < packet.size(); ++i)
                                        packet[i] = (reinterpret_cast<std::byte*>(event.packet->data) + 4)[i];
                                if (std::to_integer<unsigned char>(packet[12]) bitand 0x8 and 
                                    packet.size() < static_cast<size_t>(*reinterpret_cast<int*>(&packet[52])) + 56) break;
                                state = get_state(packet);
                            } /* deletes packet ahead of time */
                            switch (state->type) 
                            {
                                case 0: 
                                {
                                    if (getpeer->post_enter.try_lock()) // memory optimize- push only during an actual world enter
                                    {
                                        gt_packet(*event.peer, 0, true, "OnSetPos", floats{getpeer->pos[0], getpeer->pos[1]});
                                        gt_packet(*event.peer, 0, true, "OnChangeSkin", -1429995521);
                                    }
                                    getpeer->pos = state->pos;
                                    getpeer->facing_left = state->peer_state bitand 0x10;
                                    state_visuals(event, *state);
                                    break;
                                }
                                case 3: 
                                {
                                    if (not create_rt(event, 0, 200ms)) break; // this will only affect hackers (or macro spammers)
                                    short block1D = state->punch[1] * 100 + state->punch[0]; // 2D (x, y) to 1D ((destY * y + destX)) formula
                                    block& b = worlds[getpeer->recent_worlds.back()].blocks[block1D];
                                    if (state->id == 18) // punching blocks
                                    {
                                        // ... TODO add a timer that resets hits every 6-8 seconds (threaded stopwatch)
                                        if (b.bg == 0 and b.fg == 0) break;
                                        if (b.fg == 8 or b.fg == 6) {
                                            gt_packet(*event.peer, 0, false, "OnTalkBubble", getpeer->netid, b.fg == 8 ? 
                                                "It's too strong to break." : "(stand over and punch to use)");
                                            break;
                                        }
                                        block_punched(event, *state, block1D);
                                        if (b.fg not_eq 0 and b.hits[0] >= items[b.fg].hits) b.fg = 0;
                                        else if (b.bg not_eq 0 and b.hits[1] >= items[b.bg].hits) b.bg = 0;
                                        else break;
                                    }
                                    else // placing blocks
                                        (items[state->id].type == 18) ? b.bg = state->id : b.fg = state->id;
                                    auto w = std::make_unique<world>(worlds[getpeer->recent_worlds.back()]);
                                    overwrite_tile(w, block1D, b);
                                    state_visuals(event, *state);
                                    break;
                                }
                                case 11:
                                {
                                    gt_packet(*event.peer, 0, false, "OnConsoleMessage", "Collected `w{amount} {item name}{(s) >= 2}``. Rarity: `w{rarity}``"); // incomplete
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    enet_packet_destroy(event.packet); /* cleanup */
                    break;
                }
            }
    return 0;
}
