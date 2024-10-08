
void join_request(ENetEvent event, const std::string& header) 
{
    try 
    {
        if (not create_rt(event, 2, 900ms)) throw std::runtime_error("");
        std::string big_name{readch(std::string{header}, '|')[3]};
        if (not alpha(big_name) or big_name.empty()) throw std::runtime_error("Sorry, spaces and special characters are not allowed in world or door names.  Try again.");
        std::ranges::transform(big_name, big_name.begin(), [](char c) { return std::toupper(c); });
        auto w = std::make_unique<world>(world().read(big_name));
        if (w->name.empty()) 
        {
            engine::simple random;
            auto main_door = scope(random, 2, 100 * 60 / 100 - 4);
            std::vector<block> blocks(100 * 60, block{0, 0});
            for (auto& b : blocks) 
            {
                auto i = &b - &blocks[0];
                if (i >= 3700) 
                    b.bg = 14, // cave background
                    b.fg = (i >= 3800 and i < 5000 /* lava level */ and not scope(random, 0, 38)) ? 10 : 
                        (i > 5000 and i < 5400 /* bedrock level */ and scope(random, 0, 7) < 3) ? 4 : 
                        (i >= 5400) ? 8 : 2;
                if (i == 3600 + main_door) b.fg = 6; // main door
                if (i == 3700 + main_door) b.fg = 8; // bedrock below the main door
            }
            w->blocks = std::move(blocks);
            w->name = big_name; // init
        }
        {std::vector<std::byte> data(85 + w->name.length() + 5/*unknown*/ + (8 * w->blocks.size()) + 12/*initial drop*/, std::byte{0x00});
        data[0] = std::byte{0x4};
        data[4] = std::byte{0x4};
        data[16] = std::byte{0x8};
        unsigned char len = static_cast<unsigned char>(w->name.length());
        data[66] = std::byte{len};
        for (size_t i = 0; i < static_cast<int>(len); ++i)
            data[68 + i] = static_cast<std::byte>(w->name[i]);
        short y = w->blocks.size() / 100, x = w->blocks.size() / y;
        data[68 + static_cast<int>(len)] = static_cast<std::byte>(x);
        data[72 + static_cast<int>(len)] = static_cast<std::byte>(y);
        *reinterpret_cast<unsigned short*>(data.data() + 76 + len) = static_cast<unsigned short>(w->blocks.size());
        int pos = 85 + static_cast<int>(len);
        short i = 0;
        for (const auto& block : w->blocks)
        {
            auto [fg, bg, hits] = block;
            *reinterpret_cast<short*>(data.data() + pos) = fg;
            *reinterpret_cast<short*>(data.data() + (pos + 2)) = bg;
            if (fg == 6) // @todo all door labels & signs.
            {
                getpeer->pos.front() = (i % x) * 32;
                getpeer->pos.back() = (i / x) * 32;
                getpeer->rest_pos = getpeer->pos; // @note static repsawn position
                data.resize(data.size() + 7);
                data[pos + 8] = std::byte{0x1};
                *reinterpret_cast<short*>(data.data() + (pos + 9)) = 4;
                for (size_t ii = 0; ii < 4; ++ii)
                    data[pos + 11 + ii] = static_cast<std::byte>("EXIT"[ii]);
                pos += 8; // @todo what is the missing 1 bit?
            }
            else if (fg == 242) // @todo all locks
            {
                data.resize(data.size() + 15);
                data[pos + 8] = std::byte{0x3};
                data[pos + 9] = std::byte{0x1};
                *reinterpret_cast<int*>(data.data() + (pos + 10)) = 1; // @note owner user ID
                data[pos + 14] = std::byte{0x1}; // @note number of admins
                *reinterpret_cast<int*>(data.data() + (pos + 18)) = -100; // @note default world bpm
                *reinterpret_cast<int*>(data.data() + (pos + 22)) = 1; // @note list of admins
                pos += 15;
            }
            pos += 8;
            ++i;
        }
        enet_peer_send(event.peer, 0, enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE));} // @note delete data:

        for (const auto& [uid, id, count, position] : w->ifloats)
        {
            std::vector<std::byte> compress = compress_state({.type = 14, .netid = -1, .id = id, .pos = {position[0] * 32, position[1] * 32}});
            *reinterpret_cast<int*>(compress.data() + 8) = uid + 1; // @todo
            *reinterpret_cast<float*>(compress.data() + 16) = static_cast<float>(count);
            send_data(*event.peer, compress);
        }
        if (std::find(getpeer->recent_worlds.begin(), getpeer->recent_worlds.end(), w->name) == getpeer->recent_worlds.end()) 
        {
            std::rotate(getpeer->recent_worlds.begin(), getpeer->recent_worlds.begin() + 1, getpeer->recent_worlds.end());
            getpeer->recent_worlds.back() = w->name;
        }
        getpeer->ongoing_world = w->name;
        EmoticonDataChanged(event);
        getpeer->netid = ++w->visitors;
        gt_packet(*event.peer, false, "OnSpawn", std::format("spawn|avatar\nnetID|{}\nuserID|{}\ncolrect|0|0|20|30\nposXY|{}|{}\nname|{}\ncountry|{}\ninvis|0\nmstate|0\nsmstate|0\nonlineID|\ntype|local\n",
            getpeer->netid, getpeer->user_id, static_cast<int>(getpeer->pos[0]), static_cast<int>(getpeer->pos[1]), getpeer->nickname, getpeer->country).c_str());
        peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
        {
            if (not getp->recent_worlds.empty() and not getpeer->recent_worlds.empty() and getp->recent_worlds.back() == getpeer->recent_worlds.back() and getp->user_id not_eq getpeer->user_id)
            {
                gt_packet(p, false, "OnSpawn", std::format("spawn|avatar\nnetID|{}\nuserID|{}\ncolrect|0|0|20|30\nposXY|{}|{}\nname|{}\ncountry|{}\ninvis|0\nmstate|0\nsmstate|0\nonlineID|\n",
                    getp->netid, getp->user_id, static_cast<int>(getp->pos.front()), static_cast<int>(getp->pos.back()), getp->nickname, getp->country).c_str());
                gt_packet(p, false, "OnConsoleMessage", std::format("`5<`w{}`` entered, `w{}`` others here>``", 
                    getpeer->nickname, w->visitors).c_str());
                gt_packet(p, false, " OnTalkBubble", getpeer->netid, std::format("`5<`w{}`` entered, `w{}`` others here>``", 
                    getpeer->nickname, w->visitors).c_str());
            }
        });
        gt_packet(*event.peer, false, "OnConsoleMessage", std::format("World `w{}`` entered.  There are `w{}`` other people here, `w{}`` online.",
            w->name, w->visitors - 1, peers().size()).c_str());
        inventory_visuals(*event.peer);
        if (worlds.find(w->name) == worlds.end())
            worlds.emplace(w->name, *w);
    }
    catch (const std::exception& exc)
    {
        if (not std::string{exc.what()}.empty()) gt_packet(*event.peer, false, "OnConsoleMessage", exc.what());
        gt_packet(*event.peer, false, "OnFailedToEnterWorld");
    }
}