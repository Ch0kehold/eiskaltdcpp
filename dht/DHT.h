/*
 * Copyright (C) 2009-2010 Big Muscle, http://strongdc.sourceforge.net/
 * Copyright (C) 2019 Boris Pek <tehnick-8@yandex.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "BootstrapManager.h"
#include "Constants.h"
#include "KBucket.h"
#include "UDPSocket.h"
#include "stdafx.h"
#include "dcpp/AdcCommand.h"
#include "dcpp/CID.h"
#include "dcpp/MerkleTree.h"
#include "dcpp/Singleton.h"
#include "dcpp/TimerManager.h"

namespace dht
{

    class DHT :
        public Singleton<DHT>, public Speaker<ClientListener>, public ClientBase
    {
    public:
        explicit DHT(void);
        ~DHT(void) throw();

        enum InfType { NONE = 0, PING = 1, MAKE_ONLINE = 2 };

        /** ClientBase derived functions */
        const string& getHubUrl() const { return NetworkName; }
        string getHubName() const { return NetworkName; }
        bool isOp() const { return false; }

        /** Starts DHT. */
        void start();
        void stop(bool exiting = false);

        const string& getPort() const { return BOOLSETTING(USE_DHT) ? socket.getPort() : Util::emptyString; }

        /** Process incoming command */
        void dispatch(const string& aLine, const string& ip, const string &port, bool isUdpKeyValid);

        /** Sends command to ip and port */
        void send(AdcCommand& cmd, const string& ip, const string &port, const CID& targetCID, const CID& udpKey);

        /** Creates new (or update existing) node which is NOT added to our routing table */
        Node::Ptr createNode(const CID& cid, const string& ip, const string &port, bool update, bool isUdpKeyValid);

        /** Adds node to routing table */
        bool addNode(const Node::Ptr& node, bool makeOnline);

        /** Returns counts of nodes available in k-buckets */
        size_t getNodesCount() { Lock l(cs); return bucket->getNodes().size(); }

        /** Removes dead nodes */
        void checkExpiration(uint64_t aTick);

        /** Finds the file in the network */
        void findFile(const string& tth, const string& token = Util::toString(Util::rand()));

        /** Sends our info to specified ip:port */
        void info(const string& ip, const string &port, uint32_t type, const CID& targetCID, const CID& udpKey);

        /** Sends Connect To Me request to online node */
        void connect(const OnlineUser& ou, const string& token);

        /** Sends private message to online node */
        void privateMessage(const OnlineUser& ou, const string& aMessage, bool thirdPerson);

        /** Is DHT connected? */
        bool isConnected() const { return lastPacket && (GET_TICK() - lastPacket < CONNECTED_TIMEOUT); }

        /** Mark that network data should be saved */
        void setDirty() { dirty = true; }

        /** Saves network information to XML file */
        void saveData();

        /** Returns if our UDP port is open */
        bool isFirewalled() const { return firewalled; }

        /** Returns our IP got from the last firewall check */
        string getLastExternalIP() const { return lastExternalIP; }

        void setRequestFWCheck() { Lock l(cs); requestFWCheck = true; firewalledWanted.clear(); firewalledChecks.clear(); }

    private:
        /** Classes that can access to my private members */
        friend class Singleton<DHT>;
        friend class SearchManager;

        void handle(AdcCommand::INF, const Node::Ptr& node, AdcCommand& c) throw(); // user's info
        void handle(AdcCommand::SCH, const Node::Ptr& node, AdcCommand& c) throw(); // incoming search request
        void handle(AdcCommand::RES, const Node::Ptr& node, AdcCommand& c) throw(); // incoming search result
        void handle(AdcCommand::PUB, const Node::Ptr& node, AdcCommand& c) throw(); // incoming publish request
        void handle(AdcCommand::CTM, const Node::Ptr& node, AdcCommand& c) throw(); // connection request
        void handle(AdcCommand::RCM, const Node::Ptr& node, AdcCommand& c) throw(); // reverse connection request
        void handle(AdcCommand::STA, const Node::Ptr& node, AdcCommand& c) throw(); // status message
        void handle(AdcCommand::PSR, const Node::Ptr& node, AdcCommand& c) throw(); // partial file request
        void handle(AdcCommand::MSG, const Node::Ptr& node, AdcCommand& c) throw(); // private message
        void handle(AdcCommand::GET, const Node::Ptr& node, AdcCommand& c) throw();
        void handle(AdcCommand::SND, const Node::Ptr& node, AdcCommand& c) throw();

        /** Unsupported command */
        template<typename T> void handle(T, const Node::Ptr&, AdcCommand&) { }

        /** UDP socket */
        UDPSocket   socket;

        /** Routing table */
        KBucket*    bucket;

        /** Lock to routing table */
        CriticalSection cs;
        CriticalSection fwCheckCs;

        /** Our external IP got from last firewalled check */
        string lastExternalIP;

        /** Time when last packet was received */
        uint64_t lastPacket;

        /** IPs who we received firewalled status from */
        std::unordered_set<string> firewalledWanted;
        std::unordered_map< string, std::pair< string, string > > firewalledChecks;
        bool firewalled;
        bool requestFWCheck;

        /** Should the network data be saved? */
        bool dirty;

        /** Finds "max" closest nodes and stores them to the list */
        void getClosestNodes(const CID& cid, std::map<CID, Node::Ptr>& closest, unsigned int max, uint8_t maxType);

        /** Loads network information from XML file */
        void loadData();
    };

}
