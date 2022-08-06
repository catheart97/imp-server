import requests
import os
import json

host = "http://192.168.188.99:8000"

root = os.path.abspath(os.path.dirname(__file__)) + os.sep
folder = root + ".." + os.sep + "requests" + os.sep + "Child Puzzle" + os.sep + ""

files = [file for file in os.listdir(folder) if file.endswith(".json")]

response = requests.request("GET", host + "/clear")

files = sorted(files, key=lambda x: int(x.split(".")[0].split("_")[1]))
for file in files:
    target = file.split(".")[0].split("_")[2]
    data = json.load(open(folder + file, "r"))

    headers = {"Content-Type": "application/json"}
    response = requests.request("PUT", host + "/" + target, json=data, headers=headers)
    print(response.text)