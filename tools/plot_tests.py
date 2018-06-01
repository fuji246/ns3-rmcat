#!/usr/bin/python

###############################################################################
#  Copyright 2016-2017 Cisco Systems, Inc.                                    #
#                                                                             #
#  Licensed under the Apache License, Version 2.0 (the "License");            #
#  you may not use this file except in compliance with the License.           #
#                                                                             #
#  You may obtain a copy of the License at                                    #
#                                                                             #
#      http://www.apache.org/licenses/LICENSE-2.0                             #
#                                                                             #
#  Unless required by applicable law or agreed to in writing, software        #
#  distributed under the License is distributed on an "AS IS" BASIS,          #
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   #
#  See the License for the specific language governing permissions and        #
#  limitations under the License.                                             #
###############################################################################

import os
import sys
import json
import matplotlib.pyplot as plt

colorlist = ['blue',
             'orange',
             'green',
             'pink',
             'red',
             'purple',
             'gray',
             'black',
             'brown',
             'aqua',
             'navy',
             'teal',
             'olive',
             'coral',
             'lime',
             'royalblue',
             'maroon',
             'yellowgreen',
             'tan',
             'khaki',
             'darkslategrey',
             'darkgreen',
             'sienna',
             'peachpuff',
             'sandybrown',
             'steelblue']

tableau20 = [(31, 119, 180), (174, 199, 232), (255, 127, 14), (255, 187, 120),
             (44, 160, 44), (152, 223, 138), (214, 39, 40), (255, 152, 150),
             (148, 103, 189), (197, 176, 213), (140, 86, 75), (196, 156, 148),
             (227, 119, 194), (247, 182, 210), (127, 127, 127), (199, 199, 199),
             (188, 189, 34), (219, 219, 141), (23, 190, 207), (158, 218, 229)]

# Scale the RGB values to the [0, 1] range, which is the format matplotlib accepts.
for i in range(len(tableau20)):
    r, g, b = tableau20[i]
    colorlist.append((r / 255., g / 255., b / 255.))

def plot_mobility(tc_name, contents, dirname):
    mobility_log = contents['pos']
    if len(mobility_log) == 0: return

    node_pos = {}
    for nodeid, pos_lst in mobility_log.iteritems():
        ts_lst, x_lst, y_lst = node_pos.setdefault(nodeid, [[], [], []])
        for pos_item in pos_lst:
            ts_lst.append(pos_item[0])
            x_lst.append(pos_item[1])
            y_lst.append(pos_item[2])

    fig = plt.figure()
    l = len(colorlist)
    color_idx = 0
    for nodeid, ts_x_y_lst in node_pos.iteritems():
        c = colorlist[color_idx % l]
        plt.plot(ts_x_y_lst[1], ts_x_y_lst[2], color=c)
        color_idx += 1

    plt.ylabel('y (m)')
    plt.xlabel('x (m)')
    pngfile = '{}-mobility.png'.format(tc_name);
    fig.savefig(os.path.join(dirname, pngfile))
    plt.close(fig)

    fig = plt.figure()
    plt.subplot(211)
    for nodeid, ts_x_y_lst in node_pos.iteritems():
        plt.plot(ts_x_y_lst[0], ts_x_y_lst[1], color=colorlist[color_idx % l])
        color_idx += 1
    plt.xlabel('timestamp (second)')
    plt.ylabel('x (m)')

    plt.subplot(212)
    for nodeid, ts_x_y_lst in node_pos.iteritems():
        plt.plot(ts_x_y_lst[0], ts_x_y_lst[2], color=colorlist[color_idx % l])
        color_idx += 1
    plt.xlabel('timestamp (second)')
    plt.ylabel('y (m)')

    pngfile = '{}-mobility-timeseries.png'.format(tc_name);
    fig.savefig(os.path.join(dirname, pngfile))
    plt.close(fig)

def plot_test_case(tc_name, contents, dirname):
    rmcat_log = contents['nada']
    tcp_log = contents['tcp']
    rmcat_keys = sorted(rmcat_log.keys())
    tcp_keys = sorted(tcp_log.keys())

    print 'plotting data for tc {}...'.format(tc_name)
    nflow = len(rmcat_keys) + len(tcp_keys)

    l = len(colorlist)
    pngfile = '{}.png'.format(tc_name);
    tmax = 120
    rmax = 2.5
    fig = plt.figure()
    plt.subplot(311)
    for (i, obj) in enumerate(rmcat_keys):
        rcolor = colorlist[i % l]
        rcolor2 = colorlist[i+1 % l]
        ts = [x[0] for x in rmcat_log[obj]]
        rrate = [x[6]/1.e+6 for x in rmcat_log[obj]]
        srate = [x[7]/1.e+6 for x in rmcat_log[obj]]
        rmax = max(max(rrate), max(srate))
        if nflow == 1:
            plt.plot(ts, rrate, 'd', linewidth=1.0, color=rcolor2, mfc=rcolor2, mec='none',
                                     ms=2, label=obj+'(recv)')
            plt.plot(ts, srate, 'o', linewidth=1.0, color=rcolor, mfc=rcolor, mec='none',
                                 ms=2, label=obj+'(sent)')
        else:
            plt.plot(ts, srate, 'o', linewidth=1.0, color=rcolor, mfc=rcolor, mec='none',
                                 ms=2, label=obj)

    for (i, obj) in enumerate(tcp_keys):
        rcolor = colorlist[(i + 5) % l]
        ts = [x[0] for x in tcp_log[obj]]
        rrate = [x[2]/1.e+6 for x in tcp_log[obj]]
        rmax = max(rmax, max(rrate))
        plt.plot(ts, rrate, '-o', linewidth=.7, color=rcolor, mfc=rcolor, mec='none',
                                  ms=2, label=obj)
        if max(ts)>150:
            tmax = 300

    rmax *= 1.25
    plt.xlim(0, tmax)
    plt.ylim(0, rmax)
    print("rmax = ", rmax)
    plt.ylabel('Rate (Mbps)')
    # plt.legend(loc='upper left', prop={'size':6}, bbox_to_anchor=(1,1), ncol=1)
    all_curves = len(rmcat_keys) + len(tcp_keys)
    if all_curves < 12:
        plt.legend(ncol = (all_curves / 4) + 1, loc='upper right', fontsize = 'small')

    plt.subplot(312)
    for (i, obj) in enumerate(rmcat_keys):
        rcolor = colorlist[i % l]
        ts = [x[0] for x in rmcat_log[obj]]
        qdelay = [x[1] for x in rmcat_log[obj]]
        rtt = [x[2] for x in rmcat_log[obj]]
        xcurr = [x[5] for x in rmcat_log[obj]]
        plt.plot(ts, qdelay, 'o', color=rcolor, mfc=rcolor, mec='none', ms=2, label=obj)
        # plt.plot(ts, xcurr, 'o', color='purple', mfc='purple', mec='none', ms=2, label=obj)
    plt.xlim(0, tmax)
    plt.ylim(0, 400)
    # plt.ylim(0, plt.gca().get_ylim()[1] * 1.5) #Margin for legend
    plt.ylabel('QDelay (ms)')
    # plt.legend(loc='upper left', prop={'size':6}, bbox_to_anchor=(1,1), ncol=1)
    all_curves = len(rmcat_keys)
    # plt.legend(ncol = (all_curves / 4) + 1, loc='upper right', fontsize = 'small')

    plt.subplot(313)
    for (i, obj) in enumerate(rmcat_keys):
        rcolor = colorlist[i % l]
        ts = [x[0] for x in rmcat_log[obj]]
        ploss = [x[3] for x in rmcat_log[obj]]
        plr = [x[4]*100. for x in rmcat_log[obj]]  # ratio => %
        loglen = [x[8] for x in rmcat_log[obj]]
        plt.plot(ts, plr, 'o', color=rcolor, mfc=rcolor, mec='none', ms=2, label=obj)
        # plt.plot(ts, ploss, 'd',  color=rcolor, mfc=rcolor, mec='none', ms=4, label=obj)

    plt.xlim(0, tmax)
    # plt.ylim(0, 5)
    plt.ylabel('PLR (%)')
    plt.xlabel('Time (s)')
    # plt.legend(loc='upper left', prop={'size':6}, bbox_to_anchor=(1,1), ncol=1)
    # plt.legend(ncol = (all_curves / 4) + 1, loc='upper right', fontsize = 'small')
    fig.savefig(os.path.join(dirname, pngfile))
    plt.close(fig)

    plot_mobility(tc_name, contents, dirname)

# ---------  #
if len(sys.argv) != 2:
    print >> sys.stderr, 'Usage: python {} <log_directory>'.format(sys.argv[0])
    sys.exit(1)
dirname = sys.argv[1]
assert os.path.isdir(dirname)

f_json_name = os.path.join(dirname, 'all_tests.json')
assert os.path.isfile(f_json_name)
f_json = open(f_json_name, 'r')
all_logs = json.load(f_json)
for test_case in all_logs.keys():
    plot_test_case(test_case, all_logs[test_case], dirname)
