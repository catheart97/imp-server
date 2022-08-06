#!/usr/bin/env python3.9

import vedo
import json
import os
import numpy
import pyquaternion


def loadEST(filename):
    global VisualizeLeaves
    global VisualizeChildBox
    global VisualizeRotation

    data = json.load(open(filename))
    root_rot = pyquaternion.Quaternion(
        *data['EST']["Nodes"][0]['Config']['Rotation'])

    points = [
        pointMap(node['Config']['Position'])
        for node in data['EST']["Nodes"]
    ]
    rotations = [
        pyquaternion.Quaternion(*node['Config']['Rotation'])
        for node in data['EST']["Nodes"]
    ]

    print("n_est_nodes", len(points))

    rotation_distances = numpy.array([
        pyquaternion.Quaternion.sym_distance(root_rot, q) for q in rotations
    ])
    rotation_distances /= numpy.max(rotation_distances)

    actors = []
    part_of_solution = [False for _ in points]
    if "Matchee" in data.keys():
        matchee_pos = tuple(pointMap(data['Matchee']['Position']))
        actors.append(vedo.Point(matchee_pos, c="green"))

        sol_idx = data['SolutionIndex']
        print("Solindex", sol_idx)
        print("solnode_est", data['EST']["Nodes"][sol_idx])
        if data["CompleteSolution"]:
            actors.append(
                vedo.Line(matchee_pos, points[sol_idx], c="green", lw=3))
        if not VisualizeLeaves:
            actors.append(
                vedo.Line(points[sol_idx], points[data['EST']["Nodes"][sol_idx]['Parent']], c="red", lw=3))
        while not data['EST']["Nodes"][sol_idx]["IsRoot"]:
            part_of_solution[sol_idx] = True
            sol_idx = data['EST']["Nodes"][sol_idx]['Parent']
        part_of_solution[sol_idx] = True

    isleaf = [True for _ in points]
    for i, node in enumerate(data['EST']["Nodes"]):
        if not node['IsRoot']:
            isleaf[node['Parent']] = False
        else:
            isleaf[i] = False
    for i, node in enumerate(data['EST']["Nodes"]):
        if not node['IsRoot']:
            if not isleaf[i] or VisualizeLeaves:
                if part_of_solution[i] and part_of_solution[node['Parent']]:
                    actors.append(
                        vedo.Line(points[i], points[node['Parent']], c="green", lw=3))
                else:
                    actors.append(vedo.Line(points[i], points[node['Parent']]))
                if VisualizeRotation:
                    actors.append(vedo.Point(
                        points[i], c=[rotation_distances[i], 0, rotation_distances[i]], r=6))
        else:
            actors.append(vedo.Point(points[i], c="blue"))

    return actors


__points_added = 0


def loadKDTree(filename):
    global MaximumKDDepth
    global __points_added
    global HighlightKDNodes
    global ExclusiveHighlightKDNodes

    js = json.load(open(filename))

    data = js['KDTree']['Data']
    node = js['KDTree']['Root']

    def preTraverse(node, depth):
        if depth > MaximumKDDepth:
            return 0, MaximumKDDepth

        if node["Split"] is not None:
            mrl, mdl = preTraverse(node['Left'], depth + 1)
            mrr, mdr = preTraverse(node['Right'], depth + 1)
            return max(mrl, mrr), max(mdl, mdr)
        else:
            points = node["Points"]
            local_data = [data[i]['Rating'] for i in points] + [0]
            return max(local_data), depth

    max_rating, max_depth = preTraverse(node, 1)
    __points_added = 0

    def treeTraverse(node, depth):
        global __points_added
        global VisualizeKDNodes

        if depth > MaximumKDDepth:
            return []

        actors = [
            vedo.Box(size=[
                -node['Box']['Max']['Position'][0],
                -node['Box']['Min']['Position'][0],
                node['Box']['Min']['Position'][1],
                node['Box']['Max']['Position'][1],
                node['Box']['Min']['Position'][2],
                node['Box']['Max']['Position'][2],
        ]).c([0, 0, depth / max_depth]).wireframe().lw(4)]

        if node["Split"] is not None:
            if ExclusiveHighlightKDNodes:
                actors = []
            actors += treeTraverse(node['Left'], depth + 1)
            actors += treeTraverse(node['Right'], depth + 1)
        else:
            points = node["Points"]
            local_data = [data[i] for i in points]

            if __points_added < HighlightKDNodes:
                actors[-1].c([1, 0, 0]).lw(8)

            if len(local_data) > 0 and __points_added < HighlightKDNodes and VisualizeKDNodes:
                for p in local_data:
                    actors.append(vedo.Point(pointMap(p["Config"]["Position"]), c=[
                        p['Rating'] / max_rating, p['Rating'] / max_rating, p['Rating'] / max_rating]))
                __points_added += 1
            else:
                if ExclusiveHighlightKDNodes:
                    actors = []
                elif VisualizeKDNodes:
                    for p in local_data:
                        actors.append(vedo.Point(pointMap(p["Config"]["Position"]), c=[
                            p['Rating'] / max_rating, p['Rating'] / max_rating, p['Rating'] / max_rating]))
        return actors
    return treeTraverse(node, 1)


def pointMap(point):
    # return [-point[0], point[1], point[2]]
    return point

def pointsMap(points):
    return list(map(pointMap, points))

VisualizeLeaves = bool(1)
VisualizeObjects = bool(1)
VisualizeRotation = bool(0)
HighlightKDNodes = 0
ExclusiveHighlightKDNodes = bool(0)
VisualizeKDNodes = bool(0)
MaximumKDDepth = 10

if __name__ == "__main__":
    root = os.path.dirname(os.path.abspath(__file__)) + os.sep

    # Configuration Start

    FileRoot = root + ".." + os.sep + "build" + os.sep + "Release" + os.sep

    ID = 1645881041
    # ID = None
    Files = os.listdir(FileRoot)
    # IDs = {file.split("-")[1]for file in Files if not ".exe" in file and not "kd" in file and not "request" in file and not "world" in file}
    if ID is None:
        ID = max(IDs)
    Files = [file for file in Files if file.endswith(
        ".json") and f"est-{ID}-" in file and not "kd" in file and not "request" in file]
    # Files = sorted(Files)
    Files = ["est-1645881041-1.json"]

    # Configuration End
    actors = []
    for file in Files:
        actors += loadEST(FileRoot + file)
        actors += loadKDTree(FileRoot + file)

    if VisualizeObjects:
        data = json.load(open(FileRoot + Files[0]))
        if "Objects" in data.keys():
            for obj in data["Objects"]:
                m = vedo.Mesh([pointsMap(obj["Vertices"]), obj["Triangles"]])
                # m.rotateX(-90)
                m.rotate(obj["Angle"], obj["Axis"], rad=True)
                m.pos(*pointMap(obj["Position"]))
                m.c("lightgray")
                # m.wireframe()
                actors.append(m)

    print("Number of actors:", len(actors))

    plotter = vedo.Plotter(screensize=(800, 800))
    plotter.addCallback("KeyPress", lambda args: plotter.screenshot(
        f"graph.png") if args['keyPressed'] == 'space' else None)
    plotter.show(*actors, axes=True, interactive=True)
