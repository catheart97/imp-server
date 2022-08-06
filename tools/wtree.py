#!/usr/bin/env python3.9

import vedo
import json
import os

from tvis import pointMap
import tvis


def loadWTree(js):

    nodes = js['Nodes']
    nodes_positions = [pointMap(node['Config']['Position']) for node in nodes]
    rendered = [False for _ in nodes]
    root_idx = js['Position']
    # root_idx += 34
    print("num_nodes", len(nodes_positions))

    actors = []
    # actors += [vedo.Point([-0.0827958, -0.100457, 1.28703], c="yellow", r=13)]

    # for i, node in enumerate(nodes):
    #     if abs(node['Config']['Position'][0] + 0.0827958) < 1e-4:
    #         print(i, node)

    print("Position:", root_idx)

    # actors += [vedo.Points(nodes_positions, c="b", r=4)]

    while not nodes[root_idx]['IsRoot']:
        parent_idx = nodes[root_idx]['Parent']
        actors.append(vedo.Line([nodes_positions[root_idx], nodes_positions[parent_idx]], c="red", lw=4))
        rendered[root_idx] = True
        root_idx = parent_idx
    rendered[root_idx] = True

    for i, node in enumerate(nodes):
        if (rendered[i]):
            continue
        parent_idx = node['Parent']
        actors.append(vedo.Line([nodes_positions[i], nodes_positions[parent_idx]], c="black"))
        rendered[i] = True

    return actors

def loadEST(js):

    nodes = js['Nodes']
    nodes_positions = [pointMap(node['Config']['Position']) for node in nodes]
    actors = []
    for i, node in enumerate(nodes):
        parent_idx = node['Parent']
        actors.append(vedo.Line([nodes_positions[i], nodes_positions[parent_idx]], c="green"))

    return actors

if __name__ == "__main__":
    root = os.path.dirname(os.path.abspath(__file__)) + os.sep

    # Configuration Start-

    file_root = root + ".." + os.sep + "build" + os.sep + "Release" + os.sep
    js = json.load(open(file_root + "world_tree.json", "r"))['WTree']

    print(js.keys())

    MAXIMUM_KD_DEPTH = 100
    EXCLUSIVE_HIGHLIGHT_KD_NODES = False
    HIGHLIGHT_KD_NODES = 1
    VISUALIZE_KD_NODES = False

    # Configuration End
    actors = []
    # actors += loadKDTree(js, MAXIMUM_KD_DEPTH, EXCLUSIVE_HIGHLIGHT_KD_NODES, HIGHLIGHT_KD_NODES)
    # actors += loadEST(json.load(open(file_root + "est-1645897132-1.json", "r"))['EST'])
    actors += loadWTree(js)
    actors += tvis.loadEST(file_root + "est-1645897498-1.json")
    # actors += loadEST(file_root + "est-1645882359-2.json")
    # actors += loadEST(file_root + "est-1645881041-2.json")
    # actors += loadEST(file_root + "est-1645881041-3.json")

    print("Number of actors:", len(actors))

    plotter = vedo.Plotter(screensize=(1000, 1000))
    plotter.addCallback("KeyPress", lambda args: plotter.screenshot(
        f"graph.png") if args['keyPressed'] == 'space' else None)
    plotter.show(*actors, axes=True, interactive=True)
