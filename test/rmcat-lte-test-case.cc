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
 * Common template for rmcat lte test cases.
 *
 * This file contains implementation of the
 * RmcatWifiTestCase class. It defines the
 * common template for individual rmcat-lte
 * test cases as specified in Section 4 of the
 * following IETF draft:
 *
 * Evaluation Test Cases for Interactive Real-Time
 * Media over Wireless Networks
 * https://tools.ietf.org/html/draft-ietf-rmcat-wireless-tests-04
 *
 * @version 0.1.0
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

#include "rmcat-lte-test-case.h"

NS_LOG_COMPONENT_DEFINE ("RmcatSimTestLte");

/*
 * Implement common procedures in setting up a
 * rmcat-lte test case:
 * -- configuration of network topology, including
 *    number of wired and wireless nodes;
 * -- configuration of capacity and propagation
 *    delay of the wired connection
 * -- configuration of PHY- and MAC-layer parameters
 *    for the wireless connections
 * -- configuration of traversing traffic flows
 *    in terms of traffic type (i.e., RMCAT/TCP/UDP),
 *    arrival and departure patterns
 */

/* Constructor */
RmcatLteTestCase::RmcatLteTestCase (uint32_t speed, // speed in km/h
                                    uint32_t pdelay,   // wired link forward propagation delay (in ms)
                                    uint32_t numberOfUEs, // number of user equipments
                                    bool downstream,  // enable downstream or upstream
                                    float intensity,    // inter-arrival time, given by a poisson process
                                    std::string desc)  // test case name/description
: RmcatTestCase{0, pdelay, 0, desc} // 0 -> use default in toplogy setup
, m_speed{speed}
, m_numberOfUEs{numberOfUEs}
, m_downstream{downstream}
, m_simTime{RMCAT_TC_SIMTIME}
, m_codecType{SYNCODEC_TYPE_FIXFPS}
{}


/**
 * Instantiate RMCAT flows
 */
void RmcatLteTestCase::SetUpRMCAT (std::vector<Ptr<RmcatSender> >& send,
                                   bool downstream, size_t numFlows)
{
    const uint32_t basePort = RMCAT_TC_RMCAT_PORT + (downstream ? 0: 1000);
    std::stringstream ss0;
    if (downstream) {
        ss0 << "rmcatDn_";
    } else {
        ss0 << "rmcatUp_";
    }

    for (uint32_t i = 0; i < numFlows; i++) {
        std::stringstream ss;
        ss << ss0.str () << i;

        ApplicationContainer rmcatApps = m_topo.InstallRMCAT (ss.str (),        // flowID
                                                              i,   // nodeID
                                                              basePort + i * 2, // port # ,
                                                              downstream);             // direction

        send[i] = DynamicCast<RmcatSender> (rmcatApps.Get (0));
        send[i]->SetCodecType (m_codecType);
        send[i]->SetRinit (RMCAT_TC_RINIT);
        send[i]->SetRmin (RMCAT_TC_RMIN);
        send[i]->SetRmax (RMCAT_TC_RMAX);
        send[i]->SetStartTime (Seconds (0));
        send[i]->SetStopTime (Seconds (m_simTime-1));
    }
}

/** Instantiate CBR-over-UDP background flows */
void RmcatLteTestCase::SetUpCBR (std::vector<Ptr<Application> >& cbrSend,
                                 bool downstream, size_t numFlows, uint32_t rCBR)
{

    const uint32_t basePort = RMCAT_TC_CBR_UDP_PORT + (downstream ? 0: 1000);
    for (uint32_t i = 0; i < numFlows; i++) {
        ApplicationContainer cbrApps = m_topo.InstallCBR (i,   // node ID
                                                          basePort+i,   // port #,
                                                          rCBR,       // rate of CBR
                                                          RMCAT_TC_UDP_PKTSIZE,
                                                          downstream);

        cbrSend[i] = cbrApps.Get (0);
        cbrSend[i]->SetStartTime (Seconds (0));
        cbrSend[i]->SetStopTime (Seconds (m_simTime-1));
    }
}

/**
 * Inherited DoSetup function:
 * -- Build network topology
 * -- Enable additional logging
 */
void RmcatLteTestCase::DoSetup ()
{
    RmcatTestCase::DoSetup ();

    m_topo.Build (m_numberOfUEs, m_speed, m_delay);

    ns3::LogComponentEnable ("RmcatSimTestLte", LOG_LEVEL_INFO);
}

/**
 * Inherited DoRun () function:
 * -- Instantiate RMCAT/TCP/UDP flows
 * -- Populate routing table
 * -- Kickoff simulation
 */
void RmcatLteTestCase::DoRun ()
{
    uint32_t flowNum = m_numberOfUEs;

    if (m_downstream) {
        //uint32_t downlinkCbrRateTotal = 2000000;
        //std::vector< Ptr<Application> > sendDnCBR (flowNum);
        //setUpCBR (sendDnCBR, true, flowNum, downlinkCbrRateTotal/flowNum);

        std::vector< Ptr<RmcatSender> > sendDnRMCAT (flowNum);
        SetUpRMCAT (sendDnRMCAT, true, flowNum);
    } else {
        //uint32_t uplinkCbrRateTotal = 2000000;
        //std::vector< Ptr<Application> > sendUpCBR (flowNum);
        //setUpCBR (sendUpCBR, false, flowNum, uplinkCbrRateTotal/flowNum);

        std::vector< Ptr<RmcatSender> > sendUpRMCAT (flowNum);
        SetUpRMCAT (sendUpRMCAT, false, flowNum);
    }

    /* Kick off simulation */
    NS_LOG_INFO ("Run Simulation.");
    Simulator::Stop (Seconds (m_simTime));
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");
}

