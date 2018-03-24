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
 * Test suite for rmcat lte test cases.
 *
 * @version 0.1.0
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

/*
 * This file contains implementation of the
 * RmcatWifiTestSuite class. It instantiates
 * the collection of lte test cases as described
 * in Section 3 of the following IETF draft:
 *
 * Evaluation Test Cases for Interactive Real-Time
 * Media over Wireless Networks
 * https://tools.ietf.org/html/draft-ietf-rmcat-wireless-tests-04
 *
 */

#include "rmcat-lte-test-case.h"

/*
 * Defines common configuration parameters of a
 * RMCAT lte test case.
 *
 * Implement common procedures in setting up a
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

/*
 * Defines collection of test cases as specified in
 * Section 4 of the rmcat-wireless-tests draft
 */
class RmcatLteTestSuite : public TestSuite
{
public:
    RmcatLteTestSuite ();

private:
    void AddLteTestCase (uint32_t delay, uint32_t velocity, uint32_t numberOfUEs, bool downstream, float intensity);
};

void RmcatLteTestSuite :: AddLteTestCase (uint32_t delay, uint32_t velocity, uint32_t numberOfUEs, bool downstream, float intensity)
{
    std::stringstream ss;
    ss << "rmcat-lte-test-case-3.1.2" << "-v" << velocity << "-d" << delay << "-n"
       << numberOfUEs << (downstream?"-downlink":"-uplink")  << "-i" << intensity;
    RmcatLteTestCase * ltecase = new RmcatLteTestCase{velocity, delay, numberOfUEs, downstream, intensity, ss.str ()};
    AddTestCase (ltecase,TestCase::QUICK);
}

RmcatLteTestSuite :: RmcatLteTestSuite ()
    : TestSuite{"rmcat-lte", UNIT}
{
    // delay, velocity, numberOfUEs, downstream, intensity
    for (uint32_t i = 1; i <= 16; i *= 2) {
        AddLteTestCase(40, 3, i, true, 0.7);
        AddLteTestCase(40, 3, i, false, 0.7);
    }
}

static RmcatLteTestSuite rmcatLteTestSuite;
