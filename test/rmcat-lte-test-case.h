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
 * Template for rmcat lte test cases.
 *
 * @version 0.1.0
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

/*
 * This file contains definition of the rmcatLteTestCase
 * class. It defines the template for individual
 * rmcat-lte test cases as specified in
 * in Section 4 of the following IETF draft:
 *
 * Evaluation Test Cases for Interactive Real-Time
 * Media over Wireless Networks
 * https://tools.ietf.org/html/draft-ietf-rmcat-wireless-tests-04
 *
 */


#ifndef RMCAT_LTE_TEST_CASE_H
#define RMCAT_LTE_TEST_CASE_H

#include "ns3/lte-topo.h"
#include "ns3/rmcat-sender.h"
#include "ns3/rmcat-receiver.h"
#include "ns3/rmcat-constants.h"
#include "rmcat-common-test.h"

using namespace ns3;

/*
 * Defines common configuration parameters of a
 * RMCAT lte test case.
 *
 * Defines common procedures in setting up a
 * test case:
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

class RmcatLteTestCase : public RmcatTestCase
{
public:
    /* Constructor */
    RmcatLteTestCase (uint32_t speed, // speed in km/h
                      uint32_t pdelay,   // wired link forward propagation delay (in ms)
                      uint32_t numberOfUEs, // number of user equipments
                      bool downstream,  // enable downstream or upstream
                      float intensity,    // inter-arrival time, given by a poisson process
                      std::string desc);  // test case name/description

    virtual void DoSetup ();
    virtual void DoRun ();

    /*
     * Configure various parameters of
     * the test case by passing along function
     * input values to member variables
     */
    void SetSimTime (uint32_t simTime)  {m_simTime = simTime; };
    void SetCodec (SyncodecType codecType) {m_codecType = codecType; };

    /* Instantiate flows in DoRun () */
    void SetUpRMCAT (std::vector< Ptr<RmcatSender> >& send,
                     bool downstream, size_t numFlows);

    void SetUpCBR (std::vector<Ptr<Application> >& cbrSend,
                   bool downstream, size_t numFlows, uint32_t rCBR);

    void SetUpTCP (std::vector<Ptr<BulkSendApplication> >& tcpSend,
                   bool downstream, size_t numFlows);

protected:
    /* network toplogy configuration */
    LteTop m_topo;

private:
    uint32_t m_speed;
    uint32_t m_numberOfUEs;
    bool m_downstream;
    uint32_t m_simTime;  // simulation duration (in seconds)
    SyncodecType m_codecType; // traffic source type
};

#endif /* RMCAT_LTE_TEST_CASE_H */

