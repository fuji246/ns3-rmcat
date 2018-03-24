/******************************************************************************
 * Copyright 2016-2017 Cisco Systems, Inc.                                    *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License");            *
 * you may not use this file except in compliance with the License.           *
 *                                                                            *
 * You may obtain a copy of the License at                                    *
 *                                                                            *
 *     http://www.apache.org/licenses/LICENSE-2.0                             *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 ******************************************************************************/

/**
 * @file
 * Wired network topology setup for rmcat ns3 module.
 *
 * @version 0.1.0
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

#ifndef LTE_TOPO_H
#define LTE_TOPO_H

#include "topo.h"

namespace ns3 {

// Infrastructure LTE Network Topology
//
//  UE(user equipment): 7.0.0.0/255.0.0.0                EPC <-> Internet: 1.0.0.0/255.0.0.0
//
//                   -------------  uplink --------------->
//  *    *    *   ...   *
//  |    |    |   ...   |                            +-----------+   Point-to-Point
// ue0  ue1  ue2  ...  uen  ++)))))))))))))++ eNB -- + SGW / PGW + ----------------- remote host
//                                                   +-----------+
//                            radio network
//                   <-------------  downlink -------------
//

class LteTop: public Topo
{
public:
    virtual ~LteTop() {}

    /**
     * Build the LTE rmcat topology with the attributes passed
     *
     *
     * @param [in] numberOfUEs number of user equipments
     * @param [in] distanceInMeters distance between user equipments
     */
    void Build(uint16_t numberOfUEs, double distanceInMeters, uint32_t msDelay);

    /**
     * Install a one-way TCP flow in a pair of nodes: a UE node and a
     * remote host node
     *
     * @param [in] flowId     A string denoting the flow's id. Useful for
     *                        logging and plotting
     * @param [in] nodeId     index of the lte node where the TCP flow is
     *                        to be installed.
     * @param [in] serverPort TCP port where the server TCP application is
     *                        to listen on
     * @param [in] downstream If true, the UE node is to act as receiver
     *                        (downstream direction); if false, the UE node
     *                        is to act as sender (upstream direction)
     *
     * @retval A container with the two applications (sender and receiver)
     */
    ApplicationContainer InstallTCP (const std::string& flowId,
                                     uint32_t nodeId,
                                     uint16_t serverPort,
                                     bool downstream);

    /**
     * Install a one-way constant bitrate (CBR) flow over UDP in a pair of nodes:
     * a UE node and a remote host node
     *
     * @param [in] nodeId      index of the lte node where the CBR-over-UDP
     *                         flow is to be installed.
     * @param [in] serverPort UDP port where the receiver CBR UDP application
     *                        is to receive datagrams
     * @param [in] bitrate Bitrate (constant) at which the flow is to operate
     * @param [in] packetSize Size of of the data to be shipped in each
     *                        datagram
     * @param [in] downstream If true, the UE node is to act as receiver
     *                        (downstream direction); if false, the UE node
     *                        is to act as sender (upstream direction)
     *
     * @retval A container with the two applications (sender and receiver)
     */
    ApplicationContainer InstallCBR (uint32_t nodeId,
                                     uint16_t serverPort,
                                     uint64_t bitrate,
                                     uint32_t packetSize,
                                     bool downstream);

    /**
     * Install a one-way RMCAT flow in a pair of nodes: a UE node and a
     * remote host node
     *
     * @param [in] flowId     A string denoting the flow's id. Useful for
     *                        logging and plotting
     * @param [in] nodeId     index of the lte node where the RMCAT flow
     *                        is to be installed.
     * @param [in] serverPort UDP port where the #RmcatReceiver application
     *                        is to receive media packets
     * @param [in] downstream If true, the UE node is to act as receiver
     *                        (downstream direction); if false, the UE node
     *                        is to act as sender (upstream direction)
     *
     * @retval A container with the two applications (sender and receiver)
     */
    ApplicationContainer InstallRMCAT (const std::string& flowId,
                                       uint32_t nodeId,
                                       uint16_t serverPort,
                                       bool downstream);


    NodeContainer& GetUeNode() {
        return m_ueNodes;
    }

    NodeContainer& GetRemoteNode() {
        return m_remoteNode;
    }

private:

    static void EnableLTELogComponents();

protected:
    static Ptr<ns3::PointToPointEpcHelper>  epcHelper;
    NodeContainer m_ueNodes;
    NodeContainer m_enbNodes;
    NodeContainer m_remoteNode;
    NetDeviceContainer m_ueDevices;
    NetDeviceContainer m_enbDevices;
    NetDeviceContainer m_remoteDevices;
};

}

#endif /* LTE_TOPO_H */
